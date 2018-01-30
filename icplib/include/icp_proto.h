/*
 *	Ethernet Protocol App; icp_proto.h
 *	
 *	IC+ Corp.
 *
 *	Authors:
 *	Larry Chiu
 *
 */
#ifndef _ICP_PROTO_H_
#define _ICP_PROTO_H_

#include <pthread.h>
#include <time.h>

#include "list.h"
#include "types.h"
#include "icp_timer.h"
#include "icp_module.h"
#include "libnetapp.h"

#ifdef MODULE_STP
#include "icp_stp.h"
#endif
#ifdef MODULE_LACP
#include "icp_lacp.h"
#endif
/*******************************************************************************
*	Defines
*******************************************************************************/
#define ETH_ALEN	6

#define IF_PORT_FLAG_UP			BIT(0)
#define IF_PORT_FLAG_STP		BIT(1)
#define IF_PORT_FLAG_LACP		BIT(2)
#define IF_PORT_FLAG_CHANGE		BIT(3)		// port state changing flag, used in stp and lacp.

/*******************************************************************************
*	Structures
*******************************************************************************/
struct pro_bridge
{
	// common
	struct list_head	port_list;
	struct list_head	timer_list;

	pthread_mutex_t		mutex;
	pthread_t			cc_thread;

	u8					sys_mac[ETH_ALEN];
	
#ifdef MODULE_STP
	// stp
	pthread_t			stp_input_thread;
	u8					stp_thread_status;

	struct list_head	stp_port_list;

	enum {
		BR_NO_STP, 		/* no spanning tree */
		BR_STP,			/* Spaning Tree Protocol */
		BR_RSTP,		/* Rapid Spanning Tree Protocol */
		BR_MSTP,		/* Multiple Spanning Tree Protocol */
	} stp_enabled;

	/* STP */
	u32					root_path_cost;
	u16					topology_count;
	time_t				topology_time;

	/* MSTP */
	mst_config_id				mstConfigId;
	u8							mstiInstanceNum;

	u8							bridgeIdentifier[2];
	stp_cist_priority_vector	bridgePrio;
	stp_cist_times				bridgeTimes;
	port_id						rootPortId;
	stp_cist_priority_vector	rootPrio;
	stp_cist_times				rootTimes;
	
	struct list_head			br_msti_list;

#endif

#ifdef MODULE_LACP
	// lacp
	pthread_t			lacp_input_thread;
	u8					lacp_thread_status;
	
	struct list_head	llg_group_list;

	lacp_system_id 		system_id;
#endif

};// struct pro_bridge

struct pro_port
{
	// common
	struct pro_bridge	*br;
	struct list_head	port_entry;

	pthread_mutex_t		mutex;
	
	int 				flag;
	int 				port_no;
	int					sw_port_no;

#ifdef MODULE_STP
	// stp
	struct list_head	stp_port_entry;

	int					speed;
	/* STP */
	u8					priority;
	u8					state;
	port_id				port_id;
	u32					path_cost;
	u16					forward_transitions;

	
	/* RSTP & MSTP */
	// Single per-port
	u8 txCount;
	// Flags
	u8 autoEdge:1;
	u8 newInfo:1;
	u8 operEdge:1;
	u8 stp_enabled:1;

	u8 sendRSTP:1;
	u8 rcvdSTP:1;
	u8 rcvdRSTP:1;
	u8 rcvdInternal:1;
	//u8 rcvdTcAck:1;
	//u8 rcvdTcn:1;
	u8 tcAck:1;
	u8 newInfoXst:1;

	// one for cist and one for msti
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
	//u8 updtInfo:1;

	// variables
	bpdu_type bpduType;
	//u8 rcvdInfo;
	u8 role;
	u8 selectedRole;
	
	stp_cist_priority_vector	designatedPrio;
	stp_cist_times				designatedTimes;
	u8 							msgFlags;
	stp_cist_priority_vector	msgPrio;
	stp_cist_times				msgTimes;
	stp_cist_priority_vector	portPrio;
	stp_cist_times				portTimes;

	icp_timer 				mdelayWhile;
	icp_timer 				helloWhen;
	icp_timer 				edgeDelayWhile;
	icp_timer 				fdWhile;
	icp_timer 				rrWhile;
	icp_timer 				rbWhile;
	icp_timer 				tcWhile;
	icp_timer 				rcvdInfoWhile;
	
	struct list_head			port_msti_list;
#endif

#ifdef MODULE_LACP
	// lacp
	struct lacp_link_group	*llg;
	struct list_head		llg_port_entry;

	lacp_port_id 		Actor_Port_ID;
	u8 					NTT;
	u16 				Actor_Key;
	u8 					Actor_Port_State;
	/*
	 * Bit encoding of the Actor_State and Partner_State fields
	 *Bit
	 *0--------------1--------------2-------------3-----------------4------------5--------------6-----------7---------*
	 * LACP_Activity | LACP_Timeout | Aggregation | Synchronization | Collecting | Distributing | Defaulted | Expired |
	 *----------------------------------------------------------------------------------------------------------------*
	*/

	lacp_system_id 		Partner_System;
	lacp_port_id 		Partner_Port_ID;
	u16 				Partner_Key;
	u8 					Partner_Port_State;
	//u8 Partner_Defaulted;
	
	icp_timer 		expiredTimer;
	icp_timer 		periodTimer;
	u8 					txNum;
	u8 					lacp_enabled;
	u8 					selected;
	u8					lacp_forward;
	//u8 					groupIndex;
#endif

};// struct pro_port

/*******************************************************************************
*	Function Prototype
*******************************************************************************/
/* icp_proto_main.c */
struct pro_port *br_get_sw_port(int sw_port_no);


#endif
