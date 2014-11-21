#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define SMALL_BUF 500
#define PORT	32000

void *requestHandler(void *arg);
void sendData(FILE* clientWrite, char *contentType, char *fileName);
char *typeHandler(char *file);
void sendError(FILE *clientWrite, int errorNum);


int main(int argc, char *argv[]){
	
	int serverSock, clientSock;
	struct sockaddr_in serverDetail, clientDetail;
	int clientAddrSize;
	char buf[BUF_SIZE];
	pthread_t tid;

	if (argc != 1){
		printf("Usage : %s\n", argv[0]);
		exit(1);
	}

	serverSock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serverDetail, 0, sizeof(serverDetail));
	serverDetail.sin_family = AF_INET;
	serverDetail.sin_addr.s_addr = htonl(INADDR_ANY);
	serverDetail.sin_port = htons(PORT);

	bind(serverSock, (struct sockaddr *)&serverDetail, sizeof(serverDetail));

	listen(serverSock, 10);
	printf("Server Activated\n");
	while(1){
		clientAddrSize = sizeof(clientDetail);
		clientSock = accept(serverSock, (struct sockaddr *)&clientDetail, &clientAddrSize);
		printf("Connection Request: %s\n", inet_ntoa(clientDetail.sin_addr));
		pthread_create(&tid, NULL, requestHandler, &clientSock);
		pthread_detach(tid);
	}
	close(serverSock);
	return 0;
}


void *requestHandler(void *arg){
	int clientSock = *((int *)arg);
	char requestLine[SMALL_BUF];
	FILE *clientRead;
	FILE *clientWrite;
	char method[10];
	char contentType[15];
	char fileName[30];

	clientRead = fdopen(clientSock, "r");
	clientWrite = fdopen(dup(clientSock), "w");
	
	fgets(requestLine, SMALL_BUF, clientRead);
	printf("RL: %s\n", requestLine);
	if(strstr(requestLine, "HTTP/")==NULL){
		sendError(clientWrite, 400);
		fclose(clientRead);
		fclose(clientWrite);
		return;
	}

	strcpy(method, strtok(requestLine, " /"));
	printf("M: %s\n", method);
	strcpy(fileName, strtok(NULL, " /"));
	printf("FN: %s\n", fileName);
	strcpy(contentType, typeHandler(fileName));
	printf("CT: %s\n", contentType);
	if(strcmp(method, "GET") != 0){
		sendError(clientWrite, 400);
		fclose(clientRead);
		fclose(clientWrite);
		return;
	}

	fclose(clientRead);
	sendData(clientWrite, contentType, fileName);
	
}

void sendData(FILE* clientWrite, char *contentType, char *fileName){
	char protocol[] = "HTTP/1.1 200 OK\r\n";
	char server[] = "Server:Linux Web Server \r\n";
	char contentLength[] = "Content-length:2048\r\n";
	char typeMessage[SMALL_BUF];
	char buf[BUF_SIZE];
	FILE *sendFile;

	sprintf(typeMessage, "Content-type:%s\r\n\r\n", contentType);
	sendFile = fopen(fileName, "r");

	if(sendFile == NULL){
		sendError(clientWrite, 404);
		return;
	}

	fputs(protocol, clientWrite);
	fputs(server, clientWrite);
	fputs(contentLength, clientWrite);
	fputs(typeMessage, clientWrite);

	while(fgets(buf, BUF_SIZE, sendFile) != NULL){
		fputs(buf, clientWrite);
		fflush(clientWrite);
	}
	fflush(clientWrite);
	fclose(clientWrite);
}

char *typeHandler(char *file){
	char extension[SMALL_BUF];
	char fileName[SMALL_BUF];
	
	strcpy(fileName, file);
	printf("TH-FN: %s\n", fileName);
	strtok(fileName, ".");
	printf("TH-F: %s\n", fileName);
	strcpy(extension, strtok(NULL, "."));
	printf("TH-EX: %s\n", extension);

	if(!strcmp(extension, "html") || !strcmp(extension, "htm"))
	{
		printf("right type!!!\n");
		return "text/html";
	}
	else
		return "text/plain";
}

void sendError(FILE *clientWrite, int errorNum){
	
	char *protocol;
	char server[] = "Server:Linux Web Server \r\n";
	char contentLength[] = "Content-length:2048\r\n";
	char typeMessage[] = "Content-type:text/html\r\n\r\n";
	char content[]="<html><head><title>ERROR!</title></head>"
		"<body><h1>error</h1></body></html>";

	if(errorNum == 400)
		protocol = "HTTP/1.1 400 Bad Request\r\n";
	else if(errorNum == 404)
		protocol = "HTTP/1.1 404 No Resource\r\n";
	
	fputs(protocol, clientWrite);
	fputs(server, clientWrite);
	fputs(contentLength, clientWrite);
	fputs(typeMessage, clientWrite);
	fputs(content, clientWrite);

	fflush(clientWrite);
}
