#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "thread.h"

/* 构造线程对象(创建线程) */
int thread_constructor(struct thread * this ,const char * name, int stacksize,void *(*routine)(void*),void * arg){
	this->name = name;
	/* set stack */
	pthread_attr_t attr;
	pthread_attr_init(&attr);
//	this->stackaddr = malloc(stacksize);
//	if( !this->stackaddr )
//		return -1;

	pthread_attr_setstacksize(&attr , stacksize);
	/* thread routine assignment */
	this->routine = routine;
	if(pthread_create(&this->tid,&attr,this->routine,arg)){
		return -1;
	}
	return 0;
}

void thread_destructor(struct thread * this){
	if( pthread_cancel(this->tid) )
		fprintf(stderr , "destroy %s: failed\n" , this->name);
	printf("destroy %s: success\n" , this->name);
}

