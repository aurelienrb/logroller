#include <stdio.h>

int main(void)
{
    char str[] = "hello World!";
    str[0] = 'H'; /* Ecriture dans une variable locale statique: succès! */
    printf("%s\n", str);
    return 0;
}
