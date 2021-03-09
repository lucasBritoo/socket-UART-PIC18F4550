#include <stdio.h> 
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

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

void escreverSerial(int serial_port){
	char msg[10];

	strcpy(msg, "OLA MUNDO");
	write(serial_port, msg, strlen(msg));
}

void lerSerial(int serial_port){
	char read_buf[256];

	memset(&read_buf, '\0', sizeof(read_buf));

	int num_bytes = read(serial_port, &read_buf, sizeof(read_buf));

	if(num_bytes < 0) {
		printf("Error reading: %s\n", strerror(errno));
	}
	else {
		printf("Read %i bytes. Received message: %s\n", num_bytes, read_buf);
	}
}

int main() {
	int serial_port = inicializaSerial();					//abre porta serial

	struct termios tty;

	int connect = configuraSerial(serial_port, &tty);		//configura termios

	if(connect < 0) {
		printf("Conexão rejeitada, verifique erro");
	}

	else{
		escreverSerial(serial_port);
	
	}

	close(serial_port);
	return 0;
}