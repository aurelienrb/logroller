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
$smarty->assign('subtitle', 'Enregistrer des messages sans les recopier');
$smarty->assign('datetime', '2012');
$smarty->assign('articletime', 'Novembre 2012');
$smarty->display('header.tpl');
?>

<!--h1>Lazy logging in C/C++<br />How to record messages without copying them</h1>
<h1>Logger des messages sans en recopier le contenu en C/C++</h1-->

<p>Partant du constat que la plupart des messages de logs sont constitués d'un message texte «&nbsp;en dur&nbsp;», et que la recopie de ce message est l'opération la plus coûteuse, nous allons explorer la possibilité d'enregistrer l'adresse des messages textes plutôt que leur contenu. Les principes et la faisabilité de cette approche «&nbsp;paresseuse&nbsp;» sont traités dans cet article. Divers aspects techniques de l'organisation et du chargement d'un programme C/C++ en mémoire y sont aussi abordés.</p>

<h3>Introduction</h3>

<p>En C/C++, les chaînes de caractères littérales sont incorporées directement au sein du fichier exécutable final dans une zone bien spécifique. Dans le cadre de cet article, nous parlerons de section pour identifier une telle zone, sachant qu'en fonction du format exécutable (<a href="http://en.wikipedia.org/wiki/Portable_Executable">PE</a> sous Windows, <a href="http://en.wikipedia.org/wiki/Executable_and_Linkable_Format">ELF</a> sous Linux, <a href="http://en.wikipedia.org/wiki/Mach-O">Mach-O</a> sous Mac OS X) ce terme peut ou non être interchangeable avec celui de segment.</p>

<p>En général, la section qui contient les chaînes de caractères est en lecture seule, et contient aussi toutes les autres données initialisées constantes (statiques) du programme. Son nom (et ses caractéristiques) dépendent du compilateur / éditeur de liens utilisé, mais traditionnellement il s'agit de <tt>.rdata</tt> (Visual C++) ou encore <tt>.rodata</tt> (GCC). Sous Windows, l'outil <tt>dumpbin</tt> permet de le vérifier :</p>

<p>Sous Linux, on peut utiliser <tt>readelf</tt> :</p>

<pre>linux$ readelf --sections /bin/bash | grep "\["
  [Nr] Name              Type             Address           Offset
  [ 0]                   NULL             0000000000000000  00000000
  [ 1] .interp           PROGBITS         0000000000400238  00000238
  [ 2] .note.ABI-tag     NOTE             0000000000400254  00000254
  [ 3] .note.gnu.build-i NOTE             0000000000400274  00000274
  [ 4] .gnu.hash         GNU_HASH         0000000000400298  00000298
  [ 5] .dynsym           DYNSYM           0000000000403990  00003990
  [ 6] .dynstr           STRTAB           0000000000410560  00010560
  [ 7] .gnu.version      VERSYM           0000000000418930  00018930
  [ 8] .gnu.version_r    VERNEED          0000000000419a30  00019a30
  [ 9] .rela.dyn         RELA             0000000000419ae0  00019ae0
  [10] .rela.plt         RELA             0000000000419ba0  00019ba0
  [11] .init             PROGBITS         000000000041ae90  0001ae90
  [12] .plt              PROGBITS         000000000041aeb0  0001aeb0
  [13] .text             PROGBITS         000000000041bb60  0001bb60
  [14] .fini             PROGBITS         00000000004aabe8  000aabe8
<strong>  [15] .rodata           PROGBITS         00000000004aac00  000aac00</strong>
  [16] .eh_frame_hdr     PROGBITS         00000000004c8390  000c8390
  [17] .eh_frame         PROGBITS         00000000004cc070  000cc070
  [18] .ctors            PROGBITS         00000000006dfe08  000dfe08
  [19] .dtors            PROGBITS         00000000006dfe18  000dfe18
  [20] .jcr              PROGBITS         00000000006dfe28  000dfe28
  [21] .dynamic          DYNAMIC          00000000006dfe30  000dfe30
  [22] .got              PROGBITS         00000000006dffe0  000dffe0
  [23] .got.plt          PROGBITS         00000000006dffe8  000dffe8
  [24] .data             PROGBITS         00000000006e0660  000e0660
  [25] .bss              NOBITS           00000000006e8aa0  000e8a90
  [26] .gnu_debuglink    PROGBITS         0000000000000000  000e8a90
  [27] .shstrtab         STRTAB           0000000000000000  000e8a9c</pre>

<h3>Principe de base de l'astuce</h3>

<p>Au sein d'un module exécutable, l'emplacement des chaînes de caractères est donc figé une bonne fois pour toutes au moment de l'édition des liens. Qu'en est-il de leur adresse finale une fois chargées en mémoire par le système d'exploitation ? Le programme <tt>test1.c</tt> suivant permet de tester si elle varie d'une exécution à l'autre :</p>

<?php
$geshi->set_source(<<<CPP
#include <stdio.h>

int main(int argc, char *argv[])
{
    /* argument "-print <addr>" ? */
    if (argc == 3 && strcmp(argv[1], "-print") == 0)
    {
        /* récupérer l'adresse passée en argument et afficher ce qui s'y trouve */
        void *addr;
        if (scanf("%p", argv[2], &addr) == 1)
        {
            printf("%s\\n", addr);
        }
    }
    else
    {
        /* %p permet d'afficher un pointeur en hexa de façon compatible 32/64 bits */
        printf("%p\\n", "Hello World!");
    }
    return 0;
}
CPP
);

echo $geshi->parse_code();
?>

<p>Cela fonctionne parfaitement sous Linux (kernel 3.2 64 bits) avec GCC 4.6 :</p>
<pre class="console">
$ gcc -Wall -o test1 test1.c
$ test1
47702b
$ test1 -print 47702b
Hello World!
</pre>

<p>Il en est de même sous Windows (XP 32 bits) avec Visual C++ 2010 :</p>
<pre class="console">
D:> cl /W4 -o test1 test1.c
D:> test1
47702b
D:> test1 -print 47702b
Hello World!
</pre>

<p>Le fait que cette chaîne de caractères soit située exactement à la même adresse lors de deux instanciations différentes du même programme ne tient pas au hasard mais n'est pas non plus une garantie, en particulier sur les systèmes intégrant l'<acronym title="Address Space Layout Randomization">ASLR</acronym>. Cet point sera abordé plus en détails par la suite.</p>

<h3>Généralisation de l'astuce</h3>
<p>Selon ce même principe, il est possible de placer en section <tt>.rdata</tt> tout un ensemble d'information et de les référencer via leur adresse au lieu de les recopier. Le programme généralise cette approche aux valeurs des «&nbsp;macros&nbsp;» <tt>__FILE__</tt>, <tt>__LINE__</tt> et  <tt>__FUNCTION__</tt> (non standard mais reconnu à la fois par VC++ et GCC) :</p>

<?php
$geshi->set_source(<<<CPP
#include <stdio.h>

typedef struct
{
    const char * fileName;
    unsigned int lineNumber;
    const char * functionName;
    const char * logMessage;
} log_info_t;

void* test(void)
{
    static const log_info_t test_info = {
        __FILE__,
        __LINE__,
        __FUNCTION__,
        "Hello World!"
    };
    return &test_info;
}

int main(int argc, char *argv[])
{
    /* argument "-print <addr>" */
    if (argc == 3 && strcmp(argv[1], "-print") == 0)
    {
        /* get the address and print what is there */
        void *addr;
        if (scanf("%x", argv[2], &addr) == 1)
        {
            const log_info_t *info = (const log_info_t *)addr;
            printf(
                "[%s:%d] %s %s\\n",
                info->fileName,
                info->lineNumber,
                info->functionName,
                info->logMessage);
        }
    }
    else
    {
        printf("%p\\n", test());
    }
    return 0;
}
CPP
);

echo $geshi->parse_code();
?>

<pre  class="console">
D:\experiments\exp2\debug&gt;exp2.exe
477088
D:\experiments\exp2\debug&gt;exp2.exe -print 477088
[D:\experiments\exp2\exp2.cpp:18] main() Hello World!
</pre>
<p>Pour que cela fonctionne, il faut s'assurer que les données soient bien placées dans une section de données (initialisées) du fichier final.  C'est pourquoi le mot-clé <tt class="keyw">static</tt> est important. Quant au mot-clé <tt class="keyw">const</tt> il permet de cibler une section en lecture seule, mais c'est secondaire comme nous allons le voir.</p>

<?php
// footer
$smarty->display('footer.tpl');
?>
