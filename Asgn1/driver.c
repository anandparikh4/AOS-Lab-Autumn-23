/*
    AOS Autumn 2023
    LKM Deque
    Anand Parikh - 20CS10007
    Divyansh Vijayvergia - 20CS30016
*/

// Main driver file

#include "defs.h"

// stringizing the procfile name
#define xstr(X) #X
#define str(X) xstr(X)
#define PROC_FILE partb_1_20CS10007_20CS30016

// init, exit and file operations function prototypes
static int open_mylkm(struct inode * , struct file *);
static int close_mylkm(struct inode * , struct file *);
static ssize_t read_mylkm(struct file * , char __user * , size_t , loff_t *);
static ssize_t write_mylkm(struct file * , const char __user * , size_t , loff_t *);
static int __init init_mylkm(void);
static void __exit exit_mylkm(void);

// modinfo
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Anand Parikh , Divyansh Vijayvergia");
MODULE_DESCRIPTION("kernel level deque implementation");

// register entry and exit functions
module_init(init_mylkm);
module_exit(exit_mylkm);

// globals
static struct proc_dir_entry * proc_file;       // proc_file
static struct mutex global_lock;                // lock for process list
static struct list * process_list;              // list of all processes that have the proc_file open simultaneously
struct proc_ops fops = {
    .proc_open = open_mylkm,
    .proc_release = close_mylkm,
    .proc_read = read_mylkm,
    .proc_write = write_mylkm,
};

// init implementation
static int __init init_mylkm(void){
    proc_file = proc_create(str(PROC_FILE) , 0666 , NULL , &fops);          // create proc file
    if(!proc_file){
        printk(KERN_ALERT "MYLKM: procfile creation failed\n");
        return -ENOMEM;
    }
    process_list = init_list();                                             // initialize empty process list
    if(!process_list){
        proc_remove(proc_file);
        printk(KERN_ALERT "MYLKM: initializing process list failed\n");
        return -ENOMEM;
    }
    mutex_init(&global_lock);                                            // initialize global lock
    printk(KERN_INFO "MYLKM: created /proc/"str(PROC_FILE)"\n");
    return 0;
}

// exit implementation
static void __exit exit_mylkm(void){
    mutex_destroy(&global_lock);        // destroy global lock
    destroy_list(process_list);         // destroy process list
    proc_remove(proc_file);             // remove proc file
    printk(KERN_INFO "MYLKM: deleted /proc/"str(PROC_FILE)"\n");
    return;
}

// open callback handler
static int open_mylkm(struct inode * inode , struct file * file){
    int ret = 0;
    mutex_lock(&global_lock);           // lock the process_list to iterate over it and find the process

    pid_t pid = current->tgid;
    struct process * process_p = find_process(process_list , pid);
    if(!process_p){                                 // if process not found, then the proc_file opened (not re-opened while already open)
        ret = add_process(process_list , pid);      // add process to the list
        if(ret < 0){
            printk(KERN_ALERT "MYLKM: error in registering process %d\n" , pid);
            mutex_unlock(&global_lock);
            return ret;
        }
        printk(KERN_INFO "MYLKM: process %d opened procfile\n" , pid);
    }
    else{                                           // if process found, process re-opens proc file while already open
        mutex_lock(&(process_p->lock));             // acquire process lock also, since another thread might be working on it

        ret = delete_process(process_list , pid);   // delete the process from the list
        if(ret < 0){
            printk(KERN_ALERT "MYLKM: error in re-registering process %d\n" , pid);
            mutex_unlock(&(process_p->lock));
            mutex_unlock(&global_lock);
            return ret;
        }
        ret = add_process(process_list , pid);      // and add it back to the list, to re-initialize all fields
        if(ret < 0){
            printk(KERN_ALERT "MYLKM: error in re-registering process %d\n" , pid);
            // mutex_unlock(&(process_p->lock));    // do NOT unlock mutex here, since it is already unlocked and destroyed
            mutex_unlock(&global_lock);
            return ret;
        }
        printk(KERN_INFO "MYLKM: process %d re-opened procfile\n" , pid);

        // mutex_unlock(&(process_p->lock));    // do NOT unlock mutex here, since it is already unlocked and destroyed
    }

    mutex_unlock(&global_lock);
    return ret;
}

// close callback handler
static int close_mylkm(struct inode * inode , struct file * file){
    int ret = 0;
    mutex_lock(&global_lock);       // lock process list

    pid_t pid = current->tgid;
    struct process * process_p = find_process(process_list , pid);      // find process
    if(!process_p){
        mutex_unlock(&global_lock);
        return ret;
    }
    else{
        mutex_lock(&(process_p->lock));                 // if found, lock process (since other threads might be accessing the deque)

        ret = delete_process(process_list , pid);       // and delete from list
        if(ret < 0){
            printk(KERN_ALERT "MYLKM: error in de-registering process %d\n" , pid);
            mutex_unlock(&(process_p->lock));
            mutex_unlock(&global_lock);
            return ret;
        }
        printk(KERN_INFO "MYLKM: process %d closed procfile\n" , pid);

        // mutex_unlock(&(process_p->lock));    // do NOT unlock mutex here, since it is already unlocked and destroyed
    }

    mutex_unlock(&global_lock);
    return ret;
}

// read callback handler
static ssize_t read_mylkm(struct file * file , char __user * to_buf , size_t to_len , loff_t * offset){
    int ret = 0;
    mutex_lock(&global_lock);       // lock process list

    pid_t pid = current->tgid;
    struct process * process_p = find_process(process_list , pid);      // find process
    if(!process_p){
        printk(KERN_ALERT "MYLKM: unregistered process %d attempted read\n" , pid);
        mutex_unlock(&global_lock);
        return -EACCES;
    }
    
    mutex_lock(&(process_p->lock));     // if found, lock process
    mutex_unlock(&global_lock);         // and then unlock process list [explanation at the end]

    if(process_p->state == NASCENT){    // if in NASCENT state, do not allow read
        printk(KERN_ALERT "MYLKM: process %d attempted read from uninitialized deque\n" , pid);
        mutex_unlock(&(process_p->lock));
        return -EACCES;
    }
    int value;
    ret = pop_front(process_p->dq , &value);        // else pop from the front
    if(ret < 0){
        printk(KERN_ALERT "MYLKM: process %d attempted read from empty deque\n" , pid);
        mutex_unlock(&(process_p->lock));
        return ret;
    }
    int to_copy = min((int)to_len , 4);
    int not_copied = copy_to_user(to_buf , (const char *)&value , to_copy);     // and copy to user space buffer
    if(not_copied){
        printk(KERN_ALERT "MYLKM: could not copy %d out of %d bytes to process %d from kernel\n" , not_copied , to_copy , pid);
        mutex_unlock(&(process_p->lock));
        return -EACCES;
    }
    ret = to_copy - not_copied;     // will always be = to_copy in our case, but written like so to preserve semantic meaningfulness
    printk(KERN_INFO "MYLKM: sucessfully copied %d bytes to process %d from kernel (popped %d from left end of deque)\n" , to_copy , pid , value);

    mutex_unlock(&(process_p->lock));   // finally unlock process
    return ret;
}

// write callback handler
static ssize_t write_mylkm(struct file * file , const char __user * from_buf , size_t from_len , loff_t * offset){
    int ret = 0;
    mutex_lock(&global_lock);       // lock process list

    pid_t pid = current->tgid;
    struct process * process_p = find_process(process_list , pid);      // find process
    if(!process_p){
        printk(KERN_ALERT "MYLKM: unrecognized process %d attempted write\n" , pid);
        mutex_unlock(&global_lock);
        return -EACCES;
    }

    mutex_lock(&(process_p->lock));     // if found, lock process
    mutex_unlock(&global_lock);         // and then unlock process list [explanation at the end]

    if(process_p->state == NASCENT){    // if in NASCENT state
        int capacity = 0;
        int to_copy = min((int)from_len , 1);
        int not_copied = copy_from_user((char *)&capacity , from_buf , to_copy);    // read capacity
        if(not_copied){
            printk(KERN_ALERT "MYLKM: could not copy %d out of %d bytes to kernel from process %d\n" , not_copied , to_copy , pid);
            mutex_unlock(&(process_p->lock));
            return -EACCES;
        }
        if(capacity < 1 || capacity > 100){
            printk(KERN_ALERT "MYLKM: requested capacity %d by process %d is out of allowed range [1,100]\n" , capacity , pid);
            mutex_unlock(&(process_p->lock));
            return -EACCES;
        }
        
        process_p->dq->capacity = capacity;     // initialize capacity of deque
        process_p->state = MATURE;              // change process state
        printk(KERN_INFO "MYLKM: initialized deque of capacity %d for process %d\n" , capacity , pid);
    }
    else{                                       // else, if in MATURE state
        int value = 0;
        int to_copy = min((int)from_len , 4);
        int not_copied = copy_from_user((char *)&value , from_buf , to_copy);       // copy from user space buffer
        if(not_copied){
            printk(KERN_ALERT "MYLKM: could not copy %d out of %d bytes to kernel from process %d\n" , not_copied , to_copy , pid);
            mutex_unlock(&(process_p->lock));
            return -EACCES;
        }

        if(value % 2){      // odd input, push front
            ret = push_front(process_p->dq , value);
            if(ret == -ENOMEM) printk(KERN_ALERT "MYLKM: error allocating space to push value %d in deque of process %d\n" , value , pid);
            if(ret == -EACCES) printk(KERN_ALERT "MYLKM: process %d attempted write value %d to full deque\n" , pid , value);
            if(ret < 0){
                mutex_unlock(&(process_p->lock));
                return ret;
            }
            printk(KERN_INFO "MYLKM: successfully copied %d bytes from process %d to kernel (pushed value %d to left end of deque)\n" , to_copy , pid , value);
        }
        else{               // even input, push back
            ret = push_back(process_p->dq , value);
            if(ret == -ENOMEM) printk(KERN_ALERT "MYLKM: error allocating space to push value %d in deque of process %d\n" , value , pid);
            if(ret == -EACCES) printk(KERN_ALERT "MYLKM: process %d attempted write value %d to full deque\n", pid , value);
            if(ret < 0){
                mutex_unlock(&(process_p->lock));
                return ret;
            }
            printk(KERN_INFO "MYLKM: successfully copied %d bytes from process %d to kernel (pushed value %d to right end of deque)\n" , to_copy , pid , value);
        }
    }

    mutex_unlock(&(process_p->lock));       // finally unlock process
    return ret;
}

/*
In open and close, both global lock and process lock are acquired. This ensures that no other thread is accessing anything while a process might possibly be de-allocated

In read and write, once the process is found from the process list, first the process lock is acquired after which it is safe to release the global lock
This adds parallelism since, now other threads can search the process list WHILE a given thread is accessing its deque
Also, since we acquire the process lock only after the process is found in the process list, other threads can access the deque WHILE a given thread is searching the process list

The above architecture guarantees maximum parallelism, since we can let the different threads parallely access NEITHER the global process list NOR the process deque
Since all possible threads of execution first attempt to lock global lock and then process lock
And this is done in a manner compliant with the 2-phase locking protocol (locking phase where both locks are acuired and unlocking phase where both are unlocked)
It is formally provable never to deadlock

All resources are cleaned in a naive fashion upon failures and destruction,
So no sort of buggy or malicious user-level code with races/etc. can ever cause a kernel level memory leak
*/
