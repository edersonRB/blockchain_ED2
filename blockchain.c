#include <stdio.h>
#include <string.h>
#include <limits.h>   // para saber o maior unsigned int UINT_MAX
#include <sys/time.h> // medir o tempo de mineração dos blocos
#include "mtwister.h" //gerador de numeros pseudo-aleatorios
#include "openssl/crypto.h"
#include "openssl/sha.h"

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

BlocoNaoMinerado gerarBloco(unsigned int numero, unsigned char hashAnterior[SHA256_DIGEST_LENGTH], MTRand *r);
void gerarTransacao(BlocoNaoMinerado *bloco, MTRand *r, int *posData);
void efetuarTransacoes(BlocoNaoMinerado *bloco, unsigned int *carteiras);
BlocoMinerado minerarBloco(BlocoNaoMinerado *bloco, int dificuldade);
int hashValido(unsigned char hash[], int dificuldade);
void inicializarCarteiras(unsigned int carteiras[]);
void printTransacoes(BlocoNaoMinerado *bloco);
void printHash(unsigned char hash[], int length);
void printCarteiras(unsigned int carteiras[]);

int encontrarMaximo(unsigned int carteiras[]);

int main()
{
    // Declaracao de variaveis
    FILE *infoFile;   // armazena informacoes
    FILE *chainFile;  // armazena os blocos
    long tamInfoFile; // tamanho do arquivo infoFile

    unsigned int carteiras[256];
    int numero;                                   // numeros dos blocos
    int dificuldade = 2;                          // quantos zeros para validar hash
    int op, qtdMinerar = 0;                       // opcao do menu e quantidade de blocos a serem minerados
    int blocoX;                                   // consultar hash do bloco X - opcao 1
    int endereco;                                 // para consultar determinada carteira - opcao 2
    int i;                                        // indexador generico
    unsigned char anterior[SHA256_DIGEST_LENGTH]; // hash do bloco anterior
    MTRand r = seedRand(1234567);                 // seed do gerador de numeros pseudo-aleatorios

    struct timeval tempo_inicio, tempo_fim; // coletar tempo de execução
    double ti = 0, tf = 0, tempo = 0;
    //######################################

    infoFile = fopen("./infoFile.bin", "rb+");
    chainFile = fopen("./infoFile.bin", "rb+");
    if (!infoFile)
    {
        printf("criando arquivo info...\n");
        infoFile = fopen("./infoFile.bin", "wb+");
    }
    if (!chainFile)
    {
        printf("criando arquivo chain...\n");
        chainFile = fopen("./chainFile.bin", "wb+");
    }

    fseek(infoFile, 0L, SEEK_END); // vai para o fim do arquivo
    tamInfoFile = ftell(infoFile); // le a posicao atual para saber o tamanho do arquivo
    rewind(infoFile);              // volta o ponteiro de leitura de infoFile para o inicio

    if (tamInfoFile == 0)
    {
        printf("Blockchain vazia, criando uma nova\n");
        numero = 0;
        fwrite(&numero, sizeof(int), 1, infoFile);
    }
    else
    {
        fread(&numero, sizeof(int), 1, infoFile);

        fseek(infoFile, sizeof(unsigned char[SHA256_DIGEST_LENGTH]), SEEK_CUR); // desloca 32bytes apos o hash para recuperar o &r
        fread(&r, sizeof(MTRand), 1, infoFile);

        printf("Blockchain  com tamanho %d encontrada, continuando mineracao\n", numero);
    }

    BlocoNaoMinerado bloco;
    BlocoMinerado minerado;

    numero++; // como vai ter lido o tamanho da blockchain (ou 0 se nao existir), soma para minerar o proximo bloco
    printf("numero++:%d\n", numero);

    inicializarCarteiras(carteiras);

    //#############MENU############
    do
    {

        rewind(infoFile);

        fread(&numero, sizeof(int), 1, infoFile);

        fseek(infoFile, sizeof(unsigned char[SHA256_DIGEST_LENGTH]), SEEK_CUR); // desloca 32bytes apos o hash para recuperar o &r
        fread(&r, sizeof(MTRand), 1, infoFile);

        numero++;
    menu:
    {
        printf("\n\t----------------------MENU----------------------\n");
        printf("\t1. Dado o número do bloco qual seu hash?\n");
        printf("\t2. Quantos bitcoins um dado endereço tem?\n");
        printf("\t3. Qual o endereço tem mais bitcoins? \n");
        printf("\t4. Listar endereços com respectivas quantidades\n");
        printf("\t5. Criar e minerar N blocos\n");
        printf("\t0. Sair\n");
        printf("\t------------------------------------------------\n");
        printf("\tDigite sua opcao:");
        scanf("%d", &op);
    }

        // fread(&numero, sizeof(int), 1, infoFile);

        fseek(infoFile, sizeof(unsigned char[SHA256_DIGEST_LENGTH]), SEEK_CUR); // desloca 32bytes apos o hash para recuperar o &r
        fread(&r, sizeof(MTRand), 1, infoFile);

        switch (op)
        {
        case 1:
        {
            printf("numero: %d\n", numero);
            if (numero > 1)
            {
                printf("\tQual bloco deseja saber o hash? ()");
                scanf("%d", &blocoX);

                if (blocoX < 1 || blocoX >= numero)
                {
                    if (numero == 2)
                        printf("\t\tBloco invalido, o unico bloco minerado e o bloco 1\n");
                    else
                        printf("\t\tBloco invalido, deve estar entre 1 e %d\n", numero - 1);
                }
                else
                {
                    fseek(chainFile,sizeof(BlocoMinerado)*(numero-1),);
                }
            }
            else
            {
                printf("\tBlockchain vazia");
            }
            break;
        }
        case 2:
        {
            printf("\tDigite o endereco que deseja consultar (0 ate 255): ");
            scanf("%d", &endereco);
            printf("\t\tO endereco %03d possui %d bitcoins\n", endereco, carteiras[endereco]);
            break;
        }
        case 3:
        {
            printf("\t\tEndereco %03d, com %d bitcoins\n", encontrarMaximo(carteiras), carteiras[encontrarMaximo(carteiras)]);
            break;
        }
        case 4:
        {
            // provisorio
            printf("\t\tCARTEIRAS:\n");
            printCarteiras(carteiras);
            break;
        }
        case 5:
        {
            printf("\tQuantos blocos deseja minerar? (maximo de aproximadamente 13 minutos por bloco):");
            scanf("%d", &qtdMinerar);
            printf("nmero;%d\n", numero);

            for (i = 0; i < qtdMinerar; i++)
            {
                if (numero == 1)
                {
                    bloco = gerarBloco(numero, NULL, &r);
                }
                else
                {
                    fseek(infoFile, sizeof(int), SEEK_SET); // desloca 4bytes (apos o tamanho)
                    fread(anterior, sizeof(unsigned char), SHA256_DIGEST_LENGTH, infoFile);
                    bloco = gerarBloco(numero, anterior, &r);
                }
                printTransacoes(&bloco);

                gettimeofday(&tempo_inicio, NULL);
                minerado = minerarBloco(&bloco, dificuldade);
                gettimeofday(&tempo_fim, NULL);

                ti = tempo_inicio.tv_sec + (tempo_inicio.tv_usec / 1000000.0);
                tf = tempo_fim.tv_sec + (tempo_fim.tv_usec / 1000000.0);
                tempo = tf - ti;
                printf("\ttempo->%f\n", tempo);

                SHA256((unsigned char *)&bloco, sizeof(BlocoNaoMinerado), anterior);

                rewind(infoFile);
                fwrite(&numero, sizeof(int), 1, infoFile);

                fseek(infoFile, sizeof(int), SEEK_SET);                                     // desloca 4bytes (apos o tamanho)
                fwrite(anterior, sizeof(unsigned char[SHA256_DIGEST_LENGTH]), 1, infoFile); // guarda o ultimo hash
                fwrite(&r, sizeof(MTRand), 1, infoFile);

                fseek(chainFile, (numero - 1) * sizeof(BlocoMinerado), SEEK_SET);
                fwrite(&minerado, sizeof(BlocoMinerado), 1, chainFile); // segmentation fault

                numero++;
            }

            break;
        }

        case 0:
        {
            printf("\tSaindo...\n");
            break;
        }
        default:
        {
            printf("\tOpcao invalida!");
            break;
        }
        }

    } while (op);

    fclose(infoFile);

    return 0;
}

BlocoNaoMinerado gerarBloco(unsigned int numero, unsigned char hashAnterior[SHA256_DIGEST_LENGTH], MTRand *r)
{
    BlocoNaoMinerado bloco;

    unsigned char qtdTransacoes;
    int i, posData = 0;
    qtdTransacoes = (unsigned char)(genRandLong(r) % 61) + 1; // PODE GERAR ZERO TRANSACOES

    bloco.numero = numero;
    bloco.nonce = 0;

    printf("\nGerando bloco %d com %d transacoes\n", bloco.numero, qtdTransacoes);

    for (i = 0; i < qtdTransacoes; i++)
    {
        gerarTransacao(&bloco, r, &posData);
    }

    i = qtdTransacoes * 3;
    while (i < 185) // inicializa com zeros
    {
        bloco.data[i++] = 0;
        bloco.data[i++] = 0;
        bloco.data[i++] = 0;
    }

    if (hashAnterior == NULL) // gerando genesis
    {
        for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            bloco.hashAnterior[i] = 0;
        }
        printf("Hash anterior:\n");
        printHash(bloco.hashAnterior, SHA256_DIGEST_LENGTH);
    }
    else
    {
        for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            bloco.hashAnterior[i] = hashAnterior[i];
        }
        printf("Hash anterior:\n");
        printHash(bloco.hashAnterior, SHA256_DIGEST_LENGTH);
    }

    return bloco;
}

void gerarTransacao(BlocoNaoMinerado *bloco, MTRand *r, int *posData)
{
    unsigned int origem, destino, qtdBitcoin;
    origem = (unsigned char)genRandLong(r) % 256;
    destino = (unsigned char)genRandLong(r) % 256;
    qtdBitcoin = (unsigned char)genRandLong(r) % 50 + 1;

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
        SHA256((unsigned char *)bloco, sizeof(BlocoNaoMinerado), hash);
        // printf("nonce:%010d/4294967295\r", bloco->nonce);
        // printHash(hash, SHA256_DIGEST_LENGTH);
    } while (!hashValido(hash, dificuldade) && bloco->nonce < UINT_MAX);

    if (!hashValido(hash, dificuldade))
    {
        printf("\n nonce %d\n", bloco->nonce);
        printHash(hash, SHA256_DIGEST_LENGTH);

        printf("Nao foi possivel encontrar um hash valido, diminuindo dificuldade\n");
        bloco->nonce = 0;
        dificuldade--;
        return minerarBloco(bloco, dificuldade);
    }

    printf("Bloco %d minerado com sucesso!\n", bloco->numero);
    printf("nonce %d\n", bloco->nonce);
    printHash(hash, SHA256_DIGEST_LENGTH);
    BlocoMinerado minerado;
    minerado.bloco = *bloco;
    SHA256((unsigned char *)&bloco, sizeof(bloco), minerado.hash);

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
        carteiras[i] = 0;
    }
}

// printa as transacoes do campo data do bloco
void printTransacoes(BlocoNaoMinerado *bloco)
{
    int i;
    for (i = 0; i < 184 && (bloco->data[i + 2] != 0); i += 3)
    {
        printf("\tTransacao %02d: %03d -> %03d -> %03d\n", (i + 3) / 3, bloco->data[i], bloco->data[i + 1], bloco->data[i + 2]);
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
        printf("\t");
        for (j = 0; j < 8; j++)
        {
            printf("[%03d:%04d]", pos, carteiras[pos]);
            pos++;
        }
        printf("\n");
    }
}

int encontrarMaximo(unsigned int carteiras[])
{
    int maximo = carteiras[0], i;

    for (i = 0; i < 256; i++)
    {
        if (carteiras[i] > maximo)
            maximo = i;
    }

    return maximo;
}