#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/msg.h>
#include<sys/wait.h>
/*信号灯控制用的共同体*/
typedef union semuns {
int val;
} Sem_uns;
//管程中使用的信号量
class Sema{
public:
    Sema(int id);
    ~Sema();
    int down(); //信号量加 1
    int up(); //信号量减 1
private:
    int sem_id; //信号量标识符
};
//管程中使用的锁
class Lock{
public:
    Lock(Sema *lock);
    ~Lock();
    void close_lock();
    void open_lock();
private:
    Sema *sema; //锁使用的信号量
};
//管程中使用的条件变量
class Condition{
public:
    Condition(Sema *sema1, Sema *sema2);
    ~Condition();
    void Wait(Lock *conditionLock,int direct); //条件变量阻塞操作
    int Signal(int direc); //条件变量唤醒操作
private:
    Sema *sema0; //a block queue of one way
    Sema *sema1; //the other
    Lock *lock; //the lock acquired when entering the tube;
};
class OneWay{
public:
    OneWay (int maxall,int maxcur);
        ~OneWay();
    void Arrive(int direc);//car is about to ride on oneway
    void Cross(int direc);//on the oneway
    void Quit(int direc);//pass the oneway
    int *eastCount;//tot
    int *westCount;//tot
    int *eastWait;
    int *westWait;
    int *sumPassedCars;//total amount of already passed cars;
private:
    //build functions to get ipc sema;
    int get_ipc_id(char *proc_file,key_t key);
    int set_sem(key_t sem_key,int sem_val,int sem_flag);
    //create sharing memory
    char *set_shm(key_t shm_key,int shm_num,int shm_flag);
    int rate;//velocity of the car;
    int *maxCars;//max amount of cars in the same direction
    int *numCars;//amount of cars which is passing;
    int *currentDire;// current crossing car's direction
    Condition *condition;// condition variable to cross the oneway
    Lock *lock1,*lock2,*lock3;
};