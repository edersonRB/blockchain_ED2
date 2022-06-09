// Exemplo de uso da funcao SHA256 da biblioteca Open SSL
//  Este código requer a instalação do pacote libssl-dev
//  Para compilar este código use gcc hash.c -lcrypto

#include "openssl/crypto.h" //arquivo de definição necessário para SHA256_DIGEST_LENGTH
#include "openssl/sha.h"    //arquivo de definição necessário função SHA256
#include <string.h>

struct Pessoa
{
    char nome[20];
    int idade;
}; typedef struct Pessoa Pessoa;

void printHash(unsigned char hash[], int length)
{
    int i;
    for (i = 0; i < length; ++i)
        printf("%02x", hash[i]);

    printf("\n");
}

int main(int argc, char *argv[])
{
    Pessoa p1;
    printf("digite o nome ");
    scanf("%s", p1.nome);
    printf("digite a idade ");
    scanf("%d", &p1.idade);

    unsigned char hash[SHA256_DIGEST_LENGTH]; // vetor que armazenará o resultado do hash. Tamanho definidio pela libssl

    // A funcao SHA256 requer tres parametros
    // 1. o ponteiro para um vetor de unsigned char contendo o dado cujo hash você deseja calcular
    // 2. o tamanho em bytes do dado cujo hash você deseja calcular
    // 3. o ponteiro para um vetor de unsigned char que armazenará o resultado do hash calculado.
    SHA256((unsigned char *)&p1, sizeof(p1), hash);
    printHash(hash, SHA256_DIGEST_LENGTH);
    return 0;
}