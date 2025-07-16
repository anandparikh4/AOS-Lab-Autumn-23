// Data structure implementation file

/*
DISCLAIMER: 
    The following code is not for general-purpose library usage by other kernel functionalities,
    but rather only for the mylkm driver
    (the only point of interest is the function implementation: destroy_process) 
*/

#include "defs.h"

// allocate a deque node with given value
struct node * init_node(int value){
    struct node * node_p = (struct node *)kmalloc(sizeof(struct node) , GFP_KERNEL);
    if(!node_p) return NULL;
    node_p->prev = node_p->next = NULL;
    node_p->value = value;
    return node_p;
}

// de-allocate the deque node
void destroy_node(struct node * node_p){
    node_p->prev = node_p->next = NULL;
    kfree(node_p);
    return;
}

// allocate a deque with given capacity
struct deque * init_deque(int capacity){
    struct deque * deque_p = (struct deque *)kmalloc(sizeof(struct deque) , GFP_KERNEL);
    if(!deque_p) return NULL;
    deque_p->front = deque_p->back = NULL;
    deque_p->size = 0;
    deque_p->capacity = capacity;
    return deque_p;
}

// de-allocate the deque
void destroy_deque(struct deque * deque_p){
    struct node * prev = NULL;
    struct node * curr = deque_p->front;
    deque_p->front = deque_p->back = NULL;
    while(curr){
        prev = curr;
        curr = curr->next;
        if(curr) curr->prev = NULL;
        destroy_node(prev);
    }
    kfree(deque_p);
    return;
}

// push given value at the front of the deque
int push_front(struct deque * deque_p , int value){
    if(deque_p->size >= deque_p->capacity) return -EACCES;
    struct node * node_p = init_node(value);
    if(!node_p) return -ENOMEM;
    if(deque_p->size == 0){
        deque_p->front = deque_p->back = node_p;
    }
    else{
        node_p->next = deque_p->front;
        deque_p->front->prev = node_p;
        deque_p->front = node_p;
    }
    (deque_p->size)++;
    return 0;
}

// push given value at the back of the deque
int push_back(struct deque * deque_p , int value){
    if(deque_p->size >= deque_p->capacity) return -EACCES;
    struct node * node_p = init_node(value);
    if(!node_p) return -ENOMEM;
    if(deque_p->size == 0){
        deque_p->back = deque_p->front = node_p;
    }
    else{
        node_p->prev = deque_p->back;
        deque_p->back->next = node_p;
        deque_p->back = node_p;
    }
    (deque_p->size)++;
    return 0;
}

// pop value from the front of the deque
int pop_front(struct deque * deque_p , int * value_p){
    if(deque_p->size <= 0) return -EACCES;
    *value_p = deque_p->front->value;
    struct node * node_p = deque_p->front;
    if(deque_p->size == 1){
        deque_p->front = deque_p->back = NULL;
    }
    else{
        deque_p->front = deque_p->front->next;
        deque_p->front->prev = NULL;
    }
    destroy_node(node_p);
    (deque_p->size)--;
    return 0;
}

// allocate a process with given pid
struct process * init_process(pid_t pid){
    struct process * process_p = (struct process *)kmalloc(sizeof(struct process) , GFP_KERNEL);
    if(!process_p) return NULL;
    struct deque * deque_p = init_deque(0);
    if(!deque_p){
        kfree(process_p);
        return NULL;
    }
    process_p->dq = deque_p;
    process_p->next = NULL;
    process_p->pid = pid;
    mutex_init(&(process_p->lock));
    process_p->state = NASCENT;
    return process_p;
}

// de-allocate the process
void destroy_process(struct process * process_p){
    mutex_unlock(&(process_p->lock));       // it is verifiable that all flows leading to this point have the mutex locked at this point
    mutex_destroy(&(process_p->lock));
    process_p->next = NULL;
    destroy_deque(process_p->dq);
    kfree(process_p);
    return;
}

// allocate a list
struct list * init_list(void){
    struct list * list_p = (struct list *)kmalloc(sizeof(struct list) , GFP_KERNEL);
    if(!list_p) return NULL;
    list_p->head = NULL;
    list_p->size = 0;
    return list_p;
}

// de-allocate the list
void destroy_list(struct list * list_p){
    struct process * prev = NULL;
    struct process * curr = list_p->head;
    list_p->head = NULL;
    while(curr){
        prev = curr;
        curr = curr->next;
        destroy_process(prev);
    }
    kfree(list_p);
    return;
}

// add process with given pid to the list
int add_process(struct list * list_p , pid_t pid){
    struct process * process_p = init_process(pid);
    if(!process_p) return -ENOMEM;
    process_p->next = list_p->head;
    list_p->head = process_p;
    (list_p->size)++;
    return 0;
}

// delete process with given pid from the list
int delete_process(struct list * list_p , pid_t pid){
    struct process * prev = NULL;
    struct process * curr = list_p->head;
    while(curr){
        if(curr->pid == pid){
            if(curr == list_p->head) list_p->head = curr->next;
            else prev->next = curr->next;
            destroy_process(curr);
            (list_p->size)--;
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    return -EINVAL;
}

// find process with given pid in the list
struct process * find_process(struct list * list_p , pid_t pid){
    struct process * prev = NULL;
    struct process * curr = list_p->head;
    while(curr){
        if(curr->pid == pid){
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}
