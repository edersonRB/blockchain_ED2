#include <stdio.h>
#include <string.h>
#include "openssl/crypto.h"
#include "openssl/sha.h"
#include "mtwister.h" //gerador de numeros pseudo-aleatorios

struct BlocoNaoMinerado
{
    unsigned int numero;                              // 4
    unsigned int nonce;                               // 4
    unsigned char data[184];                          // nao alterar. Deve ser inicializado com zeros.
    unsigned char hashAnterior[SHA256_DIGEST_LENGTH]; // 32
};
typedef struct BlocoNaoMinerado BlocoNaoMinerado;

struct BlocoMinerado
{
    BlocoNaoMinerado bloco;
    unsigned char hash[SHA256_DIGEST_LENGTH]; // 32 bytes
};
typedef struct BlocoMinerado BlocoMinerado;

BlocoNaoMinerado gerarBloco(unsigned int numero, MTRand r)
{
    BlocoNaoMinerado bloco;

    unsigned char qtdTransacoes, origem, destino, qtdBitcoin;
    int i, posData = 0;
    qtdTransacoes = (unsigned char)(1000 * genRand(&r)) % 62;

    bloco.numero = numero;

    printf("\n\nBloco %d\n", bloco.numero);

    if (numero != 0)
    {
        for (i = 0; i < qtdTransacoes; i++)
        {
            origem = (unsigned char)(1000 * genRand(&r)) % 256;
            destino = (unsigned char)(1000 * genRand(&r)) % 256;
            qtdBitcoin = ((unsigned char)(1000 * genRand(&r)) % 50) + 1;

            bloco.data[posData++] = origem;
            bloco.data[posData++] = destino;
            bloco.data[posData++] = qtdBitcoin;
        }
    }
    else
    {
        i = 0;
        while (i < 184)
        {
            bloco.data[i++] = 0;
            bloco.data[i++] = 0;
            bloco.data[i++] = 0;
        }
    }

    return bloco;
}

int main()
{
    int numero = 0, i;
    MTRand r = seedRand(1234567);
    // genRand(&r);
    BlocoNaoMinerado bloco = gerarBloco(numero, r);

    for (i = 0; i < 184 && bloco.data[i + 2] != 0; i += 3)
    {
        printf("transacao:%d|%d|%d|%d|\n", i / 3, bloco.data[i], bloco.data[i + 1], bloco.data[i + 2]);
    }

    numero++;
    bloco = gerarBloco(numero, r);
    for (i = 0; i < 184 && bloco.data[i + 2] != 0; i += 3)
    {
        printf("transacao:%d|%d|%d|%d|\n", i / 3, bloco.data[i], bloco.data[i + 1], bloco.data[i + 2]);
    }

    numero++;
    bloco = gerarBloco(numero, r);
    for (i = 0; i < 184 && bloco.data[i + 2] != 0; i += 3)
    {
        printf("transacao:%d|%d|%d|%d|\n", i / 3, bloco.data[i], bloco.data[i + 1], bloco.data[i + 2]);
    }

    return 0;
}