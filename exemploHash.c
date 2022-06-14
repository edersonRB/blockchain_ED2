// Exemplo de uso da funcao SHA256 da biblioteca Open SSL
//  Este código requer a instalação do pacote libssl-dev
//  Para compilar este código use gcc hash.c -lcrypto

#include <limits.h>
#include "openssl/crypto.h" //arquivo de definição necessário para SHA256_DIGEST_LENGTH
#include "openssl/sha.h"    //arquivo de definição necessário função SHA256
#include <string.h>
#include <sys/time.h>

struct Pessoa
{
    char nome[20];
    int idade;
};
typedef struct Pessoa Pessoa;

void printHash(unsigned char hash[], int length)
{
    int i;
    for (i = 0; i < length; ++i)
        printf("%d ", hash[i]);

    printf("\n");
}

int hashValido(unsigned char hash[], int dificuldade)
{
    int i;
    for (i = 0; i < dificuldade; ++i)
        if (hash[i] != 0)
            return 0;
    return 1;
}

int main(int argc, char *argv[])
{
    unsigned char hash[SHA256_DIGEST_LENGTH]; // vetor que armazenará o resultado do hash. Tamanho definidio pela libssl
    int i = INT_MIN;
    Pessoa p1;
    // printf("digite o nome ");
    strcpy(p1.nome, "Joao");
    // printf("digite a idade ");
    struct timeval tempo_inicio, tempo_fim; // coletar tempo de execução

    double ti, tf, tempo;

    int dificuldade = 4;

    gettimeofday(&tempo_inicio, NULL);
    do
    {
        p1.idade = i++;
        SHA256((unsigned char *)&p1, sizeof(p1), hash);
    } while (!hashValido(hash, dificuldade));

    printf("hash valido %d\n", i);
    printHash(hash, SHA256_DIGEST_LENGTH);

    gettimeofday(&tempo_fim, NULL);
    tf = (double)tempo_fim.tv_usec + ((double)tempo_fim.tv_sec * (1000000.0));
    ti = (double)tempo_inicio.tv_usec + ((double)tempo_inicio.tv_sec * (1000000.0));
    tempo = (tf - ti) / 1000;

    printf("tempo de execucao %f\n", tempo);

    return 0;
}