# blockchain_ED2

- Caso não tenha a biblioteca openssl:

    sudo apt-get install openssl-dev

- Caso nao tenha a biblioteca mtwister compilada:

    gcc -c mtiwster.c

- Compilar o código:

    gcc mtwister.o blockchain.c -lcrypto -o BlockChainMain