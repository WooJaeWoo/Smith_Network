#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serverSocket;
	int clientSocket;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	char message[BUF_SIZE];
	int str_len, i;

	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

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

	for (i = 0; i < 5; i++)
	{
		clientSocket = accept(serverSocket, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		if (clientSocket == -1)
		{
			error_handling("accept() error");
		}
		else
		{
			printf("Conneted Client %d \n", i + 1);
		}
	
		while ((str_len = read(clientSocket, message, BUF_SIZE)) != 0)
		{
			write(clientSocket, message, str_len);
		}
	
		close(clientSocket);
	}
	close(serverSocket);
	return 0;
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

