#ifndef __MAJOR_NODE_H
#define __MAJOR_NODE_H


#include <pthread.h>
#include "types.h"
#include "raw_socket.h"
#include "thread.h"
#include "vlan.h"

enum major_node_status{
	MAJOR_COMPLETE = 0,
	MAJOR_FAILED,
};

struct major_node{
	struct raw_socket raw_sock;
	/* attributes */
	uint16_t did;
	uint16_t rid;
	vlan_t   vlan[2];
	int main_portno;
	int second_portno;
	enum major_node_status status;
	
	/* pthread */
	pthread_mutex_t access_mutex;
	int    recv_hello_timeout;
	struct thread recv_thread;
	struct thread hello_thread;
	struct thread check_hello_thread;
	
	/* base methods */
	int  (* sendto_port)(struct major_node * this,int port,const void * src,int len);
	int  (* block_port)(struct major_node * this, int port , const vlan_t * exclude);
	int  (* release_port)(struct major_node * this, int port ,const vlan_t * exclude);
	int  (* refresh_fdb)(struct major_node * this);
	int  (* judge_frame_type)(struct major_node * this,const void * frame,int * port);
	void (* recv_hello)(struct major_node * this, int port);
	void (* recv_link_down)(struct major_node * this, int port);
	void (* recv_link_up)(struct major_node * this, int port);
};


int major_node_constructor(
	struct major_node * this,
	uint16_t did,
	uint16_t rid,
	const uint16_t vid[2],
	int main_portno,
	int second_portno );
void major_node_destructor(struct major_node * this);
void * major_node_recv_thread(void *);
void * major_node_hello_thread(void *);
void * major_node_check_hello_thread(void *);
int major_node_sendto_port(struct major_node *,int port,const void * src,int len);
int major_node_block_port(struct major_node * , int port , const vlan_t * exclude);
int major_node_release_port(struct major_node * , int port ,const vlan_t * exclude);
int major_node_refresh_fdb(struct major_node *);
void major_node_recv_link_down(struct major_node *,int );
void major_node_recv_link_up(struct major_node *,int);
void major_node_recv_hello(struct major_node *,int);
int major_node_judge_frame_type(struct major_node *,const void * frame,int * port);




#endif

