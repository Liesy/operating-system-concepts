#include "train.h"
using namespace std;
#define BUFSZ 256
Sema::Sema(int id)
{
sem_id = id;
}
Sema::~Sema(){ }
/*
* 信号灯上的 down/up 操作
* semid:信号灯数组标识符
* semnum:信号灯数组下标
* buf:操作信号灯的结构
*/
int Sema::down()
{
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if((semop(sem_id,&buf,1)) <0) {
    perror("down error ");
    exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
int Sema::up()
{
    Sem_uns arg;
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if((semop(sem_id,&buf,1)) <0) {
        perror("up error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
/*
* 用于oneway管程的互斥执行
*/
Lock::Lock(Sema * s)
{
    sema = s;
}
Lock::~Lock(){ }
//上锁
void Lock::close_lock()
{
sema->down();
}
//开锁
void Lock::open_lock()
{
sema->up();
}
int OneWay::get_ipc_id(char *proc_file,key_t key){
    FILE *pf;
    int i,j;
    char line[BUFSZ],column[BUFSZ];
    if((pf=fopen(proc_file,"r"))==NULL){
        perror("proc file not open");
        exit(EXIT_FAILURE);
    }
    fgets(line,BUFSZ,pf);
    while(!feof(pf)){
        i=j=0;
        fgets(line,BUFSZ,pf);
        while(line[i]==' ')
            i++;
        while(line[i]!=' '){
            column[j++]=line[i++];
        }
        column[j]='\0';
        if(atoi(column)!=key)
            continue;
        j=0;
        while(line[i]==' ')
            i++;
        while(line[i]!=' '){
            column[j++]=line[i++];
        }
        column[j]='\0';
        i=atoi(column);
        fclose(pf);
        return i;
    }
    fclose(pf);
    return -1;
}
/*
*
set_sem 函数建立一个具有 n 个信号灯的信号量
*
如果建立成功,返回 一个信号量的标识符 sem_id
*
输入参数:
*
sem_key 信号量的键值
*
sem_val 信号量中信号灯的个数
*
sem_flag 信号量的存取权限
*/
int OneWay::set_sem(key_t sem_key,int sem_val,int sem_flg)
{
    int sem_id;
    Sem_uns sem_arg;
    //测试由 sem_key 标识的信号量是否已经建立
    if((sem_id=get_ipc_id("/proc/sysvipc/sem",sem_key)) < 0 ){
        //semget 新建一个信号灯,其标号返回到 sem_id
        if((sem_id = semget(sem_key,1,sem_flg)) < 0){
            perror("semaphore create error");
            exit(EXIT_FAILURE);
        }
    }
    //设置信号量的初值
    sem_arg.val = sem_val;
    if(semctl(sem_id,0,SETVAL,sem_arg) < 0){
        perror("semaphore set error");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}
/*
*
set_shm 函数建立一个具有 n 个字节 的共享内存区
*
如果建立成功,返回 一个指向该内存区首地址的指针 shm_buf
*
输入参数:
*
shm_key 共享内存的键值
*
shm_val 共享内存字节的长度
*
shm_flag 共享内存的存取权限
*/
char * OneWay::set_shm(key_t shm_key,int shm_num,int shm_flg)
{
    int i,shm_id;
    char *shm_buf;
    //测试由 shm_key 标识的共享内存区是否已经建立
    if((shm_id=get_ipc_id("/proc/sysvipc/shm",shm_key))<0){
        //shmget 新建 一个长度为 shm_num 字节的共享内存
        if((shm_id= shmget(shm_key,shm_num,shm_flg)) <0){
            perror("shareMemory set error");
            exit(EXIT_FAILURE);
        }
        //shmat 将由 shm_id 标识的共享内存附加给指针 shm_buf
        if((shm_buf=(char *)shmat(shm_id,0,0)) < (char *)0){
            perror("get shareMemory error");
            exit(EXIT_FAILURE);
        }
        for(i=0; i<shm_num; i++) shm_buf[i] = 0; //初始为 0
    }
    //共享内存区已经建立,将由 shm_id 标识的共享内存附加给指针 shm_buf
    if((shm_buf = (char *)shmat(shm_id,0,0)) < (char *)0) {
        perror("get shareMemory error");
        exit(EXIT_FAILURE);
    }
    return shm_buf;
}            
Condition::Condition(Sema *semax1,Sema *semax2){
    sema0=semax1;
    sema1=semax2;
}
/*
* check if it can pass
*/
void Condition::Wait(Lock *lock,int direc){
    if(direc == 0){
        cout<<getpid()<<"等待, 向东"<<endl;
        //lock->open_lock();
        sema0->down();
        //lock->close_lock();
    }
    else if(direc==1){
        cout<<getpid()<<"等待, 向西"<<endl;
       // lock->open_lock();
        sema1->down();
       // lock->close_lock();
    }
}
int Condition::Signal(int direc){
    int i;
    if(direc==0)//wake up a direction
    {
        i=sema0->up();
    }
    else if(direc==1){
        i=sema1->up();
    }
    return i;
}
Condition::~Condition(){}
OneWay::OneWay(int maxall,int maxcur){
    Sema *sema0;
    Sema *sema1;
    Sema *semaLock1,*semaLock2,*semaLock3;
    int ipc_flg=IPC_CREAT|0644;
    maxCars=(int *) set_shm(100,1,ipc_flg);
    numCars=(int *) set_shm(200,1,ipc_flg);
    currentDire=(int *) set_shm(300,1,ipc_flg);
    eastCount=(int *) set_shm(501,1,ipc_flg);
    westCount=(int *) set_shm(502,1,ipc_flg);
    sumPassedCars=(int *) set_shm(700,1,ipc_flg);
    eastWait=(int *) set_shm(801,1,ipc_flg);
    westWait=(int *) set_shm(802,1,ipc_flg);
    int sema0_id=set_sem(401,0,ipc_flg);
    int sema1_id=set_sem(402,0,ipc_flg);
    int semaLock1_id=set_sem(601,maxcur,ipc_flg);
    int semaLock2_id=set_sem(602,maxcur,ipc_flg);
    int semaLock3_id=set_sem(603,maxcur,ipc_flg);
    //init
    *maxCars=maxcur;
    *numCars=0;
    *currentDire=0;
    *eastCount=0;
    *westCount=0;
    *sumPassedCars=0;
    *eastWait=0;
    *westWait=0;
    sema0=new Sema(sema0_id);
    sema1=new Sema(sema1_id);
    semaLock1=new Sema(semaLock1_id);
    semaLock2=new Sema(semaLock2_id);
    semaLock3=new Sema(semaLock3_id);
    lock1=new Lock(semaLock1);
    lock2=new Lock(semaLock2);
    lock3=new Lock(semaLock3);
    condition=new Condition(sema0,sema1);
}
void OneWay::Arrive(int direc){
    lock1->close_lock();//FIFO,if lock1 is already taken by a different direction,then stop and wait
    if((*currentDire!=direc||*numCars>=*maxCars)&*sumPassedCars>0)
    {
        if(direc==0){
            *eastWait+=1;
        }
        else if(direc==1){
            *westWait+=1;
        }
        condition->Wait(lock1,direc);
        if(direc==0){
            *eastWait-=1;
        }
        else if(direc==1){
            *westWait-=1;
        }
    
    }// cout<<"ddddd"<<endl;
    *currentDire=direc;
    *numCars=*numCars+1;
    *sumPassedCars+=1; 
    lock1->open_lock();
    if(direc==0){
        //*eastWait-=1;//这个地方出bug了
        *eastCount=*eastCount+1;
        cout<<getpid()<<"进入车站, 向东"<<endl;
    }
    else if(direc==1){
        //*westWait-=1;
        *westCount=*westCount+1;
        cout<<getpid()<<"进入车站, 向西"<<endl;
    } 
    
}
void OneWay::Cross(int direc){
    //lock2->close_lock();
    if(direc==0){
        cout<<getpid()<<"通过 ,向东, 轨道上车辆数:"
        <<*numCars<<endl;
    }
    else if(direc==1){
        cout<<getpid()<<"通过 ,向西, 轨道上车辆数:"
        <<*numCars<<endl;
    }
    sleep(4);
    //lock2->open_lock();
}
void OneWay::Quit(int direc){
    lock3->close_lock();
    *numCars-=1;
    if(direc==0){
        cout<<getpid()<<"离开车站, 向东"<<endl;
    }else if(direc==1){
        cout<<getpid()<<"离开车站, 向西"<<endl;
    }
    //cout<<*numCars<<endl;
    if(*numCars==0){
        if(direc==0){
            if(*westWait>0){
                condition->Signal(1);
            }
            else if(*eastWait>0){
                condition->Signal(0);
            }
        }
        else if(direc==1){
            if(*eastWait>0){
                condition->Signal(0);
            }
            else if(*westWait>0){
                condition->Signal(1);
            }
        }
    }
    lock3->open_lock();
}
OneWay::~OneWay(){
    delete condition;
}
int main(int argc,char **argv){
    int maxCars;
    int maxSingleDirect;
    cout<<"请输入总车辆数:";
    cin>>maxCars;
    cout<<"请输入单方向通过的最大车数:";
    cin>>maxSingleDirect;
    OneWay *oneway=new OneWay(maxCars,maxSingleDirect);
    //build tube, check if enterable,decide the direction to ride into oneway;
    int i;
    int pid[maxCars];
    srand(time(NULL));
    for(i=0;i<maxCars;i++){
        pid[i]=fork();
        //when a new process is running,the random series will reset,
        //so we should decide the direction outside the declaration if();
        int direct;
        direct=rand()%2;//fair decision of direction
        if(pid[i]==0){
            sleep(1);
            //direct=*oneway->sumPassedCars%2;
            oneway->Arrive(direct);
            oneway->Cross(direct);
            oneway->Quit(direct);
            exit(EXIT_SUCCESS);
        }
    }
    for(int i=0;i<maxCars;i++){
        waitpid(pid[i],NULL,0);
    }
    cout<<*(oneway->eastCount)<<" 辆列车向东,"<<*(oneway->westCount)
    <<" 辆列车向西,正常通行."<<endl;
    delete oneway;
    return EXIT_SUCCESS;
}