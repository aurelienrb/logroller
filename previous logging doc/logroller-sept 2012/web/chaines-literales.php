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
$smarty->assign('subtitle', 'Stockage des chaînes litérales en C/C++');
$smarty->assign('datetime', '2012');
$smarty->assign('articletime', 'Novembre 2012');
$smarty->display('header.tpl');
?>

<p>Avant de continuer, revenons un instant sur le fait que les chaînes litérales sont stockées dans une section en lecture seule.</p>

<p>Le programme C suivant tente de modifier une chaîne litérale, ce qui du point de vue de la norme aboutit à un comportement indéfini<sup>1</sup> sans pour autant être illégal (en C comment en C++).</p>

<?php
$geshi->set_source(<<<CPP
#include <stdio.h>

int main(void)
{
    char * str = "hello World!";
    str[0] = 'H'; /* Ecriture en mémoire read-only: erreur fatale! */
    printf("%s\\n", str);
    return 0;
}
CPP
);

echo $geshi->parse_code();
?>

<p>Ce code compile en effet sans erreur ni même un warning<sup>2</sup> en C, car la norme prévoit et autorise une conversion depuis une chaîne litérale vers un pointeur non constant. En C++ cette conversion a été rendue obsolète et est donc susceptible de générer un warning. Il semble même qu'elle soit interdite en C++11 bien qu'à ce jour g++ 4.6 (en mode C++11) continue de l'autoriser.</p>

<p>Mais que ce soit C ou en C++, le résultat de l'exécution de ce programme est une violation d'accès (segfault) sur la plupart des systèmes car, comme nous l'avons dit, la section qui contient les chaînes litérales est en lecture seule<sup>3</sup>. On peut vérifier que notre programme compilé contient bien notre chaîne via un simple <tt>grep</tt> :</p>

<pre>$ gcc -o test1 test1.c
$ grep "hello World\!" ./test1
Binary file ./test1 matches
$ ./test1
Segmentation fault
</pre>

<p>Toute la base du procédé repose sur ce fait d'incorporer les chaînes de caractères directement au sein du module compilé. Ceci implique que les chaînes soient de type <i>static storage</i> et non de type tableau de caractères local comme dans l'exemple qui suit :</p>

<?php
$geshi->set_source(<<<CPP
#include <stdio.h>

int main(void)
{
    char str[] = "hello World!";
    str[0] = 'H'; /* Ecriture dans une variable locale: succès! */
    printf("%s\\n", str);
    return 0;
}
CPP
);

echo $geshi->parse_code();
?>

<p>Ce nouveau code est très différent du précédent. La chaîne n'est plus incorporée dans une section du module final mais est construite dynamiquement sur la pile. C'est pourquoi il possible de la modifier. Ceci explique aussi qu'il ne soit plus possible de la trouver via un <tt>grep</tt> sur le fichier généré :</p>

<pre>$ gcc -o test2 test2.c
$ grep "hello World\!" ./test2
$ ./test2
Hello World!
</pre>

<p>En revanche, rendre la variable <tt>str</tt> statique produit une sorte de résultat hybride :</p>
<?php
$geshi->set_source(<<<CPP
#include <stdio.h>

int main(void)
{
    static char str[] = "hello World!";
    str[0] = 'H'; /* Ecriture dans une variable locale statique: succès! */
    printf("%s\\n", str);
    return 0;
}
CPP
);

echo $geshi->parse_code();
?>

<p>Bien que locale à la fonction <tt>main</tt>, la variable <tt>str</tt> n'en demeure pas moins statique et se retrouve donc stockée directement au sein du fichier compilé :</p>

<pre>$ gcc -o test3 test3.c
$ grep "hello World\!" ./test3
Binary file ./test3 matches
$ ./test3
Hello World!
</pre>

<p>Comme on peut le voir, ce nouvel exemple ne provoque pas de violation d'accès, tout simplement parce que la variable <tt>str</tt> n'est pas <tt>const</tt> et n'a donc pas été placée dans la section <tt>.rodata</tt> mais dans la section des données (statiques initialisées) <u>non</u> constantes : <tt>.data</tt>.

<p>On peut le vérifier en comparant les tailles respectives (première colonne) de ces deux sections dans chacun des 3 exemples précédents :</p>

<pre>$ objdump -h test1 | grep data
 14 .rodata       00000011  0000000000400608  0000000000400608  00000608  2**2
 23 .data         00000010  0000000000601010  0000000000601010  00001010  2**3
$ objdump -h test2 | grep data
 14 .rodata       00000004  00000000004006a8  00000000004006a8  000006a8  2**2
 23 .data         00000010  0000000000601018  0000000000601018  00001018  2**3
$ objdump -h test3 | grep data
 14 .rodata       00000004  00000000004005f8  00000000004005f8  000005f8  2**2
 23 .data         00000020  0000000000601010  0000000000601010  00001010  2**3
</pre>

<p>On peut donc conclure cette introduction en retenant que ce qui importe avant tout est que nos données à logger soient de type <tt>static</tt>. Le fait qu'il s'agisse de variables globales ou locales, constantes ou modifiables n'est que secondaire.</p>

<p>Ceci est valable aussi bien pour un binaire (.exe) que pour une bibliothèque partagée (.dll/.so), sous Linux/UNIX comme sous Windows. Il y a en effet peu de différences entre l'un et l'autre au niveau du conteneur exécutable final. Aussi le terme de module [exécutable] sera utilisé pour désigner indifférement ces deux types de fichiers.</p>

<p><font size="0.8">1. «&nbsp;The effect of attempting to modify a string literal is undefined.&nbsp;» (2.13 String literals)</font></p>

<p><font size="0.8">2. GCC dispose du warning <tt>-Wwrite-strings</tt> à ce sujet (<i>initialization discards ‘const’ qualifier from pointer target type</i>) mais il n'est pas activé par défaut en C même si <tt>-Wall</tt> est spécifié. En revanche il est actif par défaut en C++ (<i>deprecated conversion from string constant to ‘char*’</i>).</font></p>

<p><font size="0.8">3. GCC disposait de l'option <tt>-fwritable-strings</tt> pour permettre la modification des chaînes litérales jusqu'à ce qu'elle soit retirée dans la version 4.</font></p>

<?php
// footer
$smarty->display('footer.tpl');
?>
