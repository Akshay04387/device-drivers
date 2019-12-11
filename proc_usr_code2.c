//user space code for the procfs to get the pid of the process

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <linux/fcntl.h>
#include <stdlib.h>

int main()
{
	int fd,ret,buf[1024];

	fd =open("/proc/check/get_pid_tgid",O_RDONLY);
	if(fd<0){
		perror("error in opening\n");
		exit(1);
	}

	printf("value of fd is %d\n",fd);
	while(1){
		ret = read(fd,buf,4096);
		if(ret <0){
			perror("error in read\n");
			exit(-6);
		}
		if(ret==0){
			printf("completion of the reading\n");
			break;
		}
		if(ret>0){
			write(STDOUT_FILENO,buf,ret);
		}
	}
	exit(0);
}
