/*
 *	Ethernet Protocol App
 *	Link Aggregation Control Protocol; icp_lacp.h
 *	
 *	IC+ Corp.
 *
 *	Authors:
 *	Larry Chiu
 *
 */
#ifndef _ICP_LACP_H_
#define _ICP_LACP_H_

#include "list.h"
#include "types.h"
#include "icp_timer.h"

/*******************************************************************************
*	Defines
*******************************************************************************/

/*******************************************************************************
*	Structures
*******************************************************************************/
#pragma pack(push, 1)

typedef struct _lacp_system_id
{
	u16		priority;
	u8		macid[6];	
}lacp_system_id,*p_lacp_system_id;

typedef struct _lacp_port_id
{
	u16		priority;
	u16		portid;
}lacp_port_id,*p_lacp_port_id;

typedef struct _linkinfo
{
	u8				TLV_type;
	u8				length;
	lacp_system_id	system_info;
	u16				opKey;
	lacp_port_id	portID;
	u8				state;
	u8				reserved[3];
}linkinfo,*p_linkinfo;

typedef struct _colinfo
{
	u8		TLV_type;
	u8		length;
	u16		maxDelay;
	u8		reserved[12];
}colinfo,*p_colinfo;

typedef struct _lacpdu
{
	u16			type;				// Slow Protocols Type Field:0x8809
	u8			subType;			// 0x01 
	u8			version;
	linkinfo	ActorInfo;			// TLV_type=0x01, Length=0x14
	linkinfo	PartnerInfo;		// TLV_type=0x02, Length=0x14
	colinfo		CollectorInfo;		// TLV_type=0x03, Length=0x10
	u16			terminator;			// TLV_type=0x00, Length=0x00
	u8			reserved[50];
}lacpdu,*p_lacpdu;

#pragma pack(pop)

struct lacp_link_group
{
	struct pro_bridge		*br;
	struct list_head		llg_port_list;
	struct list_head		llg_group_entry;
	
	u8				state;
	u8				group_id;

	u32				groupMemMask;
	u32				groupMem;
	u16				Actor_Key;
	lacp_system_id	Partner_System;
	u16				Partner_Key;
	u8				flag;					// INITIAL/CHANGED/DONE
};

enum {UNSELECTED,SELECTED,STANDBY};
enum {Fast_Periodic_Time=1,Slow_Periodic_Time=30};
enum {INITIAL,CHANGED,DONE};
enum {LACP_EQAL,LACP_SMALL,LACP_BIG};
enum {NO_TRUNK,LINK_STATIC,LINK_LACP};

#endif
