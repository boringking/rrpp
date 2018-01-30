/*
 *	Multicast Control Protocol App; mld.h
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
#ifndef _MLD_H_
#define _MLD_H_

#include "libnetapp.h"
//#include "types.h"
//#include "ip1829_netb.h"
/*******************************************************************************
*	Define
*******************************************************************************/
#define IPV6_EXT_H_HOP_BY_HOP		0
#define IPV6_EXT_H_DST				60
#define IPV6_EXT_H_ROUTING			43
#define IPV6_EXT_H_FRAGMENT			44
#define IPV6_EXT_H_AUTHEN			51
#define IPV6_EXT_H_ESP				50
#define IPV6_EXT_H_MOBILITY			135

/*******************************************************************************
*	Structures
*******************************************************************************/
#pragma pack(push, 1)

struct ipv6_hdr{
	u32		version:4;
	u32		traffic_class:8;
	u32		flow_label:20;
	u16		payload_len;
	u8		next_header;
	u8		hop_limit;

	u16		sip6[8];
	u16		dip6[8];
};

struct hop_by_hop_hdr{
	u8		next_header;
	u8		hdr_ext_len;
};

struct mld_message{
	u8		type;
	u8		code;
	u16		check_sum;
	u16		max_response_delay;
	u16		reserved;
	
	u16		addr[8];
};

struct mldv2_hdr{
	u8		type;
	u8		code;
	u16		check_sum;
	u16		reserved;
	u16		num_records;
};

struct mldv2_record_hdr{
	u8		record_type;
	u8		aux_len;
	u16		number_of_sources;
	u16		multi_addr[8];
};

#pragma pack(pop)

struct mld_buf{
	struct pro_buff *pb;

	u16		sip6[8];
	u16		dip6[8];
};

#endif
