#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 16
#define EPOLL_SIZE 64

#define IP "10.73.42.131"
#define PORT 9000

int main(int argc, char* argv[])
{
	int serverSock, clientSock;
	struct sockaddr_in serverDetail, clientDetail;
	char* buf[BUF_SIZE];
	FILE* readfp;
	FILE* writefp;
	socklen_t addrSize;
	int strLength, i;

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

	readfp = fdopen(serverSock, "r");
	writefp = fdopen(serverSock, "w");

	epfd = epoll_create(EPOLL_SIZE);
	ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

	epollEvent.events = EPOLLIN;
	epollEvent.data.fd = serverSock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serverSock, &epollEvent);

	while(1)
	{
		event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
		if (event_cnt == -1)
		{
			puts("wait error\n");
			break;
		}

		for (i = 0; i < event_cnt; ++i)
		{
			if (ep_events[i].data.fd == serverSock)
			{
				addrSize = sizeof(clientDetail);
				clientSock = accept(serverSock, (struct sockaddr*)&clientDetail, &addrSize);
				epollEvent.events = EPOLLIN;
				epollEvent.data.fd = clientSock;
				epoll_ctl(epfd, EPOLL_CLT_ADD, clientSock, &epollEvent);
				printf("conneted client: %d\n", clientSock);
			}
			else //client
			{
				fgets(buf, BUF_SIZE, readfp);
				if (strlen(buf) == 0)
				{
					epoll_ctl(epfd, EPOLL_CLT_DEL, ep_event[i].data.fd, NULL);
					close(ep_events[i].data.fd);
					printf("disconnect client: %d\n", ep_events[i].data.fd); //??? 닫았는데 어떻게 쓰지?
				}
				else
				{
					fputs(buf, writefp);
					fflush(writefp);
				}
			}
		}
	}
	fclose(writefp);
	fclose(readfp);
	close(serverSock);
	return 0;
}

