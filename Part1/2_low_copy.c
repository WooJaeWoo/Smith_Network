#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 100

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int main()
{
	int fdin;
	int fdout;	
	char buf[BUF_SIZE];

	fdin = open("data.txt", O_RDONLY);
	if (fdin == -1)
	{
		error_handling("open1() error");
	}
	fdout = open("dataCopy.txt", O_CREAT|O_APPEND|O_WRONLY);
	if (fdout == -1)
	{
		error_handling("open2() error");
	}

	if (read(fdin, buf, sizeof(buf)) == -1)
	{
		error_handling("read() error");
	}

	if (write(fdout, buf, sizeof(buf)) == -1)
	{
		error_handling("write() error");
	}

	close(fdout);
	close(fdin);
	return 0;
}
