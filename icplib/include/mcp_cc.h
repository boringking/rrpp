/*
 *	IP1829 Common Control; mcp_cc.h
 *	
 *	IC+ Corp.
 *
 */
#ifndef _MCP_CC_H_
#define _MCP_CC_H_

#include <arpa/inet.h>

/******************************************************************************
*	Defines
******************************************************************************/

#define CONFIG_ACT_CLEAR_ENTRY				1
#define CONFIG_ACT_CREATE_STATIC_GROUP		2
#define CONFIG_ACT_DELETE_STATIC_GROUP		3
//#define CONFIG_ACT_ADD_STATIC_GROUP_MEM		4
//#define CONFIG_ACT_DELETE_STATIC_GROUP_MEM	5
#define CONFIG_ACT_CLEAR_ALL_IGMP_ENTRY		6
#define CONFIG_ACT_CLEAR_ALL_MLD_ENTRY		7
#define CONFIG_ACT_GET_PORT_GROUP_TIME		8
#define CONFIG_ACT_STATIC_GROUP_MEM			9

/*******************************************************************************
*	Structures
*******************************************************************************/
struct mcp_cc_entry
{
	int		action;		// 1-clear entry
						// 2-create static group
						// 3-delete static group
						// 4-add static group members
						// 5-delete static group members
						// 6-clear all igmp entry
						// 7-clear all mld entry
						// 8-get port-group time
	int		ipv;
	int		portno;		// for action 8
	unsigned int	ports;	// for action 4,5
	union{
		struct in_addr		ip4_addr;
		struct in6_addr		ip6_addr;
	}group_addr;
#define	g4_addr		group_addr.ip4_addr.s_addr
#define g6_addr		group_addr.ip6_addr.s6_addr
#define g6_addr16	group_addr.ip6_addr.s6_addr16
#define g6_addr32	group_addr.ip6_addr.s6_addr32
};
/*
struct mcp_pg_time
{
	double	uptime;
	int		expire_time;
};
*/
#endif
