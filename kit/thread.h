#ifndef  __THREAD_H
#define  __THREAD_H




struct thread {
    const char * name;
    pthread_t tid;
    //void * stackaddr;
    void * (*routine) (void *);
};



#define DEFAULT_STACK_SIZE  (128*1024)



int thread_constructor (struct thread * this , const char * name, int stacksize,
                        void * (*routine) (void *), void * arg);
void thread_destructor (struct thread * this);


#endif

