#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30

void error_handling(char* message);

int main(int argc, char *argv[])
{
	int serverSocket;
	int clientSocket;
	FILE* fp;
	struct sockaddr_in serv_addr, clnt_addr;
	socklen_t clnt_addr_size;
	char buf[BUF_SIZE];
	int read_cnt;

	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	fp = fopen("file_server.c", "rb");
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		error_handling("socket() error");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if (bind(serverSocket, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
	{
		error_handling("bind() error");
	}

	if (listen(serverSocket, 5) == -1)
	{
		error_handling("listen() error");
	}

	clnt_addr_size = sizeof(clnt_addr);
	clientSocket = accept(serverSocket, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
	while(1)
	{
		read_cnt = fread((void *)buf, 1, BUF_SIZE, fp);
		if (read_cnt < BUF_SIZE)
		{
			write(clientSocket, buf, read_cnt);
			break;
		}
		write(clientSocket, buf, BUF_SIZE);
	}
	shutdown(clientSocket, SHUT_WR);
	read(clientSocket, buf, BUF_SIZE);
	printf("Message from client: %s \n", buf);

	fclose(fp);
	close(clientSocket);
	close(serverSocket);
	return 0;
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

