#include <stdio.h>
#include <string.h>
#include <limits.h> // para saber o maior unsigned int UINT_MAX
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

struct BlockChain
{
    BlocoMinerado *bloco;
    unsigned int dificuldade;
    struct BlockChain *proximoBloco;
};
typedef struct BlockChain BlockChain;

BlocoNaoMinerado gerarBloco(unsigned int numero, MTRand *r);
void gerarTransacao(BlocoNaoMinerado *bloco, MTRand *r, int *posData);
void efetuarTransacoes(BlocoNaoMinerado *bloco, unsigned int *carteiras);
BlocoMinerado minerarBloco(BlocoNaoMinerado *bloco, int *dificuldade);
int hashValido(unsigned char hash[], int dificuldade);
void inicializarCarteiras(unsigned int carteiras[]);
void printTransacoes(BlocoNaoMinerado *bloco);
void printHash(unsigned char hash[], int length);
void printCarteiras(unsigned int carteiras[]);

BlockChain *alocaNo(BlocoMinerado *blocoMinerado, unsigned int dificuldade1)
{
    BlockChain *novo = (BlockChain *)malloc(sizeof(BlockChain));
    if (novo == NULL)
        return NULL;
    novo->bloco = blocoMinerado;
    novo->dificuldade = dificuldade1;
    novo->proximoBloco = NULL;
    return novo;
}

void inserirBlockChain(BlockChain **chain, BlocoMinerado bloco, unsigned int dificuldade)
{
    if (*chain == NULL)
    {
        *chain = alocaNo(&bloco, dificuldade);
        return;
    }
    BlockChain *aux = *chain;
    while (aux->proximoBloco != NULL)
        aux = aux->proximoBloco;
    aux = alocaNo(&bloco, dificuldade);
}

int main()
{
    BlockChain *blockChain = NULL;
    unsigned int carteiras[256];
    int numero = 0;
    unsigned int *dificuldade;
    *dificuldade = 4;
    MTRand r = seedRand(1234567); // seed do gerador de numeros pseudo-aleatorios

    inicializarCarteiras(carteiras);

    BlocoNaoMinerado bloco = gerarBloco(numero, &r);
    BlocoMinerado minerado = minerarBloco(&bloco, dificuldade);
    inserirBlockChain(&blockChain, minerado, *dificuldade);

    numero++;
    bloco = gerarBloco(numero, &r);
    minerado = minerarBloco(&bloco, dificuldade);
    printCarteiras(carteiras);
    efetuarTransacoes(&bloco, carteiras);
    printTransacoes(&bloco);
    printCarteiras(carteiras);

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

    printf("\nGerando bloco %d\n", bloco.numero);

    i = 0;
    while (i < 184) // inicializa com zeros
    {
        bloco.data[i++] = 0;
        bloco.data[i++] = 0;
        bloco.data[i++] = 0;
    }
    if (numero != 0) // se nao for o primeiro bloco
    {
        for (i = 0; i < qtdTransacoes; i++)
        {
            gerarTransacao(&bloco, r, &posData);
        }
    }

    return bloco;
}

void gerarTransacao(BlocoNaoMinerado *bloco, MTRand *r, int *posData)
{
    unsigned int origem, destino, qtdBitcoin;
    origem = (unsigned char)(1000 * genRand(r)) % 256;
    destino = (unsigned char)(1000 * genRand(r)) % 256;
    qtdBitcoin = (unsigned char)((unsigned char)(1000 * genRand(r)) % 50) + 1;

    bloco->data[(*posData)++] = origem;
    bloco->data[(*posData)++] = destino;
    bloco->data[(*posData)++] = qtdBitcoin;
}

BlocoMinerado minerarBloco(BlocoNaoMinerado *bloco, int *dificuldade)
{
    unsigned char hash[SHA256_DIGEST_LENGTH]; // hash que deve ser encontrado

    do
    {
        bloco->nonce++;
        SHA256((unsigned char *)bloco, sizeof(bloco), hash);
        // printf("nonce:%d\n", bloco->nonce);
        // printHash(hash, SHA256_DIGEST_LENGTH);
    } while (!hashValido(hash, *dificuldade) && bloco->nonce < UINT_MAX);

    if (!hashValido(hash, *dificuldade))
    {
        printf("\n nonce %d\n", bloco->nonce);
        printHash(hash, SHA256_DIGEST_LENGTH);

        printf("Nao foi possivel encontrar um hash valido, diminuindo dificuldade\n");
        bloco->nonce = 0;
        *dificuldade--;
        return minerarBloco(bloco, dificuldade);
    }

    printf("Bloco %d minerado com sucesso!\n", bloco->numero);
    printf("nonce %d\n", bloco->nonce);
    printHash(hash, SHA256_DIGEST_LENGTH);
    BlocoMinerado minerado;
    minerado.bloco = *bloco;
    SHA256((unsigned char *)bloco, sizeof(bloco), minerado.hash);

    return minerado;
}

// diminui e aumenta respectivamente os valores das carteiras de origem e destino
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
        carteiras[origem] = (int)carteiras[origem] - (int)qtdBitcoin >= 0 ? carteiras[origem] - qtdBitcoin : 0;
        carteiras[destino] += qtdBitcoin;
    }
}

// retorna 1 se o hash for valido, 0 caso contrario
int hashValido(unsigned char hash[], int dificuldade)
{
    int i;
    for (i = 0; i < dificuldade; ++i)
        if (hash[i] != 0)
            return 0;
    return 1;
}

// inicializa as carteiras com 50 bitcoins
void inicializarCarteiras(unsigned int carteiras[])
{
    int i;
    for (i = 0; i < 256; i++)
    {
        carteiras[i] = 50;
    }
}

// printa as transacoes do campo data do bloco
void printTransacoes(BlocoNaoMinerado *bloco)
{
    int i;
    for (i = 0; i < 184 && bloco->data[i + 2] != 0; i += 3)
    {
        printf("\nTransacao %02d: %03d -> %03d -> %03d", (i + 3) / 3, bloco->data[i], bloco->data[i + 1], bloco->data[i + 2]);
    }
    printf("\n");
}

// printa um hash
void printHash(unsigned char hash[], int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

// printa as carteiras
void printCarteiras(unsigned int carteiras[])
{
    int i, j, pos = 0;
    for (i = 0; i < 32; i++)
    {
        for (j = 0; j < 8; j++)
        {
            printf("[%03d:%04d]", pos, carteiras[pos]);
            pos++;
        }
        printf("\n");
    }
}