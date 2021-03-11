#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include <pthread.h>


#define PORTA 2000
#define MAX 100

int iniciaSocket(){
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0){
		printf("Erro %i from socket: %s\n", errno, strerror(errno));
		return -1;
	}
	else {
		printf("Socket criado\n");
		return sockfd;
	}
}

int configuraSocket(struct sockaddr_in *conexao, int sockfd, int len){
	char ip[32];
	strcpy(ip, "127.0.0.1");

	conexao->sin_family = AF_INET;
	conexao->sin_port   = htons(PORTA);
	inet_pton(AF_INET, ip, &(conexao)->sin_addr);
	memset(conexao->sin_zero, 0x0, 8);

	int conectar = connect(sockfd, (struct sockaddr *) conexao, len);

	if(conectar < 0){
		printf("Erro %i from bind: %s\n", errno, strerror(errno));
		return -1;
	}
	else{
		printf("Conectando ao servidor...\n");
		return 1;
	}
}

void * threadEscreveHost(void *arg){
	int *sockfd = (int *) arg;

	char mensagem[MAX];
	printf("Client: ");
	
	fgets(mensagem, MAX, stdin);
	mensagem[strlen(mensagem) -1] = '\0';

	send(*sockfd, mensagem, strlen(mensagem), 0);
}

void * threadLeituraHost(void *arg){
	int *sockfd = (int *) arg;

	char leitura[MAX];

	int rec = recv(*sockfd, leitura, MAX, 0);
	leitura[rec] = '\0';
	printf("Servidor: %s\n", leitura);
}


void finalizaPrograma(pthread_t *thread1, pthread_t *thread2, int sockfd){
	pthread_cancel(*thread1);
	pthread_cancel(*thread2);
	close(sockfd);
	printf("Cliente Encerrado! \n");
}

int main(){
	pthread_t thread1, thread2;

	struct sockaddr_in conexao;

	int create1, create2, opc =1;

	int sockfd = iniciaSocket();

	if(sockfd < 0) {
		printf("Conexão rejeitada, verifique erro\n");
		finalizaPrograma(&thread1, &thread2, sockfd);
		return 0;
	}

	int connect_socket = configuraSocket(&conexao, sockfd, sizeof(conexao));

	if(connect_socket < 0){
		printf("Configuração errada, verifique erro\n");
		finalizaPrograma(&thread1, &thread2, sockfd);
		return 0;
	}

	create1 = pthread_create(&thread1, NULL, threadEscreveHost, (void *) (&sockfd));
	create2 = pthread_create(&thread2, NULL, threadLeituraHost, (void *) (&sockfd));

	if(create1 < 0 || create2 < 0){
		printf("Threads erradas, verifique erro\n");
		finalizaPrograma(&thread1, &thread2, sockfd);
		return 0;
	}

	while(opc != 2){
		printf("-------------------------------\n");
		printf("2 - Sair\n");
		printf("-------------------------------\n");
		scanf("%d", &opc);

		switch(opc){
			case 2:
				printf("Conexão encerrada, fechando o programa...\n");
				finalizaPrograma(&thread1, &thread2, sockfd);
				return 0;
		}
	}
}