//利用管道实现在父子进程间传递整数
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc,char* argv[]){
    int pid;//进程号
    int pipe1[2],pipe2[2];//存放两个无名管道标号
    int x;//存放要传递的整数

    //使用pipe()系统调用建立两个无名管道
    if(pipe(pipe1)<0){
        perror("pipe1 not create\n");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipe2)<0){
        perror("pipe2 not create\n");
        exit(EXIT_FAILURE);
    }

    pid=fork();
    if(pid<0){
        perror("process not create\n");
        exit(EXIT_FAILURE);
    }
    if(pid==0){//子进程负责从管道1的0端读,管道2的1端写
        //关掉管道1的1端和管道2的0端
        close(pipe1[1]);
        close(pipe2[0]);

        //每次循环从管道1的0端读一个整数放入变量X中,对X加1后写入管道2的1端，直到X大于10
        do{
            read(pipe1[0],&x,sizeof(int));
            printf("child process %d read %d\n",getpid(),x++);
            write(pipe2[1],&x,sizeof(int));
        }while(x<=9);

        //读写完成后,关闭管道
        close(pipe1[0]);
        close(pipe2[1]);

        //子进程执行结束
        exit(EXIT_SUCCESS);
    }
    else{//父进程负责从管道2的0端读,管道1的1端写
        //关掉管道1的0端和管道2的1端
        close(pipe1[0]);
        close(pipe2[1]);

        //每次循环向管道1的1端写入变量X的值,并从管道2的0端读一整数写入X再对X加1，直到X大于10
        x=1;
        do{
            write(pipe1[1],&x,sizeof(int));
            read(pipe2[0],&x,sizeof(int));
            printf("father process %d read %d\n",getpid(),x++);
        }while(x<=9);

        close(pipe1[1]);
        close(pipe2[0]);
    }
    return EXIT_SUCCESS;
}