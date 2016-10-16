<?php
$phpinc = $_SERVER["DOCUMENT_ROOT"] . '/../phpinc';
$smartydata = $_SERVER["DOCUMENT_ROOT"] . '/../smartydata';

require_once("$phpinc/geshi.php");
require_once("$phpinc/smarty/Smarty.class.php");

// init GeSHi
$geshi = new GeSHi();
$geshi->set_language('cpp');
$geshi->set_header_type(GESHI_HEADER_PRE_VALID);
$geshi->enable_classes();
$geshi->enable_line_numbers(GESHI_NORMAL_LINE_NUMBERS);
$geshi->set_overall_style('color: #0f0f0f; border: 1px solid #d0d0d0; background-color: #f8f8f8; width: 60em;', true);
$geshi->set_line_style('font: \'Courier New\', Courier, monospace;', 'font-weight: bold;', true);

// init Smarty
$smarty = new Smarty();
$smarty->setTemplateDir("$smartydata/templates");
$smarty->setCompileDir("$smartydata/templates_c");
$smarty->setCacheDir("$smartydata/cache");
$smarty->setConfigDir("$smartydata/configs");

// header
$smarty->assign('css', $geshi->get_stylesheet(true));
$smarty->assign('title', 'Lazy logging en C/C++');
$smarty->assign('subtitle', 'Utilisation avancée des sections en C/C++');
$smarty->assign('datetime', '2012');
$smarty->assign('articletime', 'Novembre 2012');
$smarty->display('header.tpl');
?>

<h3>Référencement des données de log via des indices</h3>

<p>Tous ces efforts nous ont permi de cloisonner tous nos pointeurs dans une seule et même section. Cette dernière peut alors être considérée comme un tableau de pointeurs. Connaissant l'adresse de base de ce tableau, il est facile de déduire l'indice d'un élément dans ce tableau à partir de son adresse :</p>
<pre>
    static const log_info_t * const log_info_ptr
    __attribute__((section(".logindx, \"r\"#"))) = &log_info;

    const unsigned char * base = GetSection(".logindx").baseAddr;
    unsigned short index = ((const unsigned char *)(&log_info_ptr) - base) / sizeof(log_info_ptr);
</pre>

<p>de même, à partir de l'adresse de base de la section <tt>.logindx</tt> et de sa taille virtuelle, il est facile
d'en déduire le nombre d'éléments contenus et de tous les itérer :</p>
<pre>
    const log_info_t **allElements = (const log_info_t **)GetSection(".logindx").baseAddr;
    size_t nbElements = GetSection(".logindx").virtualSize / sizeof(void*);
    for (size_t i = 0; i &lt; nbElements; ++i)
    {
        cout &lt;&lt; allElements[i].logMessage &lt;&lt "\n";
    }
</pre>

<p>De cette manière, il est possible d'utiliser cet indice comme base d'adressage des
<tt>log_info_t</tt>. Le nombres d'octets (et même de bits) utilisés pour stocker ces
index peut alors être librement défini selon les besoins. Dans le cadre de la bibliothèque
LogRoller, il a été décidé d'utiliser 16 bits (2 octets). Si cela venait à ne pas suffire
(parce qu'il y a plus de 65536 appels à log_xxx() dans le code source), la bibliothèque
s'en appercevrait immédiatement lors de son initialisation simplement en testant la taille
de la section <tt>.logindx</tt>. Cette dernière renseigne en effet directement sur le nombre de points
de logs présents dans le programme.
</p>

<p>On peut aller encore plus loin et utiliser plusieurs sections pour grouper les pointeurs
selon certains critères. Le premier critère qui vient à l'exprit est celui du niveau de log.
</p>
<p>On peut aussi envisager de ne pas placer les pointeurs dans une zone en lecture seule,
et de leur associer un compteur qui serait incrémenté chaque fois que ce pointeur est
référencé. De cette manière, il serait facile d'ordonner les points de logs selon leur fréquence
d'utilisation, et de là d'en dégager des "zones chaudes" du code source, à partir des noms de
fichiers et fonctions référencés par les pointeurs. Cela pourrait aussi permettre d'évaluer
la couverture du code.</p>
<p>Un autre utilisation possible des sections consiste à isoler tous les messages textes liés aux logs
dans une seule et même section. Cette section peut ensuite, via un outil adapté, être extraite (supprimée)
de l'exécutable. Ceci peut être justifié dans au moins deux cas concrets : 
</p>
<ul>
<li>Quand la taille est importante. Cela est particulièrement vrai dans le monde de l'embarqué :
plus c'est petit, mieux c'est&nbsp;! Et ce d'autant plus que ce n'est pas le code embarqué qui
va reformater les logs binaires en texte lisible. Il n'y a donc aucune raison d'y incorporer ces informations
potentiellement très volumineuses : 100 fichiers sources (noms complets de 50 caractères) x 10 fonctions
(noms de 15 caractères) x 2 messages (40 caractères) = ~100 Ko&nbsp;!</li>
<li>Quand on ne veut pas exposer la structure interne de l'application. Le fait que l'exécutable contienne
le nom des fichiers sources et surtout ceux des classes et fonctions qui le compose peut poser problème
dans le monde propriétaire où de telles informations sont souvent considérées comme confidentielles.
Permettre aux développeurs de bénificier de logs verbeux sans que l'application finale
ne contienne un seul nom de fichier ou de fonction est un gros avantage.</li>
</ul>
<p>Mais comment s'y prendre pour demander au compilateur de placer <u>toutes</u> les chaînes litérales spécifiques aux
messages de logs (en particulier celles résultant de l'expansion de __FILE__ et __FUNCTION__) et <u>uniquement
celles-ci</u> dans une autre section que la section .rdata par défaut ?</p>
<p>Cela ne semble pas possible d'une façon simple et facilement transposable à différents compilateurs.
Le mieux que semble offrir certains compilateurs est de changer la section à utiliser pour stocker 
les données initialisées (data_seg). Cette possibilité a le gros inconvénient d'être globale à toutes les
données constantes d'une unité de compilation.
Or nous ne devons pas seulement stocker les chaînes qui nous intéressent dans une section
à part, nous devons avant tout n'y stocker que ce qui concerne nos messages de logs. Autrement, en supprimant la dite
section, cela aurait de graves conséquences pour le reste du programme qui tenterait d'accéder au contenu d'une
section disparue.</p>
<p>Il est donc plus simple et plus sage de partir sur la base de ce qui a été déjà accompli, à savoir la possibilité
de stocker nos structures dans la section de notre choix. En modifiant légèrement notre structure, nous pouvons
lui faire incorporer directement les informations qui nous intéressent, au lieu de les référencer ailleurs :</p>
<pre>
struct log_info_t
{
    char fileName[256];
    unsigned int lineNumber;
    char functionName[124];
    char logMessage[128];
};
static const log_info_t log_info = {
    __FILE__,
    __LINE__,
    __FUNCTION__,
    "Hello World!"
};
</pre>
<p>Comme on peut le voir, l'utilisation reste la même, seule la déclaration change. Cette nouvelle approche
suscite les remarques suivantes :
</p>
<ul>
<li>La taille des structures est fixe, quelles que soient les longueurs des chaînes à stocker.
<p>Si elles sont toutes placées dans la même section, cette section sera donc équivalente à un tableau
de <tt>log_info_t</tt>, rendant possible le référencement d'une structure non pas via son adresse (virtuelle
ou relative) mais par son emplacement (index) dans ce tableau. C'est le mécanisme optimal d'adressage de ces structures
(comme vu dans le paragraphe <a href="#reduction-adresses-memoire">Réduction de la taille des adresses mémoires référencées</a>)</p></li>
<li>En conséquences, la mémoire nécessaire au stockage est beaucoup plus importante.
<p>La taille des structures étant indépendante de celle des chaînes à stocker, il en résulte une consommation
de place beaucoup plus importante que nécessaire (chaque message de log
fait grossir l'exécutable final de 512 octets dans cet exemple). Cela est à relativiser par le fait que 
tout cet espace est destiné à être retiré de l'exécutable une fois celui-ci généré, et que cela
rend aussi possible le parcours de tous les <tt>log_info_t</tt> contenus dans cette espace dédié.
En revanche, le développeur qui n'est pas intéressé par ces possibilités et souhaite conserver le contenu des messages
de log au sein de son programme est fortement pénalisé. Sans parler de celui qui souhaite enregistrer des messages
(et noms de fichiers) au format Unicode&nbsp;!</p></li>
<li>Que se passe-t-il si l'une des chaînes fournies à l'initialisation dépasse la capacité de la strcuture ?
<p>La bonne nouvelle est que le compilateur s'en apperçoit et produit un message d'erreur (error C2117).
La mauvaise nouvelle est que la bibliothèque de logs peut faire échouer la compilation du code utilisateur,
et forcer ce dernier à modifier le message qu'il souhaite logger. Autrement dit, il est potentiellement bridé
par des choix qui relèvent du détail d'implémentation de la bibliothèque.</p></li>
</ul>
<p>Les deux derniers points sont la cause d'un choix délicat et finalement arbitraire en ce qui concerne
la taille maximale autorisée pour les messages statiques. Pour ne pas brider inutilement l'utilisateur,
on est tenté de mettre des limites élevées. Mais cela impacte directement l'espace mémoire (inutilement)
alloué et donc la pénalité envers l'utilisateur qui ne souhaite pas se compliquer la vie en retirant les
messages de logs de son exécutable.</p>

<p>Face à tous ces inconvénients, l'avantage d'un adressage par index a été jugé insuffisant, d'autant
plus qu'il est possible d'y parvenir en introduisant une variable statique supplémentaire. Une autre approche a donc été
développée.</p>

<h3>Stockage optimal des messages statiques dans une section dédiée</h3>
<p>Le choix technique qui a été fait est illustré par l'exemple suivant :</p>
<pre>
void print_log_info(const char *log_info)
{
    size_t offset = 0;
    string logMessage = log_info[offset];
    
    offset += logMessage.size() + 1; 
    string fileName = log_info[offset];
    
    offset += fileName.size() + 1; 
    string functionName = log_info[offset];
    
    cout &lt;&lt; "(" &lt;&lt; fileName &lt;&lt; ") ["
        &lt;&lt; functionName &lt;&lt; "] "
        &lt;&lt; logMessage &lt;&lt; endl; 
}
 
int main()
{
    static const char log_info[] = "Hello World!" "\0" __FILE__ "\0" __FUNCTION__;
    print_log_info(log_info);    
}
</pre>
<p>L'astuce est très simple : on utilise la capacité du préprocesseur à concaténer
des chaînes de caractères pour générer une seule chaîne contenant toutes les informations
données.</p>
<p>A une exception près : le numéro de ligne.</p>

<?php
// footer
$smarty->display('footer.tpl');
?>
