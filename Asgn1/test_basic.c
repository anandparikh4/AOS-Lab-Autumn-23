// Basic test file for error checking and flow of execution

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// stringizing the procfile name
#define xstr(X) #X
#define str(X) xstr(X)
#define PROC_FILE partb_1_20CS10007_20CS30016

void exit_with_error(){
    printf("ERROR\n");
    exit(-1);
}

int main(){

    int fd , bytes_read , bytes_written , value , capacity;

    // open
    fd = open("/proc/"str(PROC_FILE) , O_RDWR);
    if(fd < 0) exit_with_error();
    printf("Opened file /proc/"str(PROC_FILE)"\n");

    // attempt read before write
    bytes_read = read(fd , &value , 4);
    if(bytes_read >= 0) exit_with_error();
    
    // attempt to initialize deque with size > 100
    capacity = 200;
    bytes_written = write(fd , &capacity , 4);
    if(bytes_written >= 0) exit_with_error();

    // initialize deque with capacity 5
    capacity = 5;
    bytes_written = write(fd , &capacity , 4);
    if(bytes_written < 0) exit_with_error();
    printf("Initlaized deque with capacity %d\n" , capacity);

    // make 5 writes
    for(int i=0;i<5;i++){
        bytes_written = write(fd , &i , 4);
        if(bytes_written < 0) exit_with_error();
        printf("Wrote value %d\n" , i);
    }

    // attempt extra write to full deque
    value = 47;
    bytes_written = write(fd , &value , 4);
    if(bytes_written >= 0) exit_with_error();

    // re-open
    fd = open("/proc/"str(PROC_FILE) , O_RDWR);
    if(fd < 0) exit_with_error();
    printf("Re-opened file /proc/"str(PROC_FILE)"\n");

    // initialize deque with capacity 5
    capacity = 5;
    bytes_written = write(fd , &capacity , 4);
    if(bytes_written < 0) exit_with_error();
    printf("Initlaized deque with capacity %d\n" , capacity);

    // make 5 writes
    for(int i=0;i<5;i++){
        bytes_written = write(fd , &i , 4);
        if(bytes_written < 0) exit_with_error();
        printf("Wrote value %d\n" , i);
    }

    // make 5 reads
    for(int i=0;i<5;i++){
        bytes_read = read(fd , &value , 4);
        if(bytes_read < 0) exit_with_error();
        printf("Read value %d\n" , value);
    }

    // attempt read from empty deque
    bytes_read = read(fd , &value , 4);
    if(bytes_read >= 0) exit_with_error();

    // close fd
    close(fd);
    printf("Closed /proc/"str(PROC_FILE)"\n");
    
    printf("SUCCESS!\n");

    return 0;
}
