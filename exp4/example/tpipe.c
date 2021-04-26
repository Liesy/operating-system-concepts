/*
利用管道实现线程间的整数传递
pthread不是Linux下的默认的库，也就是在链接的时候，无法找到phread库中哥函数的入口地址，于是链接会失败。
在gcc编译的时候，附加要加 -lpthread参数即可解决
gcc -o tpipe tpipe.c -lpthread
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

void task1(int*);//线程1
void task2(int*);//线程2

int pipe1[2],pipe2[2];//存放两个无名管道标号
pthread_t thrd1,thrd2;

int main(int argc,char* arg[]){
    int ret;
    int num1,num2;

    //使用pipe()系统调用建立两个无名管道。
    if(pipe(pipe1)<0){//if fail
        perror("pipe1 not create\n");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipe2)<0){
        perror("pipe2 not create\n");
        exit(EXIT_FAILURE);
    }

    //create thread1
    num1=1;
    if(pthread_create(&thrd1,NULL,(void*)task1,&num1)){
        perror("thread1 not create\n");
        exit(EXIT_FAILURE);
    }
    //create thread2
    num2=2;
    if(pthread_create(&thrd2,NULL,(void*)task2,&num2)){
        perror("thread2 not create\n");
        exit(EXIT_FAILURE);
    }

    //switch to thread2
    pthread_join(thrd2,NULL);
    //switch to thread1
    pthread_join(thrd1,NULL);

    exit(EXIT_SUCCESS);
}

void task1(int* num){
    int x=1;
    do{
        printf("thread %d read %d\n",*num,x++);
        write(pipe1[1],&x,sizeof(int));//write x to side1 of pipe1
        read(pipe2[0],&x,sizeof(int));//read x from side0 of pipe2, make x plus 1
    }while(x<=9);

    //close the pipe
    close(pipe1[1]);
    close(pipe2[0]);
}

void task2(int* num){
    int x;
    do{
        read(pipe1[0],&x,sizeof(int));
        printf("thread 2 read %d\n",x++);
        write(pipe2[1],&x,sizeof(int));
    }while(x<=9);
    close(pipe1[0]);
    close(pipe2[1]);
}