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
$smarty->assign('subtitle', 'Gestion du multi-threading');
$smarty->assign('datetime', '2012');
$smarty->assign('articletime', 'Novembre 2012');
$smarty->display('header.tpl');
?>

<h3>Identification des différents threads</h3>

<p>Comment identifier les différents threads depuis lesquels sont émis les messages de log ?</p>

<p>Il est d'abord tentant d'utiliser leur identifiant système, mais comment faire ?</p>
<ul>
<li><tt>pthread_t</tt> est un type opaque dont le type réel peut varier d'une implémentation à l'autre. En théorie, il peut même s'agir d'une structure. En pratique, il s'agit souvent d'un <tt>unsigned long int</tt>, dont la taille est de 4 octets en 32 bits, 8 octets en 64 bits.</li>
<li><tt>HANDLE</tt> est défini en tant que <tt>void *</tt>. Sous Win32, un handle fait donc 4 octets. Sous Win64, il en fait 8.</li>
</ul>

<p>On se rend compte que même s'il est possible de traiter l'identifiant système du thread comme un entier non signé, cet entier sera de 4 voire 8 octets en fonction du système, ce qui n'est pas vraiment optimal quand il s'agit d'associer un identifiant de thread <i>chaque</i> message de log.</p>

<p>Une autre approche consiste à générer ses propres identifiants de thread. C'est assez facile à faire :</p>
<ul>
<li>Un compteur global est incrémenté à chaque fois qu'un nouveau thread est détecté.</li>
<li>L'identifiant de chaque thread est stocké en tant que variable locale au thread (thread local storage), ce qui permet de facilement détecter que l'on a affaire à un nouveau thread.</li>
</ul>

<?php
$geshi->set_source(<<<CPP
/* compteur global */
static unsigned long nb_threads = 0;

typedef unsigned char log_thread_id;
#define LOG_MAX_THREAD UCHAR_MAX

/* Renvoie l'identifiant du thread courant à utiliser dans les logs.
   Supporte au maximum LOG_MAX_THREAD threads. Si cette limite est
   atteinte, renvoie LOG_MAX_THREAD comme identifiant des futurs threads. */
log_thread_id get_current_thread_id()
{
    /* compteur local à chaque thread */
    static __thread unsigned long current_thread_id = 0;
    
    if (current_thread_id == 0) {
        current_thread_id = atomic_inc(&nb_thread);
    }
    /* tronquer volontairement à LOG_MAX_THREAD */
    return (log_thread_id) current_thread_id;
}
CPP
);

echo $geshi->parse_code();
?>

<p>Cette approche est très simple et fonctionne très bien. Elle a l'avantage de générer des identifiants dont la valeur permet un stockage optimal (ici sur un seul byte), et qui de plus correspondent à l'ordre d'apparition des threads ce qui est plus parlant à lire que d'arbitraires nombres générés par le système.</p>

<p>Mais si une application a tendance à créer beaucoup de threads sans les réutiliser, le nombre de threads comptés va rapidement augmenter et finir par déborder la capacité d'un seul byte. Le code ci-dessus évite soigneusement de faire "boucler" les identifiant (recommencer à zéro si débordement) car typiquement le thread principal de l'application se vera affecté la valeur 0, et affecter une seconde fois cette même valeur à un autre thread ne peut que perturber la lecture des logs finaux.</p>

<h3>Enregistrement des messages dans une file partagée ou spécifique à chaque thread ?</h3>

<p>Faut-il enregistrer les messages des threads dans une même zone mémoire commune, ou au contraire dans un tampon spécifique à chaque thread ?</p>

<p>En terme de performance, une zone mémoire commune est limitante car elle implique davantage d'accès concurrents et donc une pénalité au niveau des accès. En cause: les primitives de synchronisation atomiques qui sont plus lentes que leurs homologues classiques. Et aussi l'invalidation des caches inhérente à toute modification d'une zone partagée qui détériore grandement le temps de relecture suivant. Ce dernier point est un obstacle majeur à la "scabilité" d'un algorithme, car il s'aggrave avec le nombre de coeurs physiques (l'hyperthreading n'est pas concerné car les deux processeurs logiques partagent le même cache physique). Du moins, c'est ce que dit la théorie.</p>

<p>Pour se faire une idée plus concrète de l'impact d'une écriture concurrente dans le même buffer, il faut effectuer des tests. C'est que fait le programme <tt>test_buffer_write.c/<tt>. Ce dernier permet de confirmer que le partage d'un compteur global est le principal point de congestion.</p>

<?php
// footer
$smarty->display('footer.tpl');
?>
