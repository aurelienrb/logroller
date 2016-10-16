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
$smarty->assign('subtitle', 'Introduction aux sections en C/C++');
$smarty->assign('datetime', '2012');
$smarty->assign('articletime', 'Novembre 2012');
$smarty->display('header.tpl');
?>

<h3>Réduction de la taille des adresses mémoires référencées</h3>

<p>Grâce à l'utilisation de structures constantes statiques, nous sommes parvenus à économiser la recopie
de plusieurs chaînes de caractères complètes en référençant à la place une simple et unique adresse mémoire : celle de la structure contenant ces chaînes. Mais on peut aller plus loin.</p>

<p>Sur une machine 32 bits, la taille du pointeur référencé est 4 octets. Sur une machine 64 bits, c'est le double. Or, nous n'avons pas besoin d'une telle plage d'adresses. Un programme contient rarement plus de quelques milliers de messages de log. Aussi 2 octets (65536 valeurs possibles) seraient amplement suffisants pour notre besoin, soit un gain mémoire d'un facteur 2 en 32 bits, 4 en 64. Mais comment s'y prendre ?</p>

<p>Prenons l'exemple de départ suivant :</p>
<?php
$geshi->set_source(<<<CPP
void f()
{
    static const log_info_t log_info = {
        __FILE__,
        __LINE__,
        __FUNCTION__,
        "Hello World!"
    };
}
CPP
);
echo $geshi->parse_code();
?>

<p>Comme nous l'avons vu, cette structure sera placée par le compilateur dans la section des données initialisées en lecture seule (<tt>.rdata</tt> / <tt>.rodata</tt>). Celle-ci contient non seulement notre structure (de taille 4x4=16 ou 4x8=32 octets en fonction du système), mais aussi les chaînes de caractères litérales référencées dans cette structure, ainsi que toutes les autres données constantes du programme. Cela en fait beaucoup.</p>

<p>Apportons maintenant la modification suivante :</p>
<?php
$geshi->set_source(<<<CPP
void f()
{
    static const log_info_t log_info = {
        __FILE__,
        __LINE__,
        __FUNCTION__,
        "Hello World!"
    };
    static const log_info_t * const log_info_ptr = &log_info;
}
CPP
);
echo $geshi->parse_code();
?>

<p>Nous avons introduit une nouvelle donnée statique constante (pointeur <tt>log_info_ptr</tt>) qui référence notre structure initiale. Sa taille est donc de 32 ou 64 bits en fonction du système. Notons que c'est le deuxième <tt class="keyw">const</tt> dans la déclaration qui rend la donnée constante (le premier <tt class="keyw">const</tt> indique que le type pointé est constant mais il n'empêche pas de modifier la variable <tt>log_info_ptr</tt> elle-même). Sans lui, <tt>log_info_ptr</tt> ne serait pas éligible pour la section <tt>.rdata</tt>.</p>

<p>Nous allons maintenant utiliser des extensions de compilateurs pour créer une section <tt>.logindx</tt> dans laquelle
nous allons placer notre pointeur <tt>log_info_ptr</tt>. Cette section ne comportera que des pointeurs vers nos structures de log qui seront quant à elles stockées ailleurs (dans <tt>.rdata</tt>). Cette nouvelle section <tt>.logindx</tt> peut donc être vue comme un tableau de pointeurs. Et chaque pointeur pourra être identifié par son indice dans cette table, d'où le nom de <tt>.logindx</tt>.</p>

<h3>Création d'une section de données personnalisée</h3>

<p>Sous Windows, le format exécutable PE limite le nom d'une section à 8 caractères au maximum. Cette limitation impacte tous les compilateurs de cette plateforme. Pour simplifier la portabilité de notre code, nous nous limiterons donc à des noms de section de 8 caractères au maximum, quelle que que soit la plateforme cible.</p>

<h4>Visual C++</h4>

<p>Avec Visual C++, il est nécessaire de d'abord déclarer la section via un <tt class="keyw">#pragma</tt>. C'est aussi de cette manière que l'on peut spécifier les <a href="http://msdn.microsoft.com/en-us/library/50bewfwa.aspx">attributs</a> de cette section. On peut ensuite la référencer via un attribut de stockage personnalisé (<a href="http://msdn.microsoft.com/en-us/library/5bkb2w6t(v=vs.110).aspx">__declspec(allocate)</a>). La variable ainsi qualifiée doit être globale (ou statique) puisque les variables locales sont nécessairement placées sur la pile.</p>

<?php
$geshi->set_source(<<<CPP
// déclaration de la section .logindx en lecture seule
#pragma section(".logindx", read)

void f()
{
    static const log_info_t log_info = {
        __FILE__,
        __LINE__,
        __FUNCTION__,
        "Hello World!"
    };

    __declspec(allocate(".logindx"))
    static const log_info_t * const log_info_ptr = &log_info;
}
CPP
);
echo $geshi->parse_code();
?>

<p>dumpbin nous confirme qu'une nouvelle section <tt>.logindx</tt> de 4 octets a bien été créée.</p>
<pre>
dumpbin
</pre> 

<h4>GCC (Linux)</h4>

<p>Sous GCC, il n'est pas nécessaire de d'abord déclarer la section avant de la référencer, et la définition de l'attribut de stockage se fait en aval de la déclaration :</p>

<?php
$geshi->set_source(<<<CPP
void f()
{
    static const log_info_t log_info = {
        __FILE__,
        __LINE__,
        __FUNCTION__,
        "Hello World!"
    };

    static const log_info_t * const log_info_ptr
    __attribute__((section(".logindx"))) = &log_info;
}
CPP
);
echo $geshi->parse_code();
?>

<p>Ce faisant, nous créons une section <tt>.logindx</tt> avec les attributs par défaut, ce qui autorise la modification de son contenu. Notre variable <tt>log_info_ptr</tt> n'est pas réellement const :</p>

<?php
$geshi->set_source(<<<CPP
    static const log_info_t * log_info_ptr
    __attribute__((section(".logindx"))) = &log_info;

    log_info_ptr = NULL;
CPP
);
echo $geshi->parse_code();
?>

<p>Après suppression du deuxième const (comme si dessus, ou via un const_cast), on constate bien qu'il est possible de modifier l'adresse du pointeur. Pour spécifier que notre section doit être en lecture seule, la syntaxe à utiliser est un peu obscure :</p>
<?php
$geshi->set_source(<<<CPP
    static const log_info_t * const log_info_ptr
    __attribute__((section(".logindx, \"r\"#"))) = &log_info;
CPP
);
echo $geshi->parse_code();
?>

<p>Il s'agit en fait d'éléments de syntaxe issus de <tt>gas</tt> (GNU Assembler), l'outil de la chaîne GCC qui crée la section <tt>.logindx</tt>. Le compilateur GNU agit à ce niveau comme un simple générateur de code gas. Ainsi, sous Linux (cible au format ELF), GCC va produire la ligne d'assembleur suivante :</p>

<pre>.section    .logindx,"aw",@progbits</pre>

<p>ce qui aboutit à la définition d'une section de données - <tt>"@progbits"</tt> - nommée <tt>.logindx</tt>
et qui dispose des droits en écriture : <tt>"w"</tt> (voir la <a href="http://sources.redhat.com/binutils/docs-2.12/as.info/Section.html">documentation de gas</a> en cible ELF).</p>

<p>Pour modifier cette définition par défaut, on effectue une véritable injection de code <tt>gas</tt> dans GCC, sans oublier le caractère <tt>#</tt> qui permet de marquer ce qui suit comme étant un commentaire :</p>

<?php
$geshi->set_source(<<<CPP
    static const log_info_t * const log_info_ptr
    __attribute__((section(".logindx,\"a\",@nobits#"))) = &log_info;
CPP
);
echo $geshi->parse_code();
?>

<p>ce qui produit :</p>

<pre>.section    .logindx,a,@nobits#,"aw",@progbits</pre>

<p>L'attribut <tt>"w"</tt> ayant été supprimé, la section <tt>.logindx</tt> est créee en lecture seule.</p>

<h4>MinGW</h4>

<p>En revanche il s'agit là de la syntaxe pour une cible ELF (Linux). Or la version Windows des outils GNU manipule des fichiers objets au format COFF (comme VC++). La syntaxe doit donc être légèrement adaptée dans le vas de MinGW :</p>

<?php
$geshi->set_source(<<<CPP
    static const log_info_t * const log_info_ptr
    __attribute__((section(".logindx, \"r\"#"))) = &log_info;
CPP
);
echo $geshi->parse_code();
?>

<p>Ces différences sont traitées dans la <a href="http://sources.redhat.com/binutils/docs-2.12/as.info/Section.html">documentation de gas</a>.</p>

<h4>Définition de macros dédiées</h4>

<p>Au final, nous sommes parvenus à créer une section <tt>.logindx</tt> en lecture seule, aussi bien sous Visual C++ que sous MingW.
Si l'on essaye de modifier notre pointeur comme précédemment :</p>
<pre>
    static const log_info_t * log_info_ptr
    __attribute__((section(".logindx"))) = &log_info;
    
    log_info_ptr = NULL;  
</pre>
<p>Nous obtenons bien une violation d'accès. Le deuxième const est donc de rigueur !</p>

<?php
// footer
$smarty->display('footer.tpl');
?>
