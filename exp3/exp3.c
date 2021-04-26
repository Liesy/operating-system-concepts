#include "exp3.h"

int main(int argc,char* argv[]){
	int i;
	int pid_1,pid_2;//子进程号
	int status_1,status_2;//子进程返回状态
	signal(SIGINT,(sighandler_t)sigcat);//注册处理键盘中断的函数
	//子进程要缺省执行的命令
	char* args_1[]={"/bin/ls","-a",NULL};//ls
	char* args_2[]={"/bin/ps","-a",NULL};//ps
	pid_1=fork();//创建子进程1
	
	if(pid_1<0){//创建子进程1失败
		printf("create process 1 fail\n");
		exit(EXIT_FAILURE);
	}
	
	if(pid_1==0){//子进程1执行
		printf("i'm child process ls- %d\nmy father is %d\n\n",getpid(),getppid());
		pause();//暂停，等待键盘中断信号唤醒
		
		printf("%d child will running:\n",getpid());//唤醒后继续执行
		
		//执行ls命令
		for(i=0;args_1[i]!=NULL;i++)
			printf("%s",args_1[i]);
		printf("\n");
		status_1=execve(args_1[0],args_1,NULL);
	}
	else{//子进程1结束，创建子进程2
		sleep(1);
		printf("i'm process %d\n",getpid());//报告进程号
		pid_2=fork();//创建子进程2
		
		if(pid_2<0){//创建子进程1失败
			printf("create process 2 fail\n");
			exit(EXIT_FAILURE);
		}
		
		if(pid_2==0){//子进程2执行
			printf("i'm child process ps- %d\nmy father is %d\ni will running:\n",getpid(),getppid());
			//执行ps命令
			for(i=0;args_2[i]!=NULL;i++)
				printf("%s",args_2[i]);
			printf("\n");
			status_2=execve(args_2[0],args_2,NULL);
		}
		else{//子进程2结束，父进程执行
			sleep(1);
			printf("i'm process %d\n",getpid());//报告进程号
			
			waitpid(pid_2,&status_2,0);//等待子进程2结束
			printf("child process 2 over, status=%d\n",status_2);
			
			if(kill(pid_1,SIGINT)>=0)//唤醒子进程1
				printf("%d wakeup %d child\n",getpid(),pid_1);
			waitpid(pid_1,&status_1,0);//等待子进程1结束
			printf("child process 1 over, status=%d\n",status_1);
			printf("father process over%d\n",getpid());
		}
	}
	return EXIT_SUCCESS;
}

