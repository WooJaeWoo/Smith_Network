#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char**argv)
{
	  int listenfd,connfd,n;
	    struct sockaddr_in servaddr,cliaddr;
		  socklen_t clilen;
		    char mesg[4096];
			  char output[] = "HTTP/1.0 200 OK\r\nDate: Wed, 12 Mar 2014 00:14:10 GMT\r\n\r\n<html><h1>Hello!</h1></html>\r\n";

			    listenfd=socket(AF_INET,SOCK_STREAM,0);

				  memset(&servaddr,0,sizeof(servaddr));
				    servaddr.sin_family = AF_INET;
					  servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
					    servaddr.sin_port=htons(32000);
						  bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

						    listen(listenfd,1024);

							  for(int i = 0; i < 100;i++) {
								      clilen=sizeof(cliaddr);
									      connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
										      printf("connected (%d)\n", connfd);

											      if (connfd > 0) {
													        n = recv(connfd,mesg,4096,0);
															      mesg[n] = 0;
																        printf("=====\n%s=====\n", mesg);
																		      send(connfd,output,strlen(output),0);
																			        close(connfd);
																					    }
												    }
}
