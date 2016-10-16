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
$smarty->assign('subtitle', 'Chargement d\'un programme en mémoire');
$smarty->assign('datetime', '2012');
$smarty->assign('articletime', 'Novembre 2012');
$smarty->display('header.tpl');
?>

<p>Nous avons constaté que les chaînes de caractères litérales d'un programme C/C++ sont en général
toujours situées à la même adresse mémoire d'une exécution à l'autre. Mais est-ce systématique ?
</p>

<p>La réponse est non. En théorie, leur emplacement mémoire peut varier d'une exécution à l'autre. En pratique, cela n'arrive pas dans le cas des fichiers exécutables (.exe), mais est très fréquent avec les bibliothèques dynamiques partagées (.dll/.so).</p>

<h3>Sous Windows</h3>

<p>Ces fichiers sont structurés en différentes sections qui compartimentent le code exécutable à proprement parler,
les données initialisées modifiables, les données initialisées constantes, etc.
C'est l'éditeur de liens qui va créer ces sections et les agencer comme il le souhaite, et c'est
le système d'exploitation qui va les charger en mémoire lors de la création d'un nouveau processus.</p>

<p>L'éditeur de liens a déjà défini la disposition relative des sections en mémoire, ainsi qu'une adresse
de base préférentielle. Traditionnellement, sous Windows 32 bits, les exécutables sont chargés à l'adresse
de base 0x400000. Cette valeur est renseignée dans un des nombreux champs du fichier PE.
Les sections présentes vont ensuite être disposées par rapport à cette adresse de base.
Par exemple, si la section .text est renseignée avec l'adresse relative 0x1000, et que l'adresse de base
du fichier en mémoire est 0x400000, alors son adresse finale en mémoire sera 0x400000 + 0x1000 = 0x401000.</p> 

<p>Dans l'exemple, la chaîne de caractère se trouve à l'adresse 0x47702B.
Sachant que le fichier .exe a été chargé à l'adresse de base 0x400000, l'adresse relative de la chaîne
<i>Hello World!</i> est donc : 0x47702B - 0x400000 = 0x7702B.
Si l'on ouvre ce fichier .exe dans un éditeur de fichiers PE, on peut vérifier que cette adresse relative
se situe dans la section .rdata, c'est à dire la section des données initialisées en lecture seule.
C'est dans cette section que les variables const statiques sont placées par l'éditeur de liens de mingw.
C'est aussi le cas avec Visual C++ ?</p> 

<p>Les adresse relatives sont donc constantes, seule l'adresse de base à laquelle elles sont chargées est
fluctuante. Ainsi, lors de sa deuxième exécution dans notre exemple précédent, si l'exécutable
était hypothétiquement chargé à l'adresse 0x500000 au lieu du 0x400000 précédent, alors notre chaîne <i>Hello World!</i>
ne serait plus en 0x47702b mais en 0x57702b, et il en résulterait une gros problème pour son affichage.
C'est quelque chose qui ne se produit normalement pas avec des .exe, mais qui n'est pas exceptionnel dans le cas
des dlls.
Une dll pouvant être chargée par plusieurs .exe différents, qui eux-mêmes chargent d'autres dlls, il se peut
qu'une autre dll occupe déjà une partie du créneau mémoire qu'elle avait spécifié via son adresse de base
préférentielle. Elle sera alors relogée, c'est à dire chargée à un autre endroit, et l'adresse de ses chaînes
de caratères littérales sera donc décalée.</p>

<p>Mais si on connait l'adresse de base de la dll, on peut en déduire l'adresse relative de la chaîne selon
le même principe. Il sera alors facile de retrouver l'adresse réelle de la chaîne même si la dll est
chargée à une adresse de base différente. Qu'en est-il de l'impact de l'<acronym title="Address Space
Layout Randomization">ASLR</acronym> ?
</p>

<?php
// footer
$smarty->display('footer.tpl');
?>
