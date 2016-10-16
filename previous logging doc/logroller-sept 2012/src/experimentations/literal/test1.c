#include <stdio.h>

int main(void)
{
    char * str = "hello World!";
    str[0] = 'H'; /* Ecriture en mémoire read-only: erreur fatale! */
    printf("%s\n", str);
    return 0;
}
