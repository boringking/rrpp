/*
 *	Ethernet Protocol App
 *	Multiple Spanning Tree Protocol; icp_stp.h
 *	
 *	IC+ Corp.
 *
 *	Authors:
 *	Larry Chiu
 *
 */
#ifndef _ICP_STP_H_
#define _ICP_STP_H_

#include "list.h"
#include "types.h"
#include "icp_timer.h"

/*******************************************************************************
*	Defines
*******************************************************************************/
#define TIME_UNIT		0x0100
#define MSTI_MAX_NUM 	15
#define MIGRATE_TIME 	0x0300
#define TXHOLDCOUNT		6
#define	ETH_P_8021Q_S_TAG	0x88A8
#define	ETH_P_8021Q_C_TAG	0x8100
#define GET_U16_FROM_FRAME(ptr)		((((*((char *)(ptr)))&0xff)<<8) | ((*(((char*)(ptr))+1))&0xff))

/*******************************************************************************
*	Structures
*******************************************************************************/
typedef u16 port_id;

typedef struct _bridge_id
{
	unsigned char	prio[2];
	unsigned char	addr[6];
}bridge_id;

typedef enum{
	Invalid,
	Config_Bpdu,
	TCN_Bpdu,
	RST_Bpdu,
	MST_Bpdu
}bpdu_type;

#pragma pack(push, 1)

typedef struct _mst_config_id
{
	u8	formatSelector;
	u8	name[32];
	u16	revisionLevel;
	u8	configDigest[16];
}mst_config_id;

struct logic_link_control
{
	u8			dsap;			// 0x42 for Spanning Tree BPDU
	u8			ssap;			// 0x42 for Spanning Tree BPDU
	u8			ctrl;			// 0x03 unnumbered frame
};

typedef struct _mstp_msti_config
{
	u8			flags;
	bridge_id	rRootBridgeId;
	u32			intRootPathCost;
	u8			bridgePrio;
	u8			portPrio;
	u8			remainingHops;
}mstp_msti_config,*p_mstp_msti_config;

typedef struct _stp_bpdu
{
	u16							len;
	struct logic_link_control	llc;

	u16				protocolId;
	u8				protocolVer;
	u8				bpduType;
	u8				flags;
	bridge_id		rootBridgeId;
	u32				extRootPathCost;
	bridge_id		rRootBridgeId;
	port_id			portId;

	u16 			messageAge;
	u16 			maxAge;
	u16 			helloTime;
	u16 			forwardDelay;

	// rstp/mstp only
	u8				v1_len;
	// mstp only
	u16 			v3_len;
	mst_config_id	mstConfigId;

	u32				intRootPathCost;
	bridge_id		bridgeId;
	u8				remainingHops;
}stp_bpdu,*p_stp_bpdu;

typedef struct _stp_cist_priority_vector
{
	bridge_id	rootBridgeId;			// CIST Root Identifier
	u32			extRootPathCost;		// External Root Path Cost
	bridge_id	rRootBridgeId;			// Regional Root Identifier
	u32			intRootPathCost;		// Internal Root Path Cost
	bridge_id	designatedBridgeId;		// Designated Bridge Identifier
	port_id		designatedPortId;		// Designated Port Identifier
}stp_cist_priority_vector;

typedef struct _stp_msti_priority_vector
{
	bridge_id	rRootBridgeId;
	u32			intRootPathCost;
	bridge_id	designatedBridgeId;
	port_id		designatedPortId;
}stp_msti_priority_vector;

typedef struct _stp_cist_times
{
	u16 message_age;
	u16 max_age;
	u16 hello_time;
	u16 forward_delay;
	u8 remaining_hops;
}stp_cist_times;

#pragma pack(pop)

typedef struct _stp_bridge_msti_common
{
	struct list_head			br_msti_entry;
	struct list_head			br_port_msti_list;

	u16 instance;
	u16 mstid;

	u8							bridgeIdentifier[2];
	stp_msti_priority_vector	bridgePrio;
	u8							bridgeMaxHops;
	port_id						rootPortId;
	stp_msti_priority_vector	rootPrio;
	u8							rootRemainingHops;
}stp_bridge_msti_common;

typedef struct _stp_port_msti_common
{
	struct list_head			port_msti_entry;
	struct list_head			br_port_msti_entry;
	stp_bridge_msti_common		*br_msti;
	struct pro_bridge *br;
	struct pro_port *p;

	u8 rcvdMsg:1;

	u8 agree:1;
	u8 disputed:1;
	u8 forward:1;
	u8 forwarding:1;
	u8 infoIs:2;
	u8 learn:1;
	u8 learning:1;
	u8 proposed:1;
	u8 proposing:1;

	//u8 rcvdTc:1;
	u8 reRoot:1;
	//u8 reselect:1;
	//u8 selected:1;
	u8 tcProp:1;
	u8 agreed:1;
	u8 sync:1;
	u8 synced:1;
	u8 updtInfo:1;

	u8 master:1;
	u8 mastered:1;

	//u8 rcvdInfo;
	u8 role;
	u8 selectedRole;
	u8 state;

	u32							path_cost;
	port_id						port_id;
	stp_msti_priority_vector	designatedPrio;
	u8							designated_remaining_hops;
	u8							msgFlags;
	stp_msti_priority_vector	msgPrio;
	u8							msg_remaining_hops;
	stp_msti_priority_vector	portPrio;
	u8							port_remaining_hops;

	icp_timer				fdWhile;
	icp_timer				rrWhile;
	icp_timer				rbWhile;
	icp_timer				tcWhile;
	icp_timer				rcvdInfoWhile;
}stp_port_msti_common;

enum{Master_Port,Alter_Back_Port,Root_Port,Designated_Port};
enum{DisabledPort,MasterPort,AlternatePort,BackupPort,RootPort,DesignatedPort};
enum{InfoIs_Mine,InfoIs_Aged,InfoIs_Received,InfoIs_Disabled};
enum{Port_State_Disabled,Port_State_Blocking,Port_State_Listening,Port_State_Learning,Port_State_Forwarding};

#endif
