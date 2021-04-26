/*
	filename:pctl.c
	function:父子进程的并发执行
*/
#include "pctl.h"
int main(int argc,char* argv[]){
	int i;
	int pid;//子进程号
	int status;//子进程返回状态
	char* args[]={"/bin/ls","-a",NULL};//子进程要缺省执行的命令
	signal(SIGINT,(sighandler_t)sigcat);//注册一个本进程处理键盘中断的函数

	pid=fork();//建立子进程
	if(pid<0){//建立子进程失败
		printf("create process fail\n");
		exit(EXIT_FAILURE);
	}

	if(pid==0){//子进程执行
		printf("i'm child process %d\nmy father is %d\n\n",getpid(),getppid());
		pause();//暂停，等待键盘中断信号唤醒

		printf("%d child will running:\n",getpid());//唤醒后继续执行

		if(argv[1]!=NULL){//如果在命令行输入了子进程要执行的命令，则执行输入的命令
			for(i=1;argv[i]!=NULL;i++)
				printf("%s",argv[i]);
			printf("\n");
			status=execve(argv[1],&argv[1],NULL);
		}
		else{//没有输入命令，执行缺省命令
			for(i=0;args[i]!=NULL;i++)
				printf("%s",args[i]);
			printf("\n");
			status=execve(args[0],args,NULL);
		}
	}
	else{//父进程执行
		sleep(1);
		printf("i'm father process %d\n",getpid());//报告父进程进程号
		if(argv[1]!=NULL){//如果在命令行输入了子进程要执行的命令，则等待子进程执行结束
			printf("%d waiting for child done\n\n",getpid());
			waitpid(pid,&status,0);//等待子进程结束
			printf("\nmy child exit, status=%d\n",status);
		}
		else{//在命令行上没输入子进程要执行的命令
			//唤醒子进程，与子进程并发执行不等待子进程执行结束
			if(kill(pid,SIGINT)>=0)
				printf("%d wakeup %d child\n",getpid(),pid);
			printf("%d don't wait for child done\n\n",getpid());
		}
	}
	
	return EXIT_SUCCESS;
}
