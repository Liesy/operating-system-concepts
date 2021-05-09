#include "ipc.h"

int main(int argc, char *argv[]){
    int rate;
    if(argv[1] != NULL)
        rate = atoi(argv[1]);
    else 
        rate = 3;
    
    struct msqid_ds msg_sofa_info;
    Msg_buf msg_arg;
    sem_flg = IPC_CREAT | 0644;

    //建立一个互斥帐本信号量
    s_account_key = 100;
    s_account_val = 1;
    s_account_sem = set_sem(s_account_key, s_account_val, sem_flg);
    //建立一个同步顾客信号量
    s_customer_key = 200;
    s_customer_val = 0;
    s_customer_sem = set_sem(s_customer_key, s_customer_val, sem_flg);

    //建立沙发消息队列
    q_flg = IPC_CREAT | 0644;
    q_sofa_key = 300;
    q_sofa_id = set_msq(q_sofa_key, q_flg);
    //建立等候室消息队列
    q_wait_key = 400;
    q_wait_id = set_msq(q_wait_key, q_flg);

    //建立 3 个理发师进程;
    int pid[3];
    int i;
    for (i = 0; i < 3; i++){
        pid[i] = fork();
        if (pid[i] == 0){
            while (1){
                msgctl(q_sofa_id, IPC_STAT, &msg_sofa_info);
                if (msg_sofa_info.msg_qnum == 0)
                    printf("%d 号理发师睡眠\n", getpid());
                
                //以阻塞方式从沙发队列接收一条消息
                msgrcv(q_sofa_id, &msg_arg, sizeof(msg_arg), SOFA, 0);
                //唤醒顾客进程(让下一顾客坐入沙发)
                up(s_customer_sem);

                printf("%d 号理发师给 %d 号顾客理发\n", getpid(), msg_arg.mid);
                sleep(rate);
                down(s_account_sem);
                printf("%d 号理发师向 %d 号顾客收费\n", getpid(), msg_arg.mid);
                up(s_account_sem);
            }
        }
    }
    return EXIT_SUCCESS;
}