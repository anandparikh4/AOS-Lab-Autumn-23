// Header file

#ifndef __MYLKM_H
#define __MYLKM_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/slab.h>

// deque node structure
typedef struct node{
    struct node * prev;     // pointer to previous node
    struct node * next;     // pointer to next node
    int value;              // value stored in node
}node;
struct node * init_node(int);
void destroy_node(struct node *);

// deque structure
typedef struct deque{
    struct node * front;    // pointer to leftmost node
    struct node * back;     // pointer to rightmost node
    int capacity;
    int size;
}deque;
struct deque * init_deque(int);
void destroy_deque(struct deque *);
int push_front(struct deque * , int);
int push_back(struct deque * , int);
int pop_front(struct deque * , int *);

// the possible states in which the process can be
enum process_state{
    NASCENT,        // after opening procfile, but before assiging capacity
    MATURE          // after assiging capacity
};

// process structure
typedef struct process{
    pid_t pid;                  // process id
    struct mutex lock;          // mutex for accessing deque
    struct process * next;      // pointer to next process
    struct deque * dq;          // pointer to deque
    enum process_state state;   // state of the process in its lifetime
}process;
struct process * init_process(pid_t);
void destroy_process(struct process *);

// process list structure
typedef struct list{
    struct process * head;  // pointer to first process
    int size;               // number of processes
}list;
struct list * init_list(void);
void destroy_list(struct list *);
int add_process(struct list * , pid_t);
int delete_process(struct list * , pid_t);
struct process * find_process(struct list * , pid_t);

#endif
