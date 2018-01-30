/******************************************************************************
 *
 *   Name:           ipv6.h
 *
 *   Description:
 *
 *   Copyright:      (c) 2013-2050    IC Plus Corp.
 *                   All rights reserved.  By Chance
 *   Version: 1.0.0.20131114
 *******************************************************************************/
#ifndef _IPV6_H_
#define _IPV6_H_
#define IPV6_ADDRESS_LEN 16
#define IPV6_HEADER_LEN  40

#define ICMPV6_MSG_NS	135
#define ICMPV6_MSG_NA	136
typedef union VTF{
	u_int32_t image;
	struct
	{
		u_int32_t version:4;
		u_int32_t traffic_class:8;
		u_int32_t flow_label:20;
	};
}m_VTF;
typedef struct _IPV6_HEADER{
	m_VTF vtf;
	u_int16_t payload_len;
	u_int8_t  next_hdr;
	u_int8_t  hop_limit;
	u_int8_t  src[IPV6_ADDRESS_LEN];
	u_int8_t  dest[IPV6_ADDRESS_LEN];
}m_IPV6_HEADER;
typedef struct _UDP_HDR{
	u_int16_t srcport;
	u_int16_t destport;
	u_int16_t udplen;
	u_int16_t udpchksum;
	u_int8_t udpload[1];//udp load data
}m_UDP_HDR;
typedef struct _ICMPV6_HDR{
	u_int8_t type;
	u_int8_t code;
	u_int16_t chksum;
	u_int32_t flags;
	u_int8_t target_ip[IPV6_ADDRESS_LEN];
	u_int8_t option[1];
}m_ICMPV6_HDR;
enum{
	DHCPv6_Client_Solicit,
	DHCPv6_Client_Request,
	DHCPv6_Server_Relay_Reply,
	Neighbor_Advertisement,
	Relay6_Unknow_Packet};

#endif

