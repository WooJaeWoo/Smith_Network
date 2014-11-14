#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define BUF_SIZE 1024
#define EPOLL_SIZE 64
#define PORT 9000

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main(int argc, char* argv[])
{
	int serverSock, clientSock, scSock;
	struct sockaddr_in serverDetail, clientDetail;
	char buf[BUF_SIZE];
	socklen_t addrSize;
	int i, j;
	int strLength;
	int clientArray[EPOLL_SIZE];

	struct epoll_event *ep_events;
	struct epoll_event epollEvent;
	int epfd, event_cnt;
	pid_t pid;

	if (argc != 2)
	{
		ErrorHandling("Usage: ./SmithChat(server) <name>");
	}

	serverSock = socket(PF_INET, SOCK_STREAM, 0);
	if (serverSock == -1)
	{
		ErrorHandling("socket() error");
	}
	memset(clientArray, 0, EPOLL_SIZE * sizeof(int));
	memset(&serverDetail, 0, sizeof(serverDetail));
	serverDetail.sin_family = AF_INET;
	serverDetail.sin_addr.s_addr = htonl(INADDR_ANY);
	serverDetail.sin_port = htons(PORT);

	if (bind(serverSock, (struct sockaddr*)&serverDetail, sizeof(serverDetail)) == -1)
	{
		perror("bind p");
		ErrorHandling("bind() error");
	}
	if (listen(serverSock, 5) == -1)
	{
		ErrorHandling("linsten() error");
	}
	printf("Server Activated\n");
	clientArray[serverSock] = 1;

	epfd = epoll_create(EPOLL_SIZE);
	ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

	epollEvent.events = EPOLLIN;
	epollEvent.data.fd = serverSock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serverSock, &epollEvent);

	pid = fork();
	if (pid == 0)
	{
		execlp("./SmithChat_client", "./SmithChat_client", "127.0.0.1", "smith", NULL);
	}
	else
	{
	while (1)
	{
		event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
		if (event_cnt == -1)
		{
			ErrorHandling("epoll_wait() error");
		}
		for (i = 0; i < event_cnt; ++i)
		{
			if (ep_events[i].data.fd == serverSock)
			{
				addrSize = sizeof(clientDetail);
				clientSock = accept(serverSock, (struct sockaddr*)&clientDetail, &addrSize);
				if (clientSock == -1)
				{
					ErrorHandling("accept() Error");
				}
				epollEvent.events = EPOLLIN;
				epollEvent.data.fd = clientSock;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clientSock, &epollEvent);
				clientArray[clientSock] = 1;
				printf("conneted client(%d)\n", clientSock);
			}
			else
			{
				strLength = read(ep_events[i].data.fd, buf, BUF_SIZE);
				if (strLength == 0)
				{
					epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
					clientArray[ep_events[i].data.fd] = 0;
					close(ep_events[i].data.fd);
					printf("disconnect client (%d)\n", ep_events[i].data.fd);
				}
				else
				{
					for (j = 5; j < EPOLL_SIZE; j++)
					{
						if (clientArray[j] == 1)
						{
							write(j, buf, strLength);
						}
					}
				}
			}
		}
	}
	}
	close(serverSock);
	close(epfd);
	return 0;
}

