#ifndef __KIT_H
#define __KIT_H


#include "types.h"


#define  time_test_begin() \
	struct timeval time;time_test(__func__, &time, 0);
#define  time_test_end() \
	time_test(__func__, &time,1);


void maketimeout(struct timespec *, int);
int exec_command(const char * command);
int request_command(const char * command);
int request_clish(char* pszCookie,char* command,int recvbuf_size,char* recvbuf);
int enable_stp(bool set);
int enable_stp_ports(int port , bool set);
int enable_mac_learning(int port , bool set);
int refresh_fdb(void * arg);
void hexdump(const void * p , int len);
int kill_stp(void);
void time_test(const char * runname,struct timeval * time ,int step);





#endif

