#include "neighbor.h"

#define ETHER_HEADER_LEN		14
#define SEARCHDEVICE_UDP_PORT	18000
#define AUTO_OK					0
#define AUTO_ERROR				-1

#define IP_CONFIG				"ip.config"
#define DHCP_IP					"/usr/share/udhcpc/udhcpc.ip"
#define SYSINFO_CONFIG			"system.config"
#define PORT_STATE_CONFIG		"port_status.conf"
#define POE_CONFIG				"poe.config"
#define POE_CURRENT				"poe.info"
#define POE_OVERLOAD			"poe.overload"
#define MAC_TABLE_CONFIG		"mac_table.info"

//Option non-Login
#define SEARCH_DEVICE_REQ			0x0001
#define SEARCH_DEVICE_ACK			0x0002
#define LOGIN_REQ					0x0003
#define LOGIN_ACK					0x0004
#define NEIGHBOR_MACID_REQ			0x0005
#define NEIGHBOR_MACID_ACK			0x0006
#define LOGOUT_REQ					0x0007
#define LOGOUT_ACK					0x0008

//Option need-Login
#define SET_SWITCH_IP_REQ			0x0100
#define SET_SWITCH_IP_ACK			0x0200
#define GET_SWITCH_IP_REQ			0x0300
#define GET_SWITCH_IP_ACK			0x0400
#define SET_DHCP_STATE_REQ			0x0500
#define SET_DHCP_STATE_ACK			0x0600
#define GET_DHCP_STATE_REQ			0x0700
#define GET_DHCP_STATE_ACK			0x0800
#define GET_SWITCH_VERSION_REQ		0x0900
#define GET_SWITCH_VERSION_ACK		0x0A00
#define RESTART_SWITCH_REQ			0x0B00
#define RESTART_SWITCH_ACK			0x0C00
#define GET_SWITCH_PORT_STATE_REQ	0x0D00
#define GET_SWITCH_PORT_STATE_ACK	0x0E00
#define SET_POE_PORT_STATE_REQ		0x0F00
#define SET_POE_PORT_STATE_ACK		0x1000
#define GET_POE_PORT_STATE_REQ		0x1100
#define GET_POE_PORT_STATE_ACK		0x1200
#define SET_POE_PORT_BUDGET_REQ		0x1300
#define SET_POE_PORT_BUDGET_ACK		0x1400
#define GET_POE_PORT_BUDGET_REQ		0x1500
#define GET_POE_PORT_BUDGET_ACK		0x1600
#define GET_POE_PORT_WATT_REQ		0x1700
#define GET_POE_PORT_WATT_ACK		0x1800
#define SET_POE_TOTAL_WATT_REQ		0x1900
#define SET_POE_TOTAL_WATT_ACK		0x1A00
#define GET_POE_TOTAL_WATT_REQ		0x1B00
#define GET_POE_TOTAL_WATT_ACK		0x1C00
#define RESTART_POE_REQ				0x1D00
#define RESTART_POE_ACK				0x1E00
#define GET_POE_PORT_OVERLOAD_REQ	0x1F00
#define GET_POE_PORT_OVERLOAD_ACK	0x2000
#define GET_MAC_TABLE_REQ			0x2100
#define GET_MAC_TABLE_ACK			0x2200

#pragma pack(push, 1)
typedef struct Spacket_frame{
	//ether head
	u8	da[6];
	u8	sa[6];
	u16 ether_type;
	//ip head
	u8	ver_hlen;
	u8	dif;
	u16 totallen;
	u16 identification;
	u16 offset;
	u8	ttl;
	u8	protocol;
	u16 ipchksum;
	u8	srcip[4];
	u8	destip[4];
	//udp head
	u16 srcport;
	u16 destport;
	u16 len;
	u16 udpchksum;
	u8	udpdata[1];
}packet_frame, *ppacket_frame;

typedef struct Soption_frame{
	u16 op;
	u16 len;
	u8 data[1];
}option_frame, *poption_frame;

typedef struct Soplogin_frame{
	u16 op;
	u32 connectid;
	u16 len;
	u8 data[1];
}oplogin_frame, *poplogin_frame;

typedef struct SNeighborList{
	u8 port;
	u8 number;
	u8 mac[1];
}NeighborList, *pNeighborList;

typedef struct SSearchDeviceAck{
	u16 deviceID;
	u8 mac[6];
	u8 ipaddr[4];
	u8 mask[4];
	u8 gateway[4];
	u8 namelen;
	u8 name[1];
}SearchDeviceAck, *pSearchDeviceAck;
#pragma pack(pop)

u8 netapp_regist();
u8 netapp_cancel();
void ipString2Array(u8 *str, u8 *ary);
u16 ip_chksum(u16 *sdata, u16 len);
u16 udp_chksum();

void init_udpframe(u8 *sa, u8 *srcip, u16 srcport);
u16 udpframe(u16 udpdatalen);
char CheckPOE();

//non-Login Request
u16 LoginReq(ppacket_frame rcvpkt);
u16 LogoutReq(ppacket_frame rcvpkt);
u16 SearchDeviceReq(ppacket_frame rcvpkt);
u16 neighborReq(ppacket_frame rcvpkt);
//Need-Login Request
u16 SetIPReq(ppacket_frame rcvpkt);
u16 SetDHCPReq(ppacket_frame rcvpkt);
u16 SetPOEPortStateReq(ppacket_frame rcvpkt);
u16 SetPOEPortBudgetReq(ppacket_frame rcvpkt);
u16 SetPOETotalWattReq(ppacket_frame rcvpkt);
u16 GetIPReq(ppacket_frame rcvpkt);
u16 GetDHCPReq(ppacket_frame rcvpkt);
u16 GetSwitchPortStateReq(ppacket_frame rcvpkt);
u16 GetPOEPortStateReq(ppacket_frame rcvpkt);
u16 GetPOEPortBudgetReq(ppacket_frame rcvpkt);
u16 GetPOEPortWattReq(ppacket_frame rcvpkt);
u16 GetPOETotalWattReq(ppacket_frame rcvpkt);
u16 GetPOEPortOverload(ppacket_frame rcvpkt);
u16 GetMACTableReq(ppacket_frame rcvpkt);
u16 GetSwitchVersion(ppacket_frame rcvpkt);
u16 RSTSwitchReq(ppacket_frame rcvpkt);
u16 RSTPOEReq(ppacket_frame rcvpkt);
void db_print(char *buf);
