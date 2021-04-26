/*
reference : https://blog.csdn.net/skyroben/article/details/71513385
pipe[0]指向管道的读端,pipe[1]指向管道的写端。pipe[1]的输出是pipe[0]的输入。管道是⽤环形队列实现的,数据从写端流⼊从读端流出
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int fx(int x){
    if(x<1){
        printf("x must be >= 1\n");
        exit(EXIT_FAILURE);
    }
    else if(x==1)
        return 1;
    else
        return fx(x-1)*x;
}

int fy(int y){
    if(y<1){
        printf("y must be >= 1\n");
        exit(EXIT_FAILURE);
    }
    else if(y==1||y==2)
        return 1;
    else
        return fy(y-1)+fy(y-2);
}

int main(int argc,char* argv[]){
    int x,y;
    printf("enter the value of x,y\n");
    scanf("%d %d",&x,&y);

    int pid1,pid2;//2个并发协作子进程分别完成f(x)、f(y)

    // 4 pipe
    int pipe1[2],pipe2[2];// for child process 1
    int pipe3[2],pipe4[2];// for child process 2
    if(pipe(pipe1)<0){
        perror("pipe1 not create\n");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipe2)<0){
        perror("pipe2 not create\n");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipe3)<0){
        perror("pipe3 not create\n");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipe4)<0){
        perror("pipe4 not create\n");
        exit(EXIT_FAILURE);
    }

    // 3 process
    pid1=fork();
    if(pid1<0){
        perror("process not create\n");
        exit(EXIT_FAILURE);
    }
    if(pid1==0){// child process 1, calculate f(x)
        //从管道1的0端读,管道2的1端写
        close(pipe1[1]);
        close(pipe2[0]);
        int x1,res_fx;
        read(pipe1[0],&x1,sizeof(int));
        printf("child process %d read x=%d\n",getpid(),x1);
        res_fx=fx(x1);
        write(pipe2[1],&res_fx,sizeof(int));
        close(pipe1[0]);
        close(pipe2[1]);
        exit(EXIT_SUCCESS);
    }
    else{//create child process 2
        pid2=fork();
        if(pid2<0){
            perror("process not create\n");
            exit(EXIT_FAILURE);
        }
        if(pid2==0){// child process 2, calculate f(y)
            close(pipe3[1]);
            close(pipe4[0]);
            int y1,res_fy;
            read(pipe3[0],&y1,sizeof(int));
            printf("child process %d read y=%d\n",getpid(),y1);
            res_fy=fy(y1);
            write(pipe4[1],&res_fy,sizeof(int));
            close(pipe3[0]);
            close(pipe4[1]);
            exit(EXIT_SUCCESS);
        }
        else{// father process, calculate f(x+y)
            close(pipe1[0]);
            close(pipe2[1]);
            close(pipe3[0]);
            close(pipe4[1]);
            int res;
            write(pipe1[1],&x,sizeof(int));
            write(pipe3[1],&y,sizeof(int));
            read(pipe2[0],&x,sizeof(int));
            read(pipe4[0],&y,sizeof(int));
            printf("father process %d read x:%d y:%d\n",getpid(),x,y);
            res=x+y;
            printf("the result of f(x+y) is %d\n",res);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe3[1]);
            close(pipe4[0]);
        }
    }
    return EXIT_SUCCESS;
}