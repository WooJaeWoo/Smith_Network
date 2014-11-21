#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>

#define BUF_SIZE 4096
#define SMALL_BUF 100
#define PORT	32000

void ErrorHandling(char* message)
{
	perror(message);
	exit(1);
}

int main(int argc, char *argv[]){
	
	int serverSock, clientSock;
	struct sockaddr_in serverDetail, clientDetail;
	socklen_t clientAddrSize;
	int fd, errfd, strLength;
	char buf[BUF_SIZE];
	pthread_t tid;
	char header[] = "HTTP/1.1 200 OK\r\nDate: Wed, 12 Mar 2014 00:00:00 GMT\r\n\r\n";
	char method[10];
	char fileName[30];
	char protocol[SMALL_BUF];

	if (argc != 1){
		printf("Usage : %s\n", argv[0]);
		exit(1);
	}

	serverSock = socket(PF_INET, SOCK_STREAM, 0);
	if (serverSock == -1)
	{
		ErrorHandling("socket() error");
	}

	memset(&serverDetail, 0, sizeof(serverDetail));
	serverDetail.sin_family = AF_INET;
	serverDetail.sin_addr.s_addr = htonl(INADDR_ANY); 
	serverDetail.sin_port = htons(PORT);
	
	if (bind(serverSock, (struct sockaddr *)&serverDetail, sizeof(serverDetail)) == -1)
	{
		ErrorHandling("bind() error");
	}
	if (listen(serverSock, 10) == -1)
	{
		ErrorHandling("listen() error");
	}
	printf("Server Activated\n");
	
	while (1)
	{
		clientAddrSize = sizeof(clientDetail);
		clientSock = accept(serverSock, (struct sockaddr *)&clientDetail, &clientAddrSize);
		if (clientSock == -1)
		{
			perror("accept() error");
		}
		printf("Connection Request: %s\n", inet_ntoa(clientDetail.sin_addr));
		strLength = recv(clientSock, buf, BUF_SIZE, 0);
		buf[strLength] = 0;
		
		//Request Parse
		strcpy(fileName, ".");
		strcpy(method, strtok(buf, " "));
		strcat(fileName, strtok(NULL, " "));
		
		//Default Page
		if (fileName[strlen(fileName) - 1] == '/')
		{
			strcat(fileName, "index.html");
		}
		fd = open(fileName, O_RDONLY);
		
		//404 Error
		if (fd == -1)
		{
			strcpy(protocol, "HTTP/1.1 404 Not Found\r\n\r\n");
			send(clientSock, protocol, strlen(protocol), 0);
			errfd = open("error404.html", O_RDONLY);
			while ((strLength = read(errfd, buf, BUF_SIZE)) > 0)
			{
				send(clientSock, buf, strLength, 0);
			}
			close(errfd);
		}
		//Send
		else
		{
			while ((strLength = read(fd, buf, BUF_SIZE)) > 0)
			{
				send(clientSock, buf, strLength, 0);
			}
		}
		close(fd);
		close(clientSock);
	}
	close(serverSock);
	return 0;
}

