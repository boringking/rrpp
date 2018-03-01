#ifndef __TRANSPORT_NODE_H
#define __TRANSPORT_NODE_H


#include <pthread.h>
#include "types.h"
#include "raw_socket.h"
#include "thread.h"
#include "vlan.h"

enum transport_node_status{
	TRANSPORT_UP = 0,
	TRANSPORT_DOWN,
	TRANSPORT_PREFORWARDING,
};


/* 类的解释参考struct major_node */
struct transport_node{
	struct raw_socket raw_sock;
	/* attributes */
	uint16_t did;
	uint16_t rid;
	vlan_t   vlan[2];
	int  main_portno ;
	bool main_port_link;
	int  second_portno ;
	bool second_port_link;
	int  preforwarded_port;
	enum transport_node_status status;
	
	/* pthread */
	pthread_mutex_t access_mutex;
	pthread_cond_t  recv_complete_cond;
	pthread_mutex_t recv_complete_mutex;
	int    recv_complete_timeout;
	struct thread recv_thread;
	struct thread recv_complete_thread;
	struct thread detect_link_status_thread;
	struct thread check_complete_thread;
	
	/* base methods */
	int  (* sendto_port)(struct transport_node * this,int port,const void * src,int len);
	int  (* block_port)(struct transport_node * this, int port , const vlan_t * exclude);
	int  (* release_port)(struct transport_node * this, int port ,const vlan_t * exclude);
	int  (* refresh_fdb)(struct transport_node * this);
	int  (* judge_frame_type)(struct transport_node * this,const void * frame,int * port);
};


int transport_node_constructor(
	struct transport_node * this,
	uint16_t did,
	uint16_t rid,
	const uint16_t vid[2],
	int main_portno,
	int second_portno );
void transport_node_destructor(struct transport_node * this);
void * transport_node_recv_thread(void *);
void * transport_node_hello_thread(void *);
void * transport_node_check_hello_thread(void *);
int transport_node_sendto_port(struct transport_node *,int port,const void * src,int len);
int transport_node_block_port(struct transport_node * , int port , const vlan_t * exclude);
int transport_node_release_port(struct transport_node * , int port ,const vlan_t * exclude );
int transport_node_refresh_fdb(struct transport_node *);
void transport_node_recv_link_down(struct transport_node *,int );
void transport_node_recv_link_up(struct transport_node *,int);
void transport_node_recv_hello(struct transport_node *,int);
int transport_node_judge_frame_type(struct transport_node *,const void * frame,int * port);
static void transport_node_detect_link_status_handler(
	struct transport_node * this,
	int handle_portno,
	bool handle_port_link,
	int couple_portno,
	bool couple_port_link);
void * transport_node_detect_link_status_thread(void * arg);
void * transport_node_recv_complete_thread(void * arg);
void * transport_node_check_complete_thread(void * arg);




#endif

