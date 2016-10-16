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
$smarty->assign('subtitle', 'Sommaire');
$smarty->assign('datetime', '2012');
$smarty->assign('articletime', 'Novembre 2012');
$smarty->display('header.tpl');
?>

<h3>Sommaire</h3>

<ul>
<li><a href="enregistrer-sans-recopier.php">Enregistrer des messages sans les recopier</a></li>
<li><a href="chaines-literales.php">Stockage des chaînes litérales en C/C++</a></li>
<li><a href="introduction-aux-sections.php">Introduction aux sections en C/C++</a></li>
<li><a href="utilisation-avancee-des-sections.php">Utilisation avancée des sections en C/C++</a></li>
<li><a href="chargement-programme-en-memoire.php">Chargement d\'un programme en mémoire</a></li>
<li><a href="elements-de-syntaxe-conviviale.php">Eléments d'une syntaxe conviviale</a></li>
<li><a href="multithreading.php">Gestion du multi-threading</a></li>
</ul>


<?php
// footer
$smarty->display('footer.tpl');
?>
