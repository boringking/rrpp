#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "rrpp_frame.h"


int is_rrpp_dst_mac(const uint8_t * mac){
	const uint8_t rrpp_dst_mac[6]=RRPP_DST_MAC;
	if( !memcmp(mac,rrpp_dst_mac,5) && mac[5]>=RRPP_DST_MAC_B5_BEGIN && mac[5]<=RRPP_DST_MAC_B5_END){
		return true;
	}
	return false;
}


static void fill_rrpp_frame(
	struct rrpp_vlan_packet * frame , 
	const uint8_t * local_mac , 
	uint8_t  rrpp_type,
	uint16_t vid,
	uint16_t did,
	uint16_t rid )
{
	// clear as 0
	memset(frame , 0 , sizeof(*frame));
	// ethhdr field
	uint8_t rrpp_dst_mac[6] = RRPP_DST_MAC;
	memcpy(frame->hdr.dst , rrpp_dst_mac,6);
	// rrpp_type与mac地址一一对应
	frame->hdr.dst[5] = rrpp_type;
	memcpy(frame->hdr.src , local_mac , 6);

	frame->vlan.tpid = htons(0x8100);
	frame->vlan.vid  = vid;
	
	// pre_rrpp_field
	frame->pre_rrpp.frame_length = htons(sizeof(struct pre_rrpp_field)+sizeof(struct rrpp_field)-sizeof(frame->pre_rrpp.frame_length) );
//	printf("frame_length = %d\n" ,frame->pre_rrpp.frame_length );
//	frame->pre_rrpp.const_0x00bb = 0x00bb; // big-endian
//	frame->pre_rrpp.const_0x0b99 = 0x0b99; // big-endian

	frame->rrpp.rrpp_length = sizeof(struct rrpp_field)-sizeof(frame->rrpp.rrpp_length);
//	printf("rrpp_length = %d\n" , frame->rrpp.rrpp_length);
	frame->rrpp.rrpp_type = rrpp_type;
	frame->rrpp.domain_id = did;
	frame->rrpp.ring_id   = rid;
	memcpy(frame->rrpp.sys_mac_addr,local_mac , 6);
	frame->rrpp.hello_timer = 1;
	frame->rrpp.fail_timer  = 3;
}

void fill_rrpp_hello_frame(
	struct rrpp_vlan_packet * frame , 
	const uint8_t * local_mac , 
	uint16_t vid,
	uint16_t did,
	uint16_t rid )
{
	fill_rrpp_frame(frame,local_mac,RRPP_TYPE_HELLO,vid,did,rid);
}

void fill_rrpp_linkup_frame(
	struct rrpp_vlan_packet * frame , 
	const uint8_t * local_mac , 
	uint16_t vid,
	uint16_t did,
	uint16_t rid )
{
	fill_rrpp_frame(frame,local_mac,RRPP_TYPE_LINK_UP,vid,did,rid);
}

void fill_rrpp_linkdown_frame(
	struct rrpp_vlan_packet * frame , 
	const uint8_t * local_mac , 
	uint16_t vid,
	uint16_t did,
	uint16_t rid )
{
	fill_rrpp_frame(frame,local_mac,RRPP_TYPE_LINK_DOWN,vid,did,rid);
}

void fill_rrpp_common_flush_fdb_frame(
	struct rrpp_vlan_packet * frame , 
	const uint8_t * local_mac , 
	uint16_t vid,
	uint16_t did,
	uint16_t rid )
{
	fill_rrpp_frame(frame,local_mac,RRPP_TYPE_COMMON_FLUSH_FDB,vid,did,rid);
}

void fill_rrpp_complete_flush_fdb_frame(
	struct rrpp_vlan_packet * frame , 
	const uint8_t * local_mac , 
	uint16_t vid,
	uint16_t did,
	uint16_t rid )
{
	fill_rrpp_frame(frame,local_mac,RRPP_TYPE_COMPLETE_FLUSH_FDB,vid,did,rid);
}


const void * cp_remove_special_tag(struct rrpp_vlan_packet * sendbuf,const char * buf){
	memcpy(sendbuf,buf,12);
	memcpy(&sendbuf->vlan,
	       buf+12+sizeof(struct special_tag_rx),
	       sizeof(*sendbuf)-12);
	return sendbuf;
}


