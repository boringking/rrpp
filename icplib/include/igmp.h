/*
 *	Multicast Control Protocol App; igmp.h
 *	
 *	An implementation of Internet Group Management Protocol (IGMP) and Multicast
 *	Listener Discovery (MLD) Snooping.
 *
 *	IC+ Corp.
 *
 *	Authors:
 *	Larry Chiu
 *
 */
#ifndef _IGMP_H_
#define _IGMP_H_

#include "libnetapp.h"
//#include "types.h"
//#include "ip1829_netb.h"
/*******************************************************************************
*	Define
*******************************************************************************/
#define IGMP_PACKET_TYPE_UNKNOW			0x00
#define IGMP_PACKET_TYPE_QUERY			0x11
#define IGMP_PACKET_TYPE_V1_REPORT		0x12
#define IGMP_PACKET_TYPE_V2_REPORT		0x16
#define IGMP_PACKET_TYPE_V2_LEAVE		0x17
#define IGMP_PACKET_TYPE_V3_REPORT		0x22

/*******************************************************************************
*	Structures
*******************************************************************************/
#pragma pack(push, 1)

struct ipv4_hdr{
	u8		version:4;
	u8		ihl:4;
	u8		tos;
	u16		tot_len;
	u16		id;
	u16		frag_off;
	u8		ttl;
	u8		protocol;
	u16		check_sum;
	u32		saddr;
	u32		daddr;
};

struct igmp_hdr{
	u8		type;
	u8		code;
	u16		check_sum;
	union{
		struct _v3_hdr{
			u16 reserved;
			u16	num_of_group_records;
		} v3_hdr;
		u32		group_addr;
	}ng;
#define		group_addr		ng.group_addr
#define		v3_num_of_gr	ng.v3_hdr.num_of_group_records
};

struct igmpv3_group_record_hdr{
	u8		record_type;
	u8		aux_len;
	u16		number_of_sources;
	u32		multi_addr;
};

#pragma pack(pop)
#endif

struct igmp_buf{
	struct pro_buff *pb;

	u32		sip;
	u32		dip;
};

