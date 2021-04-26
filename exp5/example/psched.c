#include<stdio.h>
#include<stdlib.h>
#include<sched.h>
#include<sys/time.h>
#include<sys/resource.h>

int main(int argc,char* argv[]){
    int i,j,status;
    int pid[3];//pids of three child processes
    struct sched_param p[3];//data structure for setting priority

    //create three child process
    for(i=0;i<3;i++){
        if((pid[i]=fork())>0){//father process set priorities for child processes
            p[i].sched_priority=(argv[i+1]!=NULL)?atoi(argv[i+1]):10;
            sched_setscheduler(pid[i],(argv[i+4]!=NULL)?atoi(argv[i+4]):SCHED_OTHER,&p[i]);
            setpriority(PRIO_PROCESS,pid[i],(argv[i+1]!=NULL)?atoi(argv[i+1]):10);
        }
        else{
            sleep(1);
            //announce pid and priority per minute
            for(i=0;i<10;i++){
                printf("child pid=%d, priority=%d\n",getpid(),getpriority(PRIO_PROCESS,0));
                sleep(1);
            }
            exit(EXIT_SUCCESS);
        }
    }

    for(i=0;i<3;i++)
        printf("my child %d's policy is %d\n",pid[i],sched_getscheduler(pid[i]));
    
    return EXIT_SUCCESS;
}