#include <stdio.h>
#include <unistd.h> 
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	int status;
	pid_t pid=fork();
	
	if(pid==0){
		sleep(5);
		printf("I'm child process. I'm terminated!!\n");
		return 24;   	
	}
	else{
		printf("Child PID: %d \n",pid);
		while(!waitpid(-1, &status, WNOHANG)){
			sleep(1);
			puts("sleep 1sec.");
		}
		printf("Child Process (PID:%d is terminated",pid);
		printf("30 sec timer is started!\n");
		sleep(30);

		if(WIFEXITED(status))
			printf("Child send %d \n", WEXITSTATUS(status));
	}
	return 0;
}