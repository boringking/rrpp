/*
 *	Link Aggregation Control Protocol; icp_lacp_config.h
 *	
 *	IC+ Corp.
 *
 *	Authors:
 *	Larry Chiu
 *
 */
#ifndef _ICP_LACP_CONFIG_H_
#define _ICP_LACP_CONFIG_H_

#include "types.h"
#include "dev_info.h"

/*******************************************************************************
*	Defines
*******************************************************************************/
#define DEV_IP1829
/*******************************************************************************
*	Structures
*******************************************************************************/
struct lacp_config_group
{
	u8		state;		// bit0:Enable/Disable
						// bit1:LACP/Static
						// bit2:Acitive/Passive
						// bit3:ShortTimeOut/LongTimeOut
	u16		opKey;
};

struct lacp_config
{
	u16		sys_priority;
	u16		port_priority[28];
	u8		port_group_index[28];
	u8		hash_method;		// 0: Port ID / port
								// 1: SA / sa
								// 2: DA / da
								// 3: DA/SA /dasa
								// 4: DIP / dip
								// 5: SIP / sip
								// 6: TCP/UDP DP / tcpudpdp
								// 7: TCP/UDP SP / tcpudpsp

	struct lacp_config_group	lacp_group[TRUNK_GROUP_NUM];

#ifdef	DEV_IP1829
	// IP1829 individual information
	u16		devinfo;			// Bit 0: combine Trunk Group A and Trunk Group B
								// Bit 1: combine Trunk Group C and Trunk Group D
								// Bit 2: combine Trunk Group E and Trunk Group F
#endif
};

#endif
