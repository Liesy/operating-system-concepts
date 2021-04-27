/*
https://blog.csdn.net/weixin_43943977/article/details/101830428
https://blog.csdn.net/Godsolve/article/details/93737731

1 三个抽烟者相当于三个不同的消费者，他们每次只会有一个抽烟者可以抽到烟，其余两个则需要等待
2 供应者每次放下两种材料后都会停下来等待直到有消费者使用了这两种材料，他才会继续放另两种材料, 相当于说缓冲区的大小为1
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>

#define BUFSZ 256

int get_ipc_id(char *proc_file,key_t key);
char *set_shm(key_t shm_key,int shm_num,int shm_flag);
int set_msq(key_t msq_key,int msq_flag);
int set_sem(key_t sem_key,int sem_val,int sem_flag);
int down(int sem_id);
int up(int sem_id);

//信号灯控制用的共同体
typedef union semuns {
    int val;
} Sem_uns;

//消息结构体
typedef struct msgbuf {
    long mtype;
    char mtext[1];
} Msg_buf;

//生产消费者共享缓冲区and有关的变量
key_t buff_key;
int buff_num;
char* buff_ptr;

//生产者放产品位置的共享指针
key_t pput_key;
int pput_num;
int* pput_ptr;

//消费者取产品位置的共享指针
key_t cget_key;
int cget_num;
int *cget_ptr;

//生产者有关的信号量
key_t tobacco_key;
key_t glue_key;
key_t paper_key;
int tobacco_sem;
int glue_sem;
int paper_sem;

//消费者有关的信号量
key_t empty_key;
key_t mutex_key;
int empty_sem;
int mutex_sem;

int sem_val;
int sem_flg;
int shm_flg;