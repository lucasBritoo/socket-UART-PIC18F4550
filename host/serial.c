#include <stdio.h> 
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PORTA 2000
#define MAX 100
int flag = 0;
int client;

//struct para parâmetro da threadSocket
typedef struct {
	struct sockaddr_in stt_remoto;
	struct sockaddr_in stt_local;
	socklen_t stt_len;
	int stt_sockfd;
}socketHost;

//inicia o socket
int iniciaSocket(struct sockaddr_in *remoto){
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0 ){
		printf("Error %i from socket: %s\n", errno, strerror(errno));
		return -1;
	}
	else{
		printf("Socket criado\n");
		return sockfd;
	}
}

//abre o socket
int configuraSocket(struct sockaddr_in *local, int sockfd){
	local->sin_family = AF_INET;
	local->sin_port = htons(PORTA);
	memset(local->sin_zero, 0x0, 8);

	int abreSocket = bind(sockfd,(struct sockaddr *) local, sizeof(*local));
	if( abreSocket < 0){
		printf("Error %i from bind: %s\n", errno, strerror(errno));
		return -1;
	}
	else{
		int lista = listen(sockfd, 1);

		if(lista < 0 ) {
			printf("Error %i from listen: %s\n", errno, strerror(errno));
			return -1;
		}
		else{
			printf("Aguardando cliente...\n");
			return 0;
		}
	}

}


//inicia a porta serial
int inicializaSerial(){
	return open("/dev/ttyUSB0", O_RDWR);
}

//configura a porta Serial
int configuraSerial(int serial_port, struct termios *tty){

	//leitura da struct termios
	if(tcgetattr(serial_port, tty) != 0){
		printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
		return -1;
	}

	else{

		//configurando a porta serial
		tty->c_cflag &= ~PARENB;
		tty->c_cflag &= ~CSTOPB;
		tty->c_cflag &= ~CSIZE;
		tty->c_cflag |= CS8;
		tty->c_cflag &= ~CRTSCTS;
		tty->c_cflag |= CREAD | CLOCAL;

		tty->c_lflag &= ~ICANON;
		tty->c_lflag &= ~ECHO;
		tty->c_lflag &= ~ECHOE;
		tty->c_lflag &= ~ECHONL;
		tty->c_lflag &= ~ISIG;

		tty->c_iflag &= ~(IXON | IXOFF | IXANY);
		tty->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | ICRNL);

		tty->c_oflag &= ~OPOST;
		tty->c_oflag &= ~ONLCR;

		tty->c_cc[VTIME] = 10;
		tty->c_cc[VMIN] = 0;

		//configurando o baudrate 9600
		cfsetispeed(tty, B9600);
		cfsetospeed(tty, B9600);

		//setando as configurações da serial
		if(tcsetattr(serial_port, TCSANOW, tty) != 0){
			printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
			return -1;
		}
	}

	return 1;
}

//thread para leitura de dados da serial
void * threadLeitura(void *arg){
	char read_buf[256];
	int *serial_port = (int *) arg;
	memset(&read_buf, '\0', sizeof(read_buf));

	while(1){
		int num_bytes = read(*serial_port, &read_buf, sizeof(read_buf));

		if(num_bytes > 0) {
			printf("\nLido %i bytes. Mensagem recebida: %s\n", num_bytes, read_buf);
			strcpy(read_buf, "");
			memset(&read_buf, '\0', sizeof(read_buf));
		}
	}
}

void * threadLeituraClient(void *arg) {

	while(1){
		if(client > 0){
			char leitura[MAX];

			int rec = recv(client, leitura, MAX, 0);

			if(rec > 0){
				leitura[rec] = '\0';
				printf("Cliente: %s\n", leitura);
			}
			else if(flag == 1){
				printf("Cliente desconectado");
				flag = 0;
				pthread_exit(NULL);
			}
		}
	}
}

void aceitaClient(socketHost *host){

	client = accept (host->stt_sockfd, (struct sockaddr *)&(host)->stt_remoto, &(host)->stt_len);

	if(client == -1) {
		printf("Error %i from accept: %s\n", errno, strerror(errno));
		pthread_exit(NULL);

	}
	else {
		printf("\nClient Conectado\n");
		flag = 1;
	}
}


//escrever dados na serial
void escreverSerial(int serial_port){
	char msg[3];

	strcpy(msg, "AB");
	write(serial_port, msg, strlen(msg));
}

void finalizaPrograma(pthread_t *thread1, pthread_t *thread2, int serial_port, int sockfd){
	pthread_cancel(*thread1);
	pthread_cancel(*thread2);
	close(serial_port);
	close(sockfd);
}


int main() {
	pthread_t thread1, thread2;

	struct sockaddr_in local;
	struct sockaddr_in remoto;
	struct termios tty;
	socketHost host;


	int create1, create2, verifica =0,opc = 1;

	int serial_port = inicializaSerial();					//abre porta serial
	int sockfd = iniciaSocket(&remoto);						//inicia socket

	if(serial_port < 0 || sockfd < 0){
		printf("Conexão rejeitada, verifique erro\n");
		finalizaPrograma(&thread1, &thread2, serial_port, sockfd);
		return 0;
	}

	int connect_serial = configuraSerial(serial_port, &tty);		//configura termios
	int connect_socket = configuraSocket(&local, sockfd);		//configura socket


	if(connect_serial < 0 || connect_socket < 0){
		printf("Configuração errada, verifique erro\n");
		finalizaPrograma(&thread1, &thread2, serial_port, sockfd);
		return 0;
	}


	//atribui os valores do socket para a struct socketHost
	host.stt_remoto = remoto;
	host.stt_local  = local;
	host.stt_len    = sizeof(remoto);
	host.stt_sockfd = sockfd;
	
	//cria as threads
	create1 = pthread_create(&thread1, NULL, threadLeitura, (void *) (&serial_port));
	
							
	if(create1 < 0){
		printf("Thread leitura serial com problema, verifique erro\n");
		finalizaPrograma(&thread1, &thread2, serial_port, sockfd);
		return 0;
	}

		while(opc != 0){
			printf("-------------------------------\n");
			printf("1 - Escrever na serial\n");
			printf("2 - Aguardar conexão com cliente\n");
			printf("3 - Sair\n");
			printf("-------------------------------\n");
			printf("Digite a opção: ");
			scanf("%d", &opc);
			printf("\n");

			switch(opc){
				case 1:
					escreverSerial(serial_port);
					break;
				case 2:
					if(flag){
						printf("Cliente já está conectado\n");
					}else{
						printf("Aguardando conexão com client\n");
						
						aceitaClient(&host);
						create2 = pthread_create(&thread2, NULL, threadLeituraClient, (void *)&sockfd);
					}

					break;
					
				case 3:
					printf("Conexão encerrada, fechando o programa...\n");
					finalizaPrograma(&thread1, &thread2, serial_port, sockfd);
					return 0;
			}
		}		
}