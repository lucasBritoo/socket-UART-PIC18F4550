#include <stdio.h> 
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include <pthread.h>

int inicializaSerial(){
	return open("/dev/ttyUSB0", O_RDWR);
}

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

void * threadLeitura(void *arg){
	char read_buf[256];
	int *serial_port = (int *) arg;
	memset(&read_buf, '\0', sizeof(read_buf));

	while(1){
		int num_bytes = read(*serial_port, &read_buf, sizeof(read_buf));

		if(num_bytes < 0) {
			printf("Error reading: %s\n", strerror(errno));
		}
		else {
			printf("Read %i bytes. Received message: %s\n", num_bytes, read_buf);
			strcpy(read_buf, "");
			memset(&read_buf, '\0', sizeof(read_buf));

		}
	}
}

void escreverSerial(int serial_port){
	char msg[3];

	strcpy(msg, "AB");
	write(serial_port, msg, strlen(msg));
}

void lerSerial(int serial_port){
	
}

int main() {
	pthread_t thread1;
	int create, opc = 1;
	long arg =1;
	int serial_port = inicializaSerial();					//abre porta serial

	struct termios tty;

	int connect = configuraSerial(serial_port, &tty);		//configura termios

	if(connect < 0) {
		printf("Conexão rejeitada, verifique erro");
		close(serial_port);
		return 0;
	}

	else{
		create = pthread_create(&thread1, NULL, threadLeitura, (void *) (&serial_port));

		while(opc != 0){
			printf("-------------------------------\n");
			printf("1 - Escrever na serial\n");
			printf("2 - Sair\n");
			scanf("%d", &opc);

			switch(opc){
				case 1:
					escreverSerial(serial_port);
					break;
				case 2:
					printf("Conexão encerrada, fechando o programa...\n");
					pthread_cancel(thread1);
					close(serial_port);
					return 0;
			}
		}
	}

}