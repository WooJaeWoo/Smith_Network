#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char* argv[])
{
	int sockServer;
	struct sockaddr_in serverAddr;
	char buf[BUF_SIZE];
	int strLength;
	int recvLength;
	int recvCount;

	if (argc != 3)
	{
		printf("argument error!\n");
		exit(1);
	}

	sockServer = socket(PF_INET, SOCK_STREAM, 0);
	if (sockServer == -1)
		error_handling("socket() error");

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddr.sin_port = htons(atoi(argv[2]));

	if ((connect(sockServer, (struct sockaddr*) &serverAddr, sizeof(serverAddr))) == -1)
		error_handling("connect() error");
	else
		puts("Connect Success");

	while (1)
	{
		fputs("Input : ", stdout);
		fgets(buf, BUF_SIZE, stdin);
		if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
			break;
		strLength = write(sockServer, buf, strlen(buf));

		recvLength = 0;
		while (recvLength < strLength)
		{
			recvCount = read(sockServer, &buf[recvLength], BUF_SIZE-1);
			recvLength += recvCount;
		}
		buf[recvLength] = 0;
		printf("Message from Server : %s", buf);
	}
	close(sockServer);
	return 0;
}

