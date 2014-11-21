#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 1024
#define MESSAGE_SIZE 100
#define EPOLL_SIZE 50
#define PORT 9000

void ErrorHandling(char* message)
{
	perror(message);
	exit(1);
}

int main(int argc, char* argv[])
{
	int serverSock, clientSock;
	struct sockaddr_in serverDetail, clientDetail;
	char buf[BUF_SIZE];
	char welcomeMessage[MESSAGE_SIZE] = "Hello, world! Connect success! Enjoy chatting!\n";
	char rejectionMessage[MESSAGE_SIZE] = "Sorry, chatting room is full. Please retry few minutes later.\n";
	socklen_t addrSize;
	int i;
	int clientfd;
	int strLength;
	int clientArray[EPOLL_SIZE];
	int arrayLength, logOutIndex;

	struct epoll_event *epEvents;
	struct epoll_event epollEvent;
	int epfd, event_cnt;
	pid_t pid;

	if (argc != 1)
	{
		ErrorHandling("Usage: ./server"); 
	}

	//socket()
	serverSock = socket(PF_INET, SOCK_STREAM, 0);
	if (serverSock == -1)
	{
		ErrorHandling("socket() error");
	}

	memset(clientArray, 0, EPOLL_SIZE * sizeof(int));
	arrayLength = 0;
	memset(&serverDetail, 0, sizeof(serverDetail));
	serverDetail.sin_family = AF_INET;
	serverDetail.sin_addr.s_addr = htonl(INADDR_ANY);
	serverDetail.sin_port = htons(PORT);
	
	//bind()
	if (bind(serverSock, (struct sockaddr*)&serverDetail, sizeof(serverDetail)) == -1)
	{
		ErrorHandling("bind() error");
	}
	
	//listen()
	if (listen(serverSock, 5) == -1)
	{
		ErrorHandling("linsten() error");
	}
	system("clear");
	printf("Server Activated\n");

	//epoll setting
	epfd = epoll_create(EPOLL_SIZE);
	epEvents = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
	epollEvent.events = EPOLLIN;
	epollEvent.data.fd = serverSock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serverSock, &epollEvent);

	while (1)
	{
		//epoll wait
		event_cnt = epoll_wait(epfd, epEvents, EPOLL_SIZE, -1);
		if (event_cnt == -1)
		{
			printf("epoll_wait error\n");
		}

		for (i = 0; i < event_cnt; ++i)
		{
			if (epEvents[i].data.fd == serverSock) //server socket event
			{
				//accept()
				addrSize = sizeof(clientDetail);
				clientSock = accept(serverSock, (struct sockaddr*)&clientDetail, &addrSize);
				if (clientSock == -1)
				{
					printf("accept() error\n");
				}
				if (arrayLength < EPOLL_SIZE - 1)
				{
					write(clientSock, welcomeMessage, MESSAGE_SIZE);
					//add client on epoll
					epollEvent.events = EPOLLIN;
					epollEvent.data.fd = clientSock;
					epoll_ctl(epfd, EPOLL_CTL_ADD, clientSock, &epollEvent);
					clientArray[arrayLength] = clientSock;
					arrayLength++;
					printf("conneted client(%d)\n", clientSock);
				}
				else
				{
					write(clientSock, rejectionMessage, MESSAGE_SIZE);
					close(clientSock);
				}
			}
			else //client socket event
			{
				strLength = read(epEvents[i].data.fd, buf, BUF_SIZE);
				if (strLength <= 0)
				{
					//client close
					epoll_ctl(epfd, EPOLL_CTL_DEL, epEvents[i].data.fd, NULL);
					//logOut re-array
					for (logOutIndex = 0; logOutIndex < arrayLength; logOutIndex++)
					{
						if (clientArray[logOutIndex] == epEvents[i].data.fd)
						{
							break;
						}
					}
					clientArray[logOutIndex] = clientArray[arrayLength - 1];
					arrayLength--;
					close(epEvents[i].data.fd);
					printf("disconnect client (%d)\n", epEvents[i].data.fd);
				}
				else
				{
					//send message to all clients(broadcast)
					for (i = 0; i < arrayLength; i++)
					{
						write(clientArray[i], buf, strLength);
					}
				}
			}
		}
	}
	close(serverSock);
	close(epfd);
	return 0;
}

