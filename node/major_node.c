#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include "kit.h"
#include "raw_socket.h"
#include "major_node.h"
#include "rrpp_frame.h"
#include "sendto_port.h"
#include "vlan.h"
#include "thread.h"
#include "rrpp_regs.h"

static void * this_node;

static void major_node_killer(int signo){
	printf("killing the node...\n");
	major_node_destructor(this_node);
	exit(EXIT_SUCCESS);
}

int major_node_constructor(
	struct major_node * this,
	uint16_t did,
	uint16_t rid,
	const uint16_t vid[2],
	int main_portno,
	int second_portno )
{
	memset(this,0,sizeof *this);
	this_node = this;
	this->did = did;
	this->rid = rid;
	this->vlan[0].vid=vid[0];
	sprintf(this->vlan[0].name , "rrpp%d",this->vlan[0].vid);
	this->vlan[1].vid=vid[1];
	sprintf(this->vlan[1].name , "rrpp%d",this->vlan[1].vid);
	this->main_portno   = main_portno;
	this->second_portno = second_portno;
	this->status  = MAJOR_FAILED;
	this->recv_hello_timeout = 0;
	this->sendto_port      = major_node_sendto_port;
	this->block_port       = major_node_block_port;
	this->release_port     = major_node_release_port;
	this->refresh_fdb      = major_node_refresh_fdb;
	this->judge_frame_type = major_node_judge_frame_type;
	this->recv_hello       = major_node_recv_hello;
	this->recv_link_down   = major_node_recv_link_down;
	this->recv_link_up     = major_node_recv_link_up;

	/* register signal handler(kill node) */
	if(SIG_ERR == signal(SIGINT ,major_node_killer )){
		fprintf(stderr, "signal: failed\n");
		return -1;
	}

	/* disbale stp */
//	enable_stp(false);
//	enable_stp_ports(this->main_portno,false);
//	enable_stp_ports(this->second_portno,false);
	if(kill_stp()<0)
		return -1;
	/* rrpp relative registers init */
	if(rrpp_regs_config()<0){
		return -1;
	}
	/* init socket */
	if(raw_socket_init(&this->raw_sock, "eth0")<0){
		printf("raw_socket_init: error\n");
		return -1;
	}

	/*------vlan------*/
	if(vlan_create_data_file()<0)
		return -1;
	/* config as tag vlan */
	exec_command("cli_vlan config vlan tagging vid");
	exec_command("cli_vlan config vlan type tag");
	/* create vlan */
	for(int i=0;i<ARRAY_SIZE(this->vlan);++i){
		//fgetc(stdin);
		//printf("vid%d = %d\n" , i,this->vlan[i].vid);
		if( !this->vlan[i].vid )
			continue;
#if  0
		vlan_add_entry(this->vlan[i].name,this->vlan[i].vid);
		vlan_add_port_into_entry(this->vlan[i].name,this->main_portno);
		vlan_add_port_into_entry(this->vlan[i].name,this->second_portno);
#else
		raw_vlan_add_ports(this->vlan[i].vid,
		                   PBIT(this->main_portno)|PBIT(this->second_portno));
#endif
	}
	
	/* create threads  */
	pthread_mutex_init(&this->access_mutex ,NULL);
	if(thread_constructor(&this->recv_thread,"recv-thread",DEFAULT_STACK_SIZE,major_node_recv_thread,this)<0)
		goto THREAD_CONSTRUCTOR_FAIL;
	if(thread_constructor(&this->hello_thread,"hello-thread",DEFAULT_STACK_SIZE,major_node_hello_thread,this)<0)
		goto THREAD_CONSTRUCTOR_FAIL;
	if(thread_constructor(&this->check_hello_thread,"check-hello-thread",DEFAULT_STACK_SIZE,major_node_check_hello_thread,this)<0)
		goto THREAD_CONSTRUCTOR_FAIL;
	// success
	printf("|--------------------------------------------------------|\n");
	return 0;

THREAD_CONSTRUCTOR_FAIL:
	printf("thread_constructor: failed\n");
	return -1;
}

void major_node_destructor(struct major_node * this){
//	thread_destructor(&this->hello_thread);
//	thread_destructor(&this->recv_thread);
//	thread_destructor(&this->check_hello_thread);
	/* destroy vlan */
	for(int i=0;i<ARRAY_SIZE(this->vlan);++i){
		if( !this->vlan[i].vid )
			continue;
		raw_vlan_rm_ports(this->vlan[i].vid,
		                   PBIT(this->main_portno)|PBIT(this->second_portno));
	}
	exec_command("rm "VLAN_DATA_PATH);
	exec_command("cli_stp enable stp ports 1-28");
}

void * major_node_recv_thread(void * arg){
	struct major_node * this = (struct major_node *)arg;
	char buf[1024]; 
	socklen_t addrlen;
	struct sockaddr_ll addr;

	//printf("sizeof(struct sockaddr_ll): %d\n",sizeof(addr));
	for(int cnter=1;;++cnter){
		int recv_len;
		if( (recv_len = raw_socket_recvfrom(&this->raw_sock, buf, sizeof(buf), &addr,&addrlen))<0){
			perror("recvfrom");
			return NULL;
		}
/*		const uint8_t rrpp_dst_mac[6] = 
			{0x01,0x80,0xc2,0x00,0x00,0x11};
		if( !memcmp(rrpp_dst_mac , buf,6) ){
			printf("get a rrpp frame\n");
		}*/
		//printf("[%d]get a packet\n" , cnter);
		int src_port;
		int rrpp_type = this->judge_frame_type(this,buf,&src_port);
		switch( rrpp_type ){
			case RRPP_TYPE_HELLO:
				this->recv_hello(this , src_port);
				break;
			case RRPP_TYPE_LINK_DOWN:
				this->recv_link_down(this, src_port);
				break;
			case RRPP_TYPE_LINK_UP:
				this->recv_link_up(this, src_port);
				break;
			default:
				break;
		}
	}
	return NULL;
}


void * major_node_hello_thread(void * arg){
	struct major_node * this = (struct major_node *)arg;
	
	struct rrpp_vlan_packet frame;
	fill_rrpp_hello_frame(&frame , 
	                this->raw_sock.local_mac,
	                this->vlan->vid,
	                this->did,
	                this->rid);
	//hexdump(&frame, sizeof frame);
	for(int cnter = 0;;cnter++){
		sleep(1);
		this->sendto_port(this ,this->main_portno,&frame,sizeof frame);
	}
	return NULL;
}

void * major_node_check_hello_thread(void * arg){
	struct major_node * this = (struct major_node *)arg;
	struct rrpp_vlan_packet frame;
	fill_rrpp_complete_flush_fdb_frame(&frame,this->raw_sock.local_mac,this->vlan->vid,this->did,this->rid);
	for(;;){
		usleep(100000);  // 100ms
		
		bool action=false;
		pthread_mutex_lock(&this->access_mutex);
		if( MAJOR_COMPLETE==this->status){
			if(this->recv_hello_timeout++ >=30){
				/* recv hello timedout... */
				this->recv_hello_timeout = 0;
				this->status = MAJOR_FAILED;
				action = true;
			}
		}
		else{
			//printf("cnter = %d\n",this->recv_hello_timeout);
			this->recv_hello_timeout = 0;
		}
		pthread_mutex_unlock(&this->access_mutex);
		
		/* do actions taking long time */
		if(action){
			printf("timeout, status => failed\n");
			this->release_port(this , this->second_portno,this->vlan);
			this->refresh_fdb(this);
			this->sendto_port(this,this->main_portno,&frame,sizeof frame);
			this->sendto_port(this,this->second_portno,&frame,sizeof frame);
		}
	}
	return NULL;
}


int major_node_judge_frame_type(struct major_node * this,const void * frame,int * port){
	const struct rrpp_special_vlan_packet * pkt = (const struct rrpp_special_vlan_packet *)frame;

	if( !is_rrpp_dst_mac(pkt->hdr.dst) ){
		/* not 01-80-c2-00-00-11~1F */
		return RRPP_TYPE_NOP;
	}
//	printf("dst mac ok\n");

	if( pkt->special.tag != SPECIAL_TAG_VAL){
		/* special tag not contained */
		return RRPP_TYPE_NOP;
	}
//	printf("special tag ok\n");

	/* obtain source port */
	*port = pkt->special.src_port+1; // (begin from 0)
	if( pkt->vlan.tpid != htons(0x8100) || pkt->vlan.vid != this->vlan->vid){
		/* no vlan tag or vid not match(major ring) */
		return RRPP_TYPE_NOP;
	}
//	printf("vlan tag ok\n");
#if  1
	if( pkt->rrpp.domain_id != this->did || pkt->rrpp.ring_id != this->rid){
		/* domain id or ring id not match */
		return RRPP_TYPE_NOP;
	}
#endif

#if  0
	switch( pkt->rrpp.rrpp_type ){
		case RRPP_TYPE_HELLO:
		case RRPP_TYPE_LINK_UP:
		case RRPP_TYPE_LINK_DOWN:
			return pkt->rrpp.rrpp_type;
		default:
			/* rrpp packet type not match */
			return RRPP_TYPE_NOP;
	}
#else
	switch( pkt->hdr.dst[5]){
		case RRPP_TYPE_HELLO:
		case RRPP_TYPE_LINK_UP:
		case RRPP_TYPE_LINK_DOWN:
			return pkt->hdr.dst[5];
		default:
			/* rrpp packet type not match */
			return RRPP_TYPE_NOP;
	}
#endif
	return RRPP_TYPE_NOP;
}

void major_node_recv_hello(struct major_node * this , int port){
//	printf("recv a hello(%d)\n",port);
	if(port == this->second_portno){
		bool action = false;
		pthread_mutex_lock(&this->access_mutex);
		if(MAJOR_FAILED==this->status){
			this->status = MAJOR_COMPLETE;
			action = true;
		}
		/* clear timeout counter */
		this->recv_hello_timeout = 0;
		pthread_mutex_unlock(&this->access_mutex);

		/* do actions taking long time */
		if(action){
			printf("status => complete\n");
			this->block_port(this,this->second_portno,this->vlan);
			this->refresh_fdb(this);
			struct rrpp_vlan_packet frame;
			fill_rrpp_complete_flush_fdb_frame(&frame,this->raw_sock.local_mac,this->vlan->vid,this->did,this->rid);
			this->sendto_port(this,this->main_portno,&frame,sizeof frame);
		}
	}
}

void major_node_recv_link_down(struct major_node * this,int port){
	//printf("recv a linkdown\n");
	if( port!=this->main_portno && port!=this->second_portno){
		/* recv from a port not in ring */
		return;
	}
	bool action = false;
	pthread_mutex_lock(&this->access_mutex);
	if( MAJOR_COMPLETE==this->status){
		this->status = MAJOR_FAILED;
		action = true;
	}
	pthread_mutex_unlock(&this->access_mutex);

	/* do actions taking long time */
	if(action){
		printf("status => failed\n");
		this->release_port(this,this->second_portno,this->vlan);
		this->refresh_fdb(this);
		// send common flush fdb to main/second port
		struct rrpp_vlan_packet frame;
		fill_rrpp_common_flush_fdb_frame(&frame,this->raw_sock.local_mac,this->vlan->vid,this->did,this->rid);
		this->sendto_port(this,this->main_portno,&frame,sizeof frame);
		this->sendto_port(this,this->second_portno,&frame,sizeof frame);

	}
}

void major_node_recv_link_up(struct major_node * this,int port){
	//printf("recv a linkup\n");
	if( port!=this->main_portno && port!=this->second_portno){
		/* recv from a port not in ring */
		return;
	}
	bool action = false;
	pthread_mutex_lock(&this->access_mutex);
	if(MAJOR_FAILED == this->status){
		action = true;
	}
	pthread_mutex_unlock(&this->access_mutex);

	/* do actions taking long time */
	if(action){
		// send common flush fdb to main/second port
		struct rrpp_vlan_packet frame;
		fill_rrpp_common_flush_fdb_frame(&frame,this->raw_sock.local_mac,this->vlan->vid,this->did,this->rid);
		this->sendto_port(this,this->main_portno,&frame,sizeof frame);
		this->sendto_port(this,this->second_portno,&frame,sizeof frame);
		/* accelerate shift(seems useless)... */
		struct rrpp_vlan_packet hello;
		fill_rrpp_hello_frame(&hello,this->raw_sock.local_mac,this->vlan->vid,this->did,this->rid);
		this->sendto_port(this,this->main_portno,&hello,sizeof hello);
	}
}

int major_node_sendto_port(struct major_node * this,int port,const void * src,int len){
	return sendto_port(&this->raw_sock,port,src,len);
}

int major_node_block_port(struct major_node * this , int port , const vlan_t * exclude){
	enable_mac_learning(port,false);
	return block_port(this,port,exclude);
}

int major_node_release_port(struct major_node * this , int port , const vlan_t * exclude){
	enable_mac_learning(port,true);
	return release_port(this,port,exclude);
}

int major_node_refresh_fdb(struct major_node * this){
	return refresh_fdb(this);
}

