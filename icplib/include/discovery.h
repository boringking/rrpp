/*
 *	Ethernet Protocol App
 *	Auto Discovery App; discovery.h
 *	
 *	IC+ Corp.
 *
 *	Authors:
 *	Larry Chiu
 *
 */
#include "types.h"

/*******************************************************************************
*	Defines
*******************************************************************************/
#define IC_PLUS_ETH_TYPE	0x8931
#define IC_PLUS_SW_SUBTYPE	0x1002

#define IC_PULS_OP_SEARCH_DEVICE_REQ	0x0001
#define IC_PULS_OP_SEARCH_DEVICE_ACK	0x0002

#define ETH_ALEN	6
#define	ETH_P_8021Q_S_TAG	0x88A8
#define	ETH_P_8021Q_C_TAG	0x8100

#define GET_U16_FROM_FRAME(ptr)		((((*((char *)(ptr)))&0xff)<<8) | ((*(((char*)(ptr))+1))&0xff))

/*******************************************************************************
*	Structures
*******************************************************************************/
#pragma pack(push, 1)
struct discovery_packet_ack
{
	u8					da[ETH_ALEN];
	u8					sa[ETH_ALEN];
	u16					eth_type;
	u16					sub_type;
	u8					op;
	u16					device_id;
	u8					mac[ETH_ALEN];
	u32					ip;
	u32					ip_mask;
	u32					ip_gateway;
};
#pragma pack(pop)


