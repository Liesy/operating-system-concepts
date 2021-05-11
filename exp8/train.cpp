#include "train.h"
using namespace std;

Sema::Sema(int id){sem_id = id;}
Sema::~Sema(){}

/*
* 信号灯上的 down/up 操作
* semid:信号灯数组标识符
* semnum:信号灯数组下标
* buf:操作信号灯的结构
*/
int Sema::down(){
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0){
        perror("down error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int Sema::up(){
    Sem_uns arg;
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0){
        perror("up error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

/*
* 用于单行道管程的互斥执行
*/
Lock::Lock(Sema *s){sema = s;}
Lock::~Lock(){}

//上锁
void Lock::close_lock(){sema->down();}
//开锁
void Lock::open_lock(){sema->up();}

int OneWay::get_ipc_id(char *proc_file, key_t key){
    #define BUFSZ 256
    FILE *pf;
    int i, j;
    char line[BUFSZ], colum[BUFSZ];
    if ((pf = fopen(proc_file, "r")) == NULL){
        perror("Proc file not open");
        exit(EXIT_FAILURE);
    }
    fgets(line, BUFSZ, pf);
    while (!feof(pf)){
        i = j = 0;
        fgets(line, BUFSZ, pf);
        while (line[i] == ' ')
            i++;
        while (line[i] != ' ')
            colum[j++] = line[i++];
        colum[j] = '\0';
        if (atoi(colum) != key)
            continue;
        j = 0;
        while (line[i] == ' ')
            i++;
        while (line[i] != ' ')
            colum[j++] = line[i++];
        colum[j] = '\0';
        i = atoi(colum);
        fclose(pf);
        return i;
    }
    fclose(pf);
    return -1;
}

/*
*set_shm 函数建立一个具有 n 个字节 的共享内存区
*如果建立成功,返回 一个指向该内存区首地址的指针 shm_buf
*输入参数:
*shm_key 共享内存的键值
*shm_val 共享内存字节的长度
*shm_flag 共享内存的存取权限
*/
char *OneWay::set_shm(key_t shm_key, int shm_num, int shm_flg){
    int i, shm_id;
    char *shm_buf;
    //测试由 shm_key 标识的共享内存区是否已经建立
    if ((shm_id = get_ipc_id("/proc/sysvipc/shm", shm_key)) < 0){
        //shmget 新建 一个长度为 shm_num 字节的共享内存
        if ((shm_id = shmget(shm_key, shm_num, shm_flg)) < 0){
            perror("shareMemory set error");
            exit(EXIT_FAILURE);
        }
        //shmat 将由 shm_id 标识的共享内存附加给指针 shm_buf
        if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0){
            perror("get shareMemory error");
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < shm_num; i++)
            shm_buf[i] = 0; //初始为 0
    }
    //共享内存区已经建立,将由 shm_id 标识的共享内存附加给指针 shm_buf
    if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0){
        perror("get shareMemory error");
        exit(EXIT_FAILURE);
    }
    return shm_buf;
}

/*
*set_sem 函数建立一个具有 n 个信号灯的信号量
*如果建立成功,返回 一个信号量的标识符 sem_id
*输入参数:
*sem_key 信号量的键值
*sem_val 信号量中信号灯的个数
*sem_flag 信号量的存取权限
*/
int OneWay::set_sem(key_t sem_key, int sem_val, int sem_flg){
    int sem_id;
    Sem_uns sem_arg;
    //测试由 sem_key 标识的信号量是否已经建立
    if ((sem_id = get_ipc_id("/proc/sysvipc/sem", sem_key)) < 0){
        //semget 新建一个信号灯,其标号返回到 sem_id
        if ((sem_id = semget(sem_key, 1, sem_flg)) < 0){
            perror("semaphore create error");
            exit(EXIT_FAILURE);
        }
    }
    //设置信号量的初值
    sem_arg.val = sem_val;
    if (semctl(sem_id, 0, SETVAL, sem_arg) < 0){
        perror("semaphore set error");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}

Condition::Condition(Sema *semax1, Sema *semax2){
    sema0 = semax1;
    sema1 = semax2;
}

/*
* 看看是否能通过
*/
void Condition::Wait(Lock *lock, int direc)
{
    if (direc == 0){
        cout << getpid() << "东边等待"<< "\n";
        lock->open_lock();
        sema0->down();
        lock->close_lock();
    }
    else if (direc == 1){
        cout << getpid() << "西边等待"<< "\n";
        lock->open_lock();
        sema1->down();
        lock->close_lock();
    }
}

int Condition::Signal(int direc){
    int i;
    if (direc == 0)
        i = sema0->up();
    else if (direc == 1)
        i = sema1->up();
    return i;
}

Condition::~Condition(){};

OneWay::OneWay(int maxall, int maxcur){
    Sema *sema0;
    Sema *sema1;
    Sema *semaLock;
    int ipc_flg = IPC_CREAT | 0644;
    maxCars = (int *)set_shm(100, 1, ipc_flg);
    numCars = (int *)set_shm(200, 1, ipc_flg);
    currentDire = (int *)set_shm(300, 1, ipc_flg);
    eastCount = (int *)set_shm(501, 1, ipc_flg);
    westCount = (int *)set_shm(502, 1, ipc_flg);
    sumPassedCars = (int *)set_shm(700, 1, ipc_flg);
    eastWait = (int *)set_shm(801, 1, ipc_flg);
    westWait = (int *)set_shm(802, 1, ipc_flg);
    int sema0_id = set_sem(401, 0, ipc_flg);
    int sema1_id = set_sem(402, 0, ipc_flg);
    int semaLock_id = set_sem(601, maxcur, ipc_flg);
    *maxCars = maxcur;
    *numCars = 0;
    *currentDire = 0;
    *eastCount = 0;
    *westCount = 0;
    *sumPassedCars = 0;
    *eastWait = 0;
    *westWait = 0;
    sema0 = new Sema(sema0_id);
    sema1 = new Sema(sema1_id);
    semaLock = new Sema(semaLock_id);
    lock = new Lock(semaLock);
    condition = new Condition(sema0, sema1);
}

void OneWay::Arrive(int direc){
    lock->close_lock();
    if ((*currentDire != direc || *numCars >= *maxCars) && *sumPassedCars > 0){
        if (direc == 0)
            *eastWait += 1;
        else if (direc == 1)
            *westWait += 1;
        condition->Wait(lock, direc);
    }
    if (direc == 0){//东 +1
        *eastWait -= 1;
        *eastCount = *eastCount + 1;
        cout << getpid() << "东边进入车站\n";
    }
    else if (direc == 1) {//西 +1
        *westCount = *westCount + 1;
        *westWait -= 1;
        cout << getpid() << "西边进入车站\n";
    }
    *numCars = *numCars + 1;
    *currentDire = direc;
    *sumPassedCars += 1;
    lock->open_lock();
}

void OneWay::Cross(int direc){
    lock->close_lock();
    sleep(3);
    if (direc == 0)
        cout << getpid() << "通过" << "\n";
    else if (direc == 1)
        cout << getpid() << "通过" << "\n";
    lock->open_lock();
}

void OneWay::Quit(int direc){
    lock->close_lock();
    *numCars -= 1;
    if (direc == 0)
        cout << getpid() << "离开车站"<< "\n";
    else if (direc == 1)
        cout << getpid() << "离开车站"<< "\n";
    if (*numCars == 0){
        if (direc == 0){
            if (*westWait > 0)
                condition->Signal(1);
            else if (*eastWait > 0)
                condition->Signal(0);
        }
        else if (direc == 1){
            if (*eastWait > 0)
                condition->Signal(0);
            else if (*westWait > 0)
                condition->Signal(1);
        }
    }
    lock->open_lock();
}

OneWay::~OneWay(){delete condition;}


int main(int argc, char **argv){
    int maxCars;
    int maxSingelDirect;
    cout << "请输入总车辆数:";
    cin >> maxCars;
    cout << "请输入单方向通过的最大车数:";
    cin >> maxSingelDirect;
    OneWay *oneWay = new OneWay(maxCars, maxSingelDirect);
    //建立管程,判断可不可进、决定方向,进入单行道
    int i;
    int pid[maxCars];
    for (i = 0; i < maxCars; i++){ //对每一辆车都创建一个子进程
        pid[i] = fork();
        if (pid[i] == 0){
            sleep(1);
            srand(time(NULL));
            int direct = rand() % 2;
            direct = *oneWay->sumPassedCars % 2;
            oneWay->Arrive(direct);
            oneWay->Cross(direct);
            oneWay->Quit(direct);
            exit(EXIT_SUCCESS);
        }
    }
    for (i = 0; i < maxCars; i++)
        waitpid(pid[i], NULL, 0);
    cout << *(oneWay->eastCount) << "辆列车向东" << *(oneWay->westCount)
         << "辆列车向西,正常通行.\n";
    delete oneWay;
    return EXIT_SUCCESS;
}
