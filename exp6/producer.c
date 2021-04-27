#include"ipc.h"

int main(int argc,char* argv[]){
    int rate;
    //可在在命令行第一参数指定一个进程睡眠秒数,以调解进程执行速度
    if(argv[1]!=NULL)
        rate = atoi(argv[1]);
    else rate=3; //不指定为3秒
    //共享内存使用的变量
    buff_key=101;//缓冲区任给的键值
    buff_num=1;//缓冲区任给的长度
    pput_key=102;//生产者放产品指针的键值
    pput_num=1;//指针数
    shm_flg=IPC_CREAT|0644;//共享内存读写权限
    //获取缓冲区使用的共享内存,buff_ptr 指向缓冲区首地址
    buff_ptr=(char*)set_shm(buff_key,buff_num,shm_flg);
    //获取生产者放产品位置指针 pput_ptr
    pput_ptr=(int*)set_shm(pput_key,pput_num,shm_flg);
    //信号量使用的变量
    tobacco_key=201;//生产者1的同步信号灯键值
    glue_key=202; //生产者2的同步信号灯键值
    paper_key=203; //生产者3的同步信号灯键值
    empty_key=301; //消费者同步信号灯键值
    mutex_key=302; //消费者互斥信号灯键值
    sem_flg=IPC_CREAT|0644;
    //生产者同步信号灯初值设为缓冲区最大可用量
    sem_val = buff_num;
    //获取生产者同步信号灯
    empty_sem = set_sem(empty_key,sem_val,sem_flg);
    //消费者初始无产品可取,同步信号灯初值设为 0
    sem_val = 0;
    //获取消费者同步信号灯
    paper_sem = set_sem(paper_key,sem_val,sem_flg);
    glue_sem = set_sem(glue_key,sem_val,sem_flg);
    tobacco_sem = set_sem(tobacco_key,sem_val,sem_flg);
    //生产者互斥信号灯初值为 1
    sem_val = 1;
    //获取生产者互斥信号灯,引用标识存 pmtx_sem
    mutex_sem = set_sem(mutex_key,sem_val,sem_flg);
    int i=0;
    //循环执行模拟生产者不断放产品
    while(1){
        int d;
        //如果缓冲区满则生产者阻塞
        down(empty_sem);
        //如果另一生产者正在放产品,本生产者阻塞
        down(mutex_sem);
        //用写一字符的形式模拟生产者放产品,报告本进程号和放入的字符及存放的位置
        sleep(rate);
        d=(i++)%3;
        printf("%d provider put: %d to Buffer[%d]\n",getpid(),d,*pput_ptr);
        //存放位置循环下移
        *pput_ptr = (*pput_ptr+1)%buff_num;
        //唤醒阻塞的生产者
        up(mutex_sem);
        //唤醒阻塞的消费者
        if(d==0)
            up(paper_sem);
        if(d==1)
            up(glue_sem);
        if(d==2)
            up(tobacco_sem);
    }
    return EXIT_SUCCESS;
}