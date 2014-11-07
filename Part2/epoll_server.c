#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>

#define BUF_SIZE 16
#define EPOLL_SIZE 64

#define IP "127.0.0.1"
#define PORT 9000

void setnonblocking(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag|O_NONBLOCK);
}

int main(int argc, char* argv[])
{
	int serverSock, clientSock;
	struct sockaddr_in serverDetail, clientDetail;
	char buf[BUF_SIZE];
	FILE* readfp[EPOLL_SIZE];
	FILE* writefp[EPOLL_SIZE];
	socklen_t addrSize;
	int i, j;

	struct epoll_event *ep_events;
	struct epoll_event epollEvent;
	int epfd, event_cnt;

	serverSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serverDetail, 0, sizeof(serverDetail));
	serverDetail.sin_family = AF_INET;
	serverDetail.sin_addr.s_addr = inet_addr(IP);
	serverDetail.sin_port = htons(PORT);

	bind(serverSock, (struct sockaddr*)&serverDetail, sizeof(serverDetail));
	listen(serverSock, 5);

	readfp[serverSock - 3] = fdopen(serverSock, "r");
	writefp[serverSock - 3] = fdopen(serverSock, "w");

	epfd = epoll_create(EPOLL_SIZE);
	ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

	epollEvent.events = EPOLLIN;
	epollEvent.data.fd = serverSock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serverSock, &epollEvent);

	while (1)
	{
		event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
		printf("wait: %d\n", event_cnt);
		if (event_cnt == -1)
		{
			puts("wait error\n");
			break;
		}
		for (i = 0; i < event_cnt; ++i)
		{
			printf("loop %d\n", i);
			if (ep_events[i].data.fd == serverSock)
			{
				printf("Server %d\n", serverSock);
				addrSize = sizeof(clientDetail);
				clientSock = accept(serverSock, (struct sockaddr*)&clientDetail, &addrSize);
				
				setnonblocking(clientSock);
				readfp[clientSock - 4] = fdopen(clientSock, "r");
				writefp[clientSock - 4] = fdopen(clientSock, "w");

				epollEvent.events = EPOLLIN;
				epollEvent.data.fd = clientSock;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clientSock, &epollEvent);
				printf("conneted client: %d\n", clientSock);
			}
			else
			{
				for (j = 1; j < clientSock - 3; j++)
				{
					if (feof(readfp[j]))
					{
						epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
						close(ep_events[i].data.fd);
						printf("disconnect client: %d\n", ep_events[i].data.fd); //??? 닫았는데 어떻게 쓰지?
					}
					else
					{
						printf("client else\n");
						fgets(buf, BUF_SIZE, readfp[j]);
						fputs(buf, writefp[j]);
						fflush(writefp[j]);
						printf("client success!\n");
					}
				}
			}
		}
	}
	printf("closing\n");
	for (i = 0; i < clientSock - 3; i++)
	{
		fclose(writefp[i]);
		fclose(readfp[i]);
	}
	close(serverSock);
	return 0;
}

