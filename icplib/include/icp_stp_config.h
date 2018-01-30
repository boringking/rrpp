/*
 *	Multiple Spanning Tree Protocol; icp_stp_config.h
 *	
 *	The configuration using to restore in a file.
 *
 *	IC+ Corp.
 *
 *	Authors:
 *	Larry Chiu
 *
 */
#ifndef _ICP_STP_CONFIG_H_
#define _ICP_STP_CONFIG_H_

#include "types.h"
#include "dev_info.h"

/*******************************************************************************
*	Defines
*******************************************************************************/

/*******************************************************************************
*	Structures
*******************************************************************************/
struct msti_vlan_range
{
	u16	v_range[256];
};

struct stp_config
{
	// temporary
	//u8		cpu_mac[6];

	u8		stp_enabled;		//enum{BR_NO_STP,BR_STP,BR_RSTP,BR_MSTP}
	u32		stp_enabled_ports;
	u16		max_age;
	u16		hello_time;
	u16		forward_delay;
	u8		max_hops;
	u8		name[32];
	u16		revisionLevel;
	u8		mstiInstanceNum;
	u16		mstid[CIST_MSTI_NUM];
	u8		bridge_prio[CIST_MSTI_NUM];
	u8		port_prio[CIST_MSTI_NUM][28];
	u32		port_rpc[CIST_MSTI_NUM][28];

	unsigned char	mstConfigTable[4096*2];
	//struct msti_vlan_range msti_range[CIST_MSTI_NUM];
};

#endif
