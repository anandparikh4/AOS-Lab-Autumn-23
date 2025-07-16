// Advanced test file to verify process management , memory management , concurrency , mutual exclusion

/*
The parent process forks 5 children, and each of them forks 5 threads
All 5 processes compete at the process-level for accessing global structures of the LKM
All 5 threads of a process compete at the user-thread level for accessing process-specific data structures
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <wait.h>
#include <pthread.h>

// stringizing the procfile name
#define xstr(X) #X
#define str(X) xstr(X)
#define PROC_FILE partb_1_20CS10007_20CS30016

int fd;
pthread_mutex_t mutex;

void * exec_thread(void * args){

    int num = *((int *)args);
    
    int pid = getpid();
    long long int tid = pthread_self();
    printf("Process %d Thread[%d] %lld starting\n" , pid , num , tid);

    int value;
    for(int i=0;i<10;i++){      // write 10 values
        value = i + 10*num;
        write(fd , &value , 4);
        printf("Process %d Thread[%d] %lld wrote value %d\n" , pid , num , tid , value);
    }
    for(int i=0;i<10;i++){      // read 10 values
        read(fd , &value , 4);
        printf("Process %d Thread[%d] %lld read value %d\n" , pid , num , tid , value);
    }
    
    pthread_exit(0);
}

void exec_process(){
    printf("Process %d started, will now create 5 threads\n" , getpid());

    fd = open("/proc/"str(PROC_FILE) , O_RDWR);     // open
    int capacity = 50;
    write(fd , &capacity , 1);

    pthread_t threads[5];
    int nums[5] = {0,1,2,3,4};

    for(int i=0;i<5;i++){
        pthread_create(&threads[i] , NULL , exec_thread , (void *)(&nums[i]));
    }
    for(int i=0;i<5;i++){
        pthread_join(threads[i] , NULL);
    }

    close(fd);

    return;
}

int main(){
    
    for(int i=0;i<5;i++){
        int pid = fork();
        if(pid == 0){
            exec_process();
            return 0;
        }
    }
    for(int i=0;i<5;i++) wait(NULL);

    printf("SUCCESS!\n");
    return 0;
}
