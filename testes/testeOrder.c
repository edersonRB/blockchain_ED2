#include <stdio.h>
#include <stdlib.h>

void inicializarCarteiras(unsigned int carteiras[])
{
    int i;
    for (i = 0; i < 256; i++)
    {
        carteiras[i] = (unsigned int)(25500.0 / i);
    }
}

void printCarteiras(unsigned int carteiras[])
{
    int i, j, pos = 0;
    for (i = 0; i < 32; i++)
    {
        for (j = 0; j < 8; j++)
        {
            printf("[%03d:%06d]", pos, carteiras[pos]);
            pos++;
        }
        printf("\n");
    }
}

void printInorder(unsigned int carteiras[])
{
    int i, j, pos = 0, temp;

    unsigned int posicoesEmOrdem[256], indexTemp;

    unsigned int carteirasEmOrdem[256];

    for (i = 0; i < 256; i++)
    {
        carteirasEmOrdem[i] = carteiras[i];
        posicoesEmOrdem[i] = i;
    }

    for (i = 0; i < 256; i++)
    {
        for (j = 0; j < 255; j++)
        {
            if (carteirasEmOrdem[j] > carteirasEmOrdem[j + 1])
            {
                temp = carteirasEmOrdem[j];
                indexTemp = j;

                carteirasEmOrdem[j] = carteirasEmOrdem[j + 1];
                carteirasEmOrdem[j + 1] = temp;

                posicoesEmOrdem[j] = posicoesEmOrdem[j + 1];
                posicoesEmOrdem[j + 1] = indexTemp;
            }
        }
    }

    for (i = 0; i < 256; i++)
    {
        printf("[%03d:%04d]\n", posicoesEmOrdem[i], carteirasEmOrdem[i]);
        // printf("[%05d]\n", carteirasEmOrdem[i]);
    }
}

int main()
{
    unsigned int carteiras[256];

    inicializarCarteiras(carteiras);

    printCarteiras(carteiras);

    printf("\n\n\n");

    printInorder(carteiras);

    return 0;
}