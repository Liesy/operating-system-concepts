#include "ipc.h"

int main(int argc, char *argv[]){
    int rate;
    if(argv[1] != NULL)
        rate = atoi(argv[1]);
    else 
        rate = 3;

    Msg_buf msg_arg;
    struct msqid_ds msg_sofa_info;
    struct msqid_ds msg_wait_info;

    //建立沙发消息队列
    q_flg = IPC_CREAT | 0644;
    q_sofa_key = 300;
    q_sofa_id = set_msq(q_sofa_key, q_flg);
    //建立等候室消息队列
    q_wait_key = 400;
    q_wait_id = set_msq(q_wait_key, q_flg);
    sem_flg = IPC_CREAT | 0644;

    //建立一个互斥帐本信号量
    s_account_key = 100;
    s_account_val = 1;
    s_account_sem = set_sem(s_account_key, s_account_val, sem_flg);
    //建立一个同步顾客信号量
    s_customer_key = 200;
    s_customer_val = 0;
    s_customer_sem = set_sem(s_customer_key, s_customer_val, sem_flg);

    int customerNumber = 1;
    while (1){
        msgctl(q_sofa_id, IPC_STAT, &msg_sofa_info);
        //沙发没座满
        if (msg_sofa_info.msg_qnum < 4){
            quest_flg = IPC_NOWAIT; //以非阻塞方式接收消息
            if (msgrcv(q_wait_id, &msg_arg, sizeof(msg_arg), WAIT, quest_flg) >= 0){
                msg_arg.mtype = SOFA;
                printf("%d号新顾客坐入沙发\n", msg_arg.mid);
                msgsnd(q_sofa_id, &msg_arg, sizeof(msg_arg), IPC_NOWAIT);
            }
            else{
                msg_arg.mtype = SOFA;
                msg_arg.mid = customerNumber;
                customerNumber++;
                printf("%d号新顾客坐入沙发\n", msg_arg.mid);
                msgsnd(q_sofa_id, &msg_arg, sizeof(msg_arg), IPC_NOWAIT);
            }
        }
        else{
            msgctl(q_wait_id, IPC_STAT, &msg_wait_info);
            if (msg_wait_info.msg_qnum < 13){
                msg_arg.mtype = WAIT;
                msg_arg.mid = customerNumber;
                printf("沙发座满,%d号新顾客进入等候室\n", customerNumber);
                customerNumber++;
                msgsnd(q_wait_id, &msg_arg, sizeof(msg_arg), IPC_NOWAIT);
            }
            else{
                printf("等候室满,%d号新顾客没有进入理发店\n", customerNumber);
                down(s_customer_sem);
            }
        }
        sleep(rate);
    }
    return EXIT_SUCCESS;
}