#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
/*信号灯控制用的共同体*/
typedef union semuns{
    int val;
} Sem_uns;

//管程中使用的信号量
class Sema{
public:
    Sema(int id);
    ~Sema();
    int down(); //信号量加 1
    int up();   //信号量减 1
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

class Condition{
public:
    Condition(Sema *sema1, Sema *sema2);
    ~Condition();
    void Wait(Lock *conditionLock, int direct); //过路条件不足时阻塞
    int Signal(int direc);
    //唤醒相反方向阻塞车辆
private:
    Sema *sema0; // 一个方向阻塞队列
    Sema *sema1; // 另一方向阻塞队列
    Lock *lock;  // 进入管程时获取的锁
};

class OneWay{
public:
    OneWay(int maxall, int maxcur);
    ~OneWay();
    void Arrive(int direc);
    // 车辆准备上单行道,direc 为行车方向
    void Cross(int direc);
    // 车辆正在单行道上
    void Quit(int direc);
    // 车辆通过了单行道
    int *eastCount;
    int *westCount;
    int *eastWait;
    int *westWait;
    int *sumPassedCars; //已经通过的车辆总数
private:
    //建立或获取 ipc 信号量的一组函数的原型说明
    int get_ipc_id(char *proc_file, key_t key);
    int set_sem(key_t sem_key, int sem_val, int sem_flag);
    //创建共享内存
    char *set_shm(key_t shm_key, int shm_num, int shm_flag);
    int rate;             //车速
    int *maxCars;         //最大同向车数
    int *numCars;         //当前正在通过的车辆数
    int *currentDire;     //当前通过的车辆的方向
    Condition *condition; //通过单行道的条件变量
    Lock *lock;           //单行道管程锁
};