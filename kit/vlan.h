#ifndef  __VLAN_H
#define  __VLAN_H


#include "types.h"
#include "vlan_raw.h"

typedef struct {
	char name[64];
	uint16_t vid;
} vlan_t;




// vlan 字段
struct vlan_field {
//  unsigned short ethertype;          //报文封装类型域
//  unsigned char vid_h:4;
//  unsigned char cfi:1;             //COS（Class of Service）优先级
//  unsigned char pri:3;   
//  unsigned char vid_l:8;            //vlan:1-4094,0,4095预留，1为默认
  unsigned short tpid;          //报文封装类型域
  unsigned int   pri:3;   
  unsigned int   cfi:1;             //COS（Class of Service）优先级
  unsigned int   vid:12;
} __attr_packed__ ;



#define VLAN_DATA_PATH   "/home/rrpp/vlan-data"



int fill_vlan_frame(void * dst , const void * src , int len , uint16_t vid);
int vlan_delete_entry(const char * name);
int vlan_add_entry(const char * name , int vid);
int vlan_delete_port_from_entry(const char * name , int port);
int vlan_add_port_into_entry(const char * name , int port);
const vlan_t ** vlan_read_all_entry_name(void);
int block_port(void * arg , int port , const vlan_t * exclude);
int release_port(void * arg , int port ,const vlan_t *);
int vlan_create_data_file(void);






#endif

