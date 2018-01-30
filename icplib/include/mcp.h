/*
 *	Multicast Control Protocol App; mcp.h
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
#ifndef _MCP_H_
#define _MCP_H_

#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#include "icp_timer.h"
#include "types.h"
#include "ip1829ds.h"

/*******************************************************************************
*	Defines
*******************************************************************************/
#define IPV4_VERSION			4
#define IPV6_VERSION			6

#define	ETH_ALEN				6
#define MCP_RECORD_ROUTER_PORT	BIT(5)
#define MCP_ROUTER				BIT(6)

#define MCT_MODE_EXCLUDE		0
#define MCT_MODE_INCLUDE		1
#define MCT_VERSION_V3			BIT(5)
#define MAX_SOURCE_LIST_TABLE	128
#define MAX_IPV4_SOURCE_ENTRY	10
#define MAX_IPV6_SOURCE_ENTRY	4

#define GROUP_RECORD_TYPE_IS_INCLUDE	1
#define GROUP_RECORD_TYPE_IS_EXCLUDE	2
#define GROUP_RECORD_TYPE_TO_INCLUDE	3
#define GROUP_RECORD_TYPE_TO_EXCLUDE	4
#define GROUP_RECORD_TYPE_ALLOW			5
#define GROUP_RECORD_TYPE_BLOCK			6

#define GROUP_MEMBERSHIP_INTERVAL	120

/*
 *	Ethernet Protocol ID
 */
#define ETH_P_IP			0x0800
#define	ETH_P_8021Q_S_TAG	0x88A8
#define	ETH_P_8021Q_C_TAG	0x8100
#define ETH_P_IPV6			0x86DD

/*
 *	Receive Packet Type
 */
#define PACK_T_UNKNOW	0
#define	PACK_T_IGMP		1
#define PACK_T_MLD		2

/*
 *	IP Protocol Numbers
 */
#define IP_PROTOCOL_IGMP		0x02
#define IP_PROTOCOL_ICMPV6		0x3A

/*
 *	Types of ICMPv6 Messages
 */
#define ICMPV6_UNKNOW			0
#define ICMPV6_MLD_QUERY		130
#define ICMPV6_MLD_REPORT		131
#define ICMPV6_MLD_DONE			132
#define ICMPV6_MLDV2_REPORT		143

/*******************************************************************************
*	Structures
*******************************************************************************/
struct router_port_control {
	struct list_head	d_router_port_list;
	unsigned int		s_router_ports;	// static router ports
	unsigned int		d_router_ports;	// dynamic router ports
};

struct mcp_man{
	pthread_t			input_thread;
	//pthread_t			timer_thread;
	pthread_t			cc_thread;
	
	pthread_mutex_t		mutex;
	u8					mcp_thread_status;

	u8					igmp_state;		// bit 7 : 0-disable, 1-enable
										// bit 6 : router
										// bit 5 : record router port
										// bit [1:0] : igmp version
	u8					mld_state;		// bit 7 : 0-disable, 1-enable
										// bit 6 : router
										// bit 5 : record router port
										// bit [1:0] : mld version

	u8					mt_method;		// ip1829 multicast table hashing method,
										// 0 : CRC, 1 : Direct
	
	int					igmp_group_aged_out;	// IGMP Group Aged Out
	int					igmp_gmi;				// IGMP Group Membership Interval
	int					igmp_router_aging;		// IGMP router port aging 
	int					mld_gmi;				// MLD Group Membership Interval
	int					mld_router_aging;		// MLD router port aging
	int					mld_group_aged_out;		// MLD Group Aged Out

	struct list_head	timer_list;
	struct list_head	mc_list;

	struct router_port_control	igmp_router_port_control;
	struct router_port_control	mld_router_port_control;

	// for IP1829
	u32					sl_used[4];		// IP1829 has 128 source list entries.
};

/*
struct ip6_addr{
	union{
		u8		u6_addr8[16];
		u16		u6_addr16[8];
		u32		u6_addr32[4];
	}_ip6_u;
#define	s6_addr			_ip6_u.u6_addr8
#define	s6_addr16		_ip6_u.u6_addr16
#define	s6_addr32		_ip6_u.u6_addr32
};
*/

struct router_port {
	struct mcp_man				*mcp;
	struct router_port_control	*rpc;
	int							port_no;
	struct list_head			d_router_port_entry;
	icp_timer					aging_timer;
};

struct mc_source_record{
	struct mc_port_record	*mp;
	struct list_head		p_sl_entry;
	
	union{
		struct in_addr			ip4_addr;
		struct in6_addr			ip6_addr;
	}_sour_addr;
#define sr_addr			_sour_addr.ip4_addr.s_addr
#define sr6_addr		_sour_addr.ip6_addr.s6_addr
#define sr6_addr16		_sour_addr.ip6_addr.s6_addr16
#define sr6_addr32		_sour_addr.ip6_addr.s6_addr32

	icp_timer	source_timer;
};

struct mc_port_record{
	struct mc_entry *mc;

	struct list_head	port_entry;
	struct list_head	p_sl_list;

	int 		port_no;
	int			sw_port_no;
	u8			mode;
	time_t		uptime;

	icp_timer	group_timer;
};

struct mc_entry{
	struct mcp_man		*mcp;

	struct list_head	mc_entry;
	struct list_head	port_list;

	//u8		mac_addr[6];
	int		status;				// 0-dynamic, 1-static
	u8		ipv;				// 4-ipv4, 6-ipv6
	u8		index;				// for IP1829's memory address
	u16		source_state;		// one source one bit, 0 - unused, 1 - used. 
	
	union{
		struct in_addr			ip4_addr;
		struct in6_addr			ip6_addr;
	}group_addr;
#define g4_addr			group_addr.ip4_addr.s_addr
#define g6_addr			group_addr.ip6_addr.s6_addr
#define g6_addr16		group_addr.ip6_addr.s6_addr16
#define g6_addr32		group_addr.ip6_addr.s6_addr32
	
	// for IP1829
	struct mt_rule		m_rule;
	struct slt_rule		sl_rule;
};

struct mcp_pg_time
{
	double	uptime;
	int		expire_time;
};
/*

struct mt_rule{
	unsigned char group[4];
	unsigned char fid;
	unsigned long port_mem;
	unsigned char slt_index;
	unsigned long port_filter_mode;		// 0 - exclude, 1 - include.
	unsigned char flag;					// [0:2] - priority
										// [3:4] - snif
										// [5] - ver, 1 for IGMPv3
										// [6] - CPU.
};

struct slt_rule{
	unsigned char type;	//4:ipv4, 6:ipv6, define in ip1829op.h
	union slt_para{
		struct slt_ipv4{
			unsigned char ip[10][4];
			unsigned long used_port[10];
		}ipv4;

		struct slt_ipv6{
			unsigned short ip[4][8];
			unsigned long used_port[4];
		}ipv6;
	}data;
};

 */

#endif
