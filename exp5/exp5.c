#include<stdio.h>
#include<stdlib.h>
#include<sched.h>
#include<sys/time.h>
#include<sys/resource.h>
#include<signal.h>

typedef void(*sighandler_t)(int);
void sigcat(){//+1
    setpriority(PRIO_PROCESS,getpid(),getpriority(PRIO_PROCESS,0)+1);
}
void sigzat(){//-1
    setpriority(PRIO_PROCESS,getpid(),getpriority(PRIO_PROCESS,0)-1);
}
void do_nothing(){}

int main(int argc,char* argv[]){
    int i,j,status;
    int pid;
    struct sched_param p[2];
    for(i=0;i<2;i++)
        p[i].sched_priority=10;

    pid=fork();
    if(pid<0){
        printf("create process failed\n");
        exit(EXIT_FAILURE);
    }
    if(pid==0){
        signal(SIGINT,(sighandler_t)sigcat);//程序终止(interrupt)信号, 在用户键入INTR字符(通常是Ctrl-C)时发出，用于通知前台进程组终止进程。
        signal(SIGTSTP,(sighandler_t)sigzat);//停止进程的运行, 但该信号可以被处理和忽略. 用户键入SUSP字符时(通常是Ctrl-Z)发出这个信号
        sched_setscheduler(getpid(),SCHED_OTHER,&p[1]);
        setpriority(PRIO_PROCESS,getpid(),10);
        sleep(1);

        for(i=0;i<10;i++){
            printf("child process pid=%d, priority=%d, scheduler=%d\n",getpid(),getpriority(PRIO_PROCESS,0),sched_getscheduler(getpid()));
            pause();
        }
    }
    else{
        signal(SIGINT,(sighandler_t)sigcat);
        signal(SIGTSTP,(sighandler_t)sigzat);
        sched_setscheduler(getpid(),SCHED_OTHER,&p[0]);
        setpriority(PRIO_PROCESS,getpid(),10);
        sleep(1);

        for(i=0;i<10;i++){
            printf("father process pid=%d, priority=%d, scheduler=%d\n",getpid(),getpriority(PRIO_PROCESS,0),sched_getscheduler(getpid()));
            pause();
        }
        exit(EXIT_SUCCESS);
    }
    return EXIT_SUCCESS;
}