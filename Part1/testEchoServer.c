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
	int sockClient;
	struct sockaddr_in serverAddr, clientAddr;
	char* buf[BUF_SIZE];
	socklen_t clientAddrSize;
	int strLength;
	int i;
	
	if (argc != 2)
	{
		printf("argument error!\n");
		exit(1);
	}

	sockServer = socket(PF_INET, SOCK_STREAM, 0);
	if (sockServer == -1)
		error_handling("socket() error");

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(atoi(argv[1]));

	if ((bind(sockServer, (struct sockaddr*) &serverAddr, sizeof(serverAddr))) == -1)
		error_handling("bind() error");

	if ((listen(sockServer, 3)) == -1)
		error_handling("listen() error");

	clientAddrSize = sizeof(&clientAddr);

	for (i = 0; i < 5; i++)
	{
		sockClient = accept(sockServer, (struct sockaddr*) &clientAddr, &clientAddrSize);
		if (sockClient == -1)
			error_handling("accept() error");
		
		while((strLength = read(sockClient, buf, BUF_SIZE)) != 0)
		{
			write(sockClient, buf, strLength);
		}
	
		close(sockClient);
	}
	close(sockServer);
	return 0;
}

