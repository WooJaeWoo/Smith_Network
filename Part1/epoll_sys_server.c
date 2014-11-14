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
	socklen_t addrSize;
	int i;
	int strLength;

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

	epfd = epoll_create(EPOLL_SIZE);
	ep_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

	epollEvent.events = EPOLLIN;
	epollEvent.data.fd = serverSock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serverSock, &epollEvent);

	while (1)
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
				epoll_ctl(epfd, EPOLL_CTL_ADD, clientSock, &epollEvent);
				printf("conneted client: %d\n", clientSock);
			}
			else
			{
				strLength = read(ep_events[i].data.fd, buf, BUF_SIZE);
				if (strLength == 0)
				{
					epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
					close(ep_events[i].data.fd);
					printf("disconnect client: %d\n", ep_events[i].data.fd); //??? 닫았는데 어떻게 쓰지?
				}
				else
				{
					write(ep_events[i].data.fd, buf, strLength);
				}
			}
		}
	}
	close(serverSock);
	close(epfd);
	return 0;
}

