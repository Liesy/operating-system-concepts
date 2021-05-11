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
#include <time.h>
using namespace std;

/*信号灯控制用的共同体*/
typedef union semuns{
    int val;
} Sem_uns;

enum State{
    waitt,
    run
};

//火车站管程中使用的信号量
class Sema{
public:
    Sema(int id);
    ~Sema();
    int down(); //信号量加 1
    int up();   //信号量减 1
private:
    int sem_id; //信号量标识符
};

//火车站管程中使用的锁
class Lock{
public:
    Lock(Sema *lock);
    ~Lock();
    void close_lock();
    void open_lock();
private:
    Sema *sema; //锁使用的信号量
};

//火车站管程中使用的条件变量
class Condition{
public:
    Condition(char *st[], Sema *sm);
    ~Condition();
    void Wait(Lock *lock, int i); //条件变量阻塞操作
    void Signal(int i);           //条件变量唤醒操作
private:
    Sema *sema;
    char **state;
};

//火车站管程的定义
class dp{
public:
    int *maxcars; //最大火车数
    int *nowcars; //当前已经通过的火车数

    int *sumeast; //当前已经通过的由东向西的火车数
    int *sumwest; //当前已经通过的由西向东的火车数

    dp(int rate, int maxcur); //管程构造函数
    ~dp();
    void start(int i); //发车
    void quit(int i);  //火车通过后离开

    //建立或获取 ipc 信号量的一组函数的原型说明
    int get_ipc_id(char *proc_file, key_t key);
    int set_sem(key_t sem_key, int sem_val, int sem_flag);
    char *set_shm(key_t shm_key, int shm_num, int shm_flag);
private:
    int rate; //车速
    Lock *lock;
    char *state[2];     //两个火车站的状态
    int cnt[2];         //火车站同时发送火车的数量
    Condition *self[2]; //火车站条件变量
};
