#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9000
#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	char message[BUF_SIZE];
	int str_len;
	int recv_len, recv_cnt;
	pid_t pid;

	if (argc != 3)
	{
		error_handling("Usage : ./SmithChat_client <IP> <name>");
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		error_handling("socket() error");
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port= htons(PORT);

	if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
	{
		error_handling("connect() error!");
	}
	else
	{
		puts("Connected Success!!!");
	}
	
	pid = fork();
	if (pid == 0)
	{
		while (1)
		{
			str_len = recv(sock, message, BUF_SIZE, 0);	
			if (str_len == -1)
			{
				error_handling("read() error!");
			}
			else if (str_len == 0)
			{
				return;
			}
			message[str_len] = 0;
			printf("Message from server : %s \n", message);
		}
	}
	else
	{
		while (1)
		{
			fputs("input: ", stdout);
			fgets(message, BUF_SIZE, stdin);
			
			if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
			{
				shutdown(sock, SHUT_WR);
				return;
			}
			send(sock, message, strlen(message), 0);
		}
	}

	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

