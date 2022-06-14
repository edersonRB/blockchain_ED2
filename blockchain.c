#include <stdio.h>
#include <limits.h>
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

BlocoNaoMinerado gerarBloco(unsigned int numero, MTRand *r);
void gerarTransacao(BlocoNaoMinerado *bloco, MTRand *r, int *posData);
void efetuarTransacoes(BlocoNaoMinerado *bloco, unsigned int *carteiras);
BlocoMinerado minerarBloco(BlocoNaoMinerado *bloco, int dificuldade);
int hashValido(unsigned char hash[], int dificuldade);
void inicializarCarteiras(unsigned int carteiras[]);
void printTransacoes(BlocoNaoMinerado *bloco);
void printHash(unsigned char hash[], int length);
void printCarteiras(unsigned int carteiras[]);

int main()
{
    int numero = 0, i;
    int dificuldade = 2;
    MTRand r = seedRand(1234567); // seed do gerador de numeros pseudo-aleatorios
    unsigned int carteiras[256];
    inicializarCarteiras(carteiras);

    BlocoNaoMinerado bloco = gerarBloco(numero, &r);
    BlocoMinerado minerado = minerarBloco(&bloco, dificuldade);

    printCarteiras(carteiras);
    efetuarTransacoes(&bloco, carteiras);
    printTransacoes(&bloco);
    printCarteiras(carteiras);

    printTransacoes(&bloco);

    numero++;
    bloco = gerarBloco(numero, &r);
    minerado = minerarBloco(&bloco, dificuldade);
    printCarteiras(carteiras);
    efetuarTransacoes(&bloco, carteiras);
    printTransacoes(&bloco);
    printCarteiras(carteiras);

    // numero++;
    // bloco = gerarBloco(numero, &r);
    // printTransacoes(&bloco);

    return 0;
}

BlocoNaoMinerado gerarBloco(unsigned int numero, MTRand *r)
{
    BlocoNaoMinerado bloco;

    unsigned char qtdTransacoes;
    int i, posData = 0;
    qtdTransacoes = (unsigned char)(1000 * genRand(r)) % 62;

    bloco.numero = numero;
    bloco.nonce = 0;

    printf("\n\nBloco %d\n", bloco.numero);

    if (numero != 0)
    {
        for (i = 0; i < qtdTransacoes; i++)
        {
            gerarTransacao(&bloco, r, &posData);
            //por algum motivo o qtdBitcoin está gerando valores maiores q 50 em alguns casos
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

void gerarTransacao(BlocoNaoMinerado *bloco, MTRand *r, int *posData)
{
    unsigned int origem, destino, qtdBitcoin;
    origem = (unsigned char)(1000 * genRand(r)) % 256;
    destino = (unsigned char)(1000 * genRand(r)) % 256;
    qtdBitcoin =(unsigned char) ((unsigned char)(1000 * genRand(r)) % 50) + 1;

    bloco->data[(*posData)++] = origem;
    bloco->data[(*posData)++] = destino;
    bloco->data[(*posData)++] = qtdBitcoin;
}

BlocoMinerado minerarBloco(BlocoNaoMinerado *bloco, int dificuldade)
{

    unsigned char hash[SHA256_DIGEST_LENGTH]; // hash que deve ser encontrado

    do
    {
        bloco->nonce++;
        SHA256((unsigned char *)bloco, sizeof(bloco), hash);
        // printf("nonce:%d\n", bloco->nonce);
        // printHash(hash, SHA256_DIGEST_LENGTH);
    } while (!hashValido(hash, dificuldade) && bloco->nonce < UINT_MAX);

    if (!hashValido(hash, dificuldade))
    {
        printf("\n nonce %d\n", bloco->nonce);
        printHash(hash, SHA256_DIGEST_LENGTH);

        printf("Nao foi possivel encontrar um hash valido, diminuindo dificuldade\n");
        bloco->nonce = 0;
        minerarBloco(bloco, dificuldade - 1);
    }

    printf("\n\nBloco %d minerado com sucesso!\n", bloco->numero);
    printf("\n nonce %d\n", bloco->nonce);
    printHash(hash, SHA256_DIGEST_LENGTH);
    BlocoMinerado minerado;
    minerado.bloco = *bloco;
    SHA256((unsigned char *)bloco, sizeof(bloco), minerado.hash);

    return minerado;
}

void efetuarTransacoes(BlocoNaoMinerado *bloco, unsigned int *carteiras)
{
    int i;
    unsigned int origem, destino, qtdBitcoin;

    printf("\nEfetuando transacoes\n\n");
    for (i = 0; i < 184 && bloco->data[i + 2] != 0; i += 3)
    {
        origem = bloco->data[i];
        destino = bloco->data[i + 1];
        qtdBitcoin = bloco->data[i + 2];
        printf("Transacao de %03d(%d) para %03d(%03d) com %02d bitcoins\n", origem, carteiras[origem], destino, carteiras[destino], qtdBitcoin);

        carteiras[origem] -= qtdBitcoin;
        carteiras[destino] += qtdBitcoin;
    }
}

int hashValido(unsigned char hash[], int dificuldade)
{
    int i;
    for (i = 0; i < dificuldade; ++i)
        if (hash[i] != 0)
            return 0;
    return 1;
}

void inicializarCarteiras(unsigned int carteiras[])
{
    int i;
    for (i = 0; i < 256; i++)
    {
        carteiras[i] = 50;
    }
}

void printTransacoes(BlocoNaoMinerado *bloco)
{
    int i;
    for (i = 0; i < 184 && bloco->data[i + 2] != 0; i += 3)
    {
        printf("\nTransacao %02d: %03d -> %03d -> %03d", (i + 3) / 3, bloco->data[i], bloco->data[i + 1], bloco->data[i + 2]);
    }
    printf("\n");
}

void printHash(unsigned char hash[], int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

void printCarteiras(unsigned int carteiras[])
{
    int i;
    for (i = 0; i < 256; i++)
    {
        printf("carteira %03d: %d\n", i, carteiras[i]);
    }
}