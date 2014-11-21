#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>

#define PORT 9000
#define NAME_SIZE 20
#define BUF_SIZE 1024

void ErrorHandling(char* message)
{
	perror(message);
	exit(1);
}

void* readThread(void* arg);
void* writeThread(void* arg);

char name[NAME_SIZE];

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serverDetail;
	pthread_t t_read, t_write;
	void* threadReturn;

	if (argc != 3)
	{
		ErrorHandling("Usage : ./chatClient <IP> <name>");
	}
	sprintf(name, "[%s]", argv[2]);
	system("clear");

	//socket()
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		ErrorHandling("socket() error");
	}

	memset(&serverDetail, 0, sizeof(serverDetail));
	serverDetail.sin_family = AF_INET;
	serverDetail.sin_addr.s_addr = inet_addr(argv[1]);
	serverDetail.sin_port= htons(PORT);

	//connect()
	if (connect(sock, (struct sockaddr*)&serverDetail, sizeof(serverDetail)) == -1)
	{
		ErrorHandling("connect() error");
	}
	
	//thread create
	if (pthread_create(&t_read, NULL, readThread, (void*)&sock) != 0)
	{
		ErrorHandling("pthread_create Error");
	}
	if (pthread_create(&t_write, NULL, writeThread, (void*)&sock) != 0)
	{
		ErrorHandling("pthread_create Error");
	}
	if (pthread_join(t_read, &threadReturn) != 0)
	{
		ErrorHandling("pthread_join Error");
	}
	if (pthread_join(t_write, &threadReturn) != 0)
	{
		ErrorHandling("pthread_join Error");
	}
	return 0;
}

void* writeThread(void* arg)
{
	int sock = *((int*)arg);
	char buf[BUF_SIZE];
	char message[NAME_SIZE + BUF_SIZE];
	while (1)
	{
		fgets(buf, BUF_SIZE, stdin);
		if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
		{
			close(sock);
			exit(0);
		}
		sprintf(message, "%s %s", name, buf); 
		write(sock, message, strlen(message));
	}
	return NULL;
}

void* readThread(void* arg)
{
	int sock = *((int*)arg);
	char message[NAME_SIZE + BUF_SIZE];
	int strLength;
	while (1)
	{
		strLength = read(sock, message, NAME_SIZE + BUF_SIZE - 1);	
		if (strLength == -1)
		{
			printf("read from server error\n");
		}
		else if (strLength == 0)
		{
			return (void*)-1;
		}
		message[strLength] = 0;
		printf("%s", message);
	}
	return NULL;
}
