#ifndef __LIB_COMMON_H__
#define __LIB_COMMON_H__
#include <sys/types.h>//for u_int8_t

#define OK		0
#define	ERROR	-1

#define FALSE	0
#define TRUE	1

#define DISABLE	0
#define ENABLE	1

#define FREE_SAFE(a)	if(a!=NULL){free(a);a=NULL;}

/************************************************************
 * Name:	getSysUptime
 * Description: get system uptime from proc file
 * Parameters:	None
 * Return value:failed:		-1
 *				success:	float type uptime
 * **********************************************************/
float getSysUptime(void);
void daemonize();
int lock_file(char *filename);
int unlock_file(int fd, char *filename);

void get_switch_mac(u_int8_t* macaddr);
void get_switch_ip(u_int8_t *ip);
void get_switch_netmask(u_int8_t *mask);
void get_gateway_ip(u_int8_t *ip);
int FlushLutbyPortmap(const unsigned long portmap, const int pmdata);
#endif//__LIB_COMMON_H__
