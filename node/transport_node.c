#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include "kit.h"
#include "raw_socket.h"
#include "transport_node.h"
#include "rrpp_frame.h"
#include "sendto_port.h"
#include "vlan.h"
#include "thread.h"
#include "rrpp_regs.h"
#include "link_status.h"

static void * this_node;

static void transport_node_killer(int signo){
	printf("killing the node...\n");
	transport_node_destructor(this_node);
	exit(EXIT_SUCCESS);
}


int transport_node_constructor(
	struct transport_node * this,
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
	/* set according to real link status */
	//this->status  = TRANSPORT_UP;
	this->recv_complete_timeout = 0;
	this->sendto_port      = transport_node_sendto_port;
	this->block_port       = transport_node_block_port;
	this->release_port     = transport_node_release_port;
	this->refresh_fdb      = transport_node_refresh_fdb;
	this->judge_frame_type = transport_node_judge_frame_type;

	/* register signal handler(kill node) */
	if(SIG_ERR == signal(SIGINT ,transport_node_killer )){
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
	pthread_cond_init( &this->recv_complete_cond,NULL);
	pthread_mutex_init( &this->recv_complete_mutex,NULL);
	if(thread_constructor(&this->detect_link_status_thread,"detect-link-thread",DEFAULT_STACK_SIZE,transport_node_detect_link_status_thread,this)<0)
		goto THREAD_CONSTRUCTOR_FAIL;
	if(thread_constructor(&this->recv_thread,"recv-thread",DEFAULT_STACK_SIZE,transport_node_recv_thread,this)<0)
		goto THREAD_CONSTRUCTOR_FAIL;
	if(thread_constructor(&this->recv_complete_thread,"recv-complete-thread",DEFAULT_STACK_SIZE,transport_node_recv_complete_thread,this)<0)
		goto THREAD_CONSTRUCTOR_FAIL;
	if(thread_constructor(&this->check_complete_thread,"check-complete-thread",DEFAULT_STACK_SIZE,transport_node_check_complete_thread,this)<0)
		goto THREAD_CONSTRUCTOR_FAIL;
	// success
	printf("|--------------------------------------------------------|\n");
	return 0;

THREAD_CONSTRUCTOR_FAIL:
	printf("thread_constructor: failed\n");
	return -1;
}


void transport_node_destructor(struct transport_node * this){
//	thread_destructor(&this->detect_link_status_thread);
//	thread_destructor(&this->recv_thread);
//	thread_destructor(&this->recv_complete_thread);
//	thread_destructor(&this->check_complete_thread);
	/* destroy vlan */
	for(int i=0;i<ARRAY_SIZE(this->vlan);++i){
		if( !this->vlan[i].vid )
			continue;
		raw_vlan_rm_ports(this->vlan[i].vid,
		                   PBIT(this->main_portno)|PBIT(this->second_portno));
	}
	exec_command("rm " VLAN_DATA_PATH);
	exec_command("cli_stp enable stp ports 1-28");
}

int transport_node_judge_frame_type(struct transport_node * this,const void * frame,int * port){
	const struct rrpp_special_vlan_packet * pkt = (const struct rrpp_special_vlan_packet *)frame;

	if( !is_rrpp_dst_mac(pkt->hdr.dst) ){
		/* not 01-80-c2-00-00-11~1F */
		return RRPP_TYPE_NOP;
	}

	if( pkt->special.tag != SPECIAL_TAG_VAL){
		/* special tag not contained */
		return RRPP_TYPE_NOP;
	}

	/* obtain source port */
	*port = pkt->special.src_port+1; // (begin from 0)
	/* 必须从环内端口接收 */
	if((*port)!=this->main_portno && (*port)!=this->second_portno)
		return RRPP_TYPE_NOP;
	if( pkt->vlan.tpid != htons(0x8100) || pkt->vlan.vid != this->vlan->vid){
		/* no vlan tag or vid not match(major ring) */
		return RRPP_TYPE_NOP;
	}
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
		/* transport node receives hello/common/complete packet */
		case RRPP_TYPE_HELLO:
		case RRPP_TYPE_COMMON_FLUSH_FDB:
		case RRPP_TYPE_COMPLETE_FLUSH_FDB:
			return pkt->hdr.dst[5];
		default:
			/* rrpp packet type not match */
			return RRPP_TYPE_NOP;
	}
#endif
	return RRPP_TYPE_NOP;
}

static void transport_node_detect_link_status_handler(
	struct transport_node * this,
	int handle_portno,
	bool handle_port_link,
	int couple_portno,
	bool couple_port_link)
{
	struct rrpp_vlan_packet frame;
	if( TRANSPORT_DOWN==this->status && true==handle_port_link){
		if( true==handle_port_link && true==couple_port_link){
			/* do actions when both ports link-up */
			pthread_mutex_lock(&this->access_mutex);
			this->status = TRANSPORT_PREFORWARDING;
			// 保存被阻塞的端口，稍后恢复
			this->preforwarded_port = handle_portno;
			pthread_mutex_unlock(&this->access_mutex);
			printf("port %d up, status => preforwarding\n",handle_portno);
			this->block_port(this,handle_portno,this->vlan);
			fill_rrpp_linkup_frame(&frame,this->raw_sock.local_mac,this->vlan->vid,this->did,this->rid);
			this->sendto_port(this,couple_portno,&frame,sizeof frame);
		}
	}
	else if(TRANSPORT_UP==this->status && false==handle_port_link ){
		pthread_mutex_lock(&this->access_mutex);
		this->status=TRANSPORT_DOWN;
		pthread_mutex_unlock(&this->access_mutex);
		printf("port %d down, status => down\n",handle_portno);
		fill_rrpp_linkdown_frame(&frame,this->raw_sock.local_mac,this->vlan->vid,this->did,this->rid);
		this->sendto_port(this,couple_portno,&frame,sizeof frame);
	}
}

void * transport_node_detect_link_status_thread(void * arg){
	struct transport_node * this = (struct transport_node *)arg;
	this->main_port_link   = read_port_link_status(this->main_portno);
	this->second_port_link = read_port_link_status(this->second_portno);
	if( true==this->main_port_link && true==this->second_port_link){
		this->status = TRANSPORT_UP;
	}
	else{
		this->status = TRANSPORT_DOWN;
	}
	printf("initial status: %s\n",TRANSPORT_UP==this->status?"up":"down");
	for(;;){
		usleep(10000);   // 10ms
		bool main_port_link   = read_port_link_status(this->main_portno);
		bool second_port_link = read_port_link_status(this->second_portno);
		if( main_port_link!=this->main_port_link){
			/* main port link status changed */
			transport_node_detect_link_status_handler(this,
			                                          this->main_portno,
			                                          main_port_link,
			                                          this->second_portno,
			                                          second_port_link);
		}
		if(second_port_link!=this->second_port_link){
			/* second port link status changed */
			transport_node_detect_link_status_handler(this,
			                                          this->second_portno,
			                                          second_port_link,
			                                          this->main_portno,
			                                          main_port_link);
		}
		this->main_port_link=main_port_link;
		this->second_port_link=second_port_link;
	}
}

void * transport_node_recv_thread(void * arg){
	struct transport_node * this = (struct transport_node *)arg;
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
		int src_port;
		int rrpp_type = this->judge_frame_type(this,buf,&src_port);

		struct rrpp_vlan_packet sendbuf;
		switch( rrpp_type ){
			case RRPP_TYPE_HELLO:
//				printf("recv hello(%d)\n" , src_port);
				/* transport hello packet directly */
				if(src_port==this->main_portno){
					this->sendto_port(this,
					                  this->second_portno,
					                  cp_remove_special_tag(&sendbuf,buf),
					                  sizeof sendbuf);
				}
				else if(src_port==this->second_portno){
					this->sendto_port(this,
					                  this->main_portno,
					                  cp_remove_special_tag(&sendbuf,buf),
					                  sizeof sendbuf);
				}
				break;
			case RRPP_TYPE_COMMON_FLUSH_FDB:
				//printf("recv common-flush-fdb(%d)\n" , src_port);
				this->refresh_fdb(this);
				break;
			case RRPP_TYPE_COMPLETE_FLUSH_FDB:
			/*	// give it to another thread
				if( TRANSPORT_PREFORWARDING==this->status){
					this->recv_complete_timeout = 0;
					this->status = TRANSPORT_UP;
					this->refresh_fdb(this);
					this->release_port(this , this->main_portno);
					this->release_port(this , this->second_portno);
				}	*/
				//printf("recv complete-flush-fdb(%d)\n" , src_port);
				pthread_cond_signal(&this->recv_complete_cond);
				/* transport complete packet */
				if(src_port==this->main_portno){
					this->sendto_port(this,
					                  this->second_portno,
					                  cp_remove_special_tag(&sendbuf,buf),
					                  sizeof sendbuf);
				}
				else if(src_port==this->second_portno){
					this->sendto_port(this,
					                  this->main_portno,
					                  cp_remove_special_tag(&sendbuf,buf),
					                  sizeof sendbuf);
				}
				break;
			default:
				break;
		}
	}
	return NULL;
}

void * transport_node_recv_complete_thread(void * arg){
	struct transport_node * this = (struct transport_node *)arg;
	for(;;){
		/* wait complete-flush-fdb */
		pthread_mutex_lock(&this->recv_complete_mutex);
		pthread_cond_wait(&this->recv_complete_cond,&this->recv_complete_mutex);
		pthread_mutex_unlock(&this->recv_complete_mutex);

		/* handle */
		bool action = false;
		pthread_mutex_lock(&this->access_mutex);
		if(TRANSPORT_PREFORWARDING==this->status){
			this->status = TRANSPORT_UP;
			action = true;
		}
		this->recv_complete_timeout = 0;
		pthread_mutex_unlock(&this->access_mutex);
		
		if(action){
			printf("status => up\n");
			//this->release_port(this , this->main_portno);
			//this->release_port(this , this->second_portno);
			this->release_port(this , this->preforwarded_port,this->vlan);
			this->refresh_fdb(this);
		}
	}
}

void * transport_node_check_complete_thread(void * arg){
	struct transport_node * this = (struct transport_node *)arg;
	for(;;){
		usleep(100000);  // 100ms
		
		bool action=false;
		pthread_mutex_lock(&this->access_mutex);
		if( TRANSPORT_PREFORWARDING==this->status){
			if(this->recv_complete_timeout++ >=30){
				/* recv hello timedout... */
				this->recv_complete_timeout = 0;
				this->status = TRANSPORT_UP;
				action = true;
			}
		}
		else{
			//printf("cnter = %d\n",this->recv_hello_timeout);
			this->recv_complete_timeout = 0;
		}
		pthread_mutex_unlock(&this->access_mutex);
		
		/* do actions taking long time */
		if(action){
			printf("timeout, status => up\n");
			//this->sendto_port(this,this->main_portno,&frame,sizeof frame);
			//this->sendto_port(this,this->second_portno,&frame,sizeof frame);
			//this->release_port(this , this->main_portno);
			//this->release_port(this , this->second_portno);
			this->release_port(this , this->preforwarded_port,this->vlan);
			this->refresh_fdb(this);
		}
	}
	return NULL;
}

int transport_node_sendto_port(struct transport_node * this,int port,const void * src,int len){
	return sendto_port(&this->raw_sock,port,src,len);
}

int transport_node_block_port(struct transport_node * this , int port , const vlan_t * exclude){
	return block_port(this,port,exclude);
}

int transport_node_release_port(struct transport_node * this , int port ,const vlan_t * exclude){
	return release_port(this,port,exclude);
}

int transport_node_refresh_fdb(struct transport_node * this){
	return refresh_fdb(this);
}

