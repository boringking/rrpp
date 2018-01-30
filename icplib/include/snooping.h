#include "dhcpd.h"
//#include <linux/spinlock.h>

//#define DB_SNOOP	1

#define TYPE_RMV	0
#define TYPE_ADD	1

#define PARSE_TO_NEXT		0
#define COMPLETE_PACKET		1
#define SEND_ARP_REPLY		2
#define SNOOPING_TABLE		"snoop_binding.table"
//DHCP
#define DHCPDISCOVERY	1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNAK			6
#define DHCPRELEASE		7
#define DHCPINFORM		8
//DHCPv6
#define MSG_SOLICIT		1
#define MSG_ADVERTISE	2
#define MSG_REQUEST		3
//#define MSG_CONFIRM		4
#define MSG_RENEW		5
#define MSG_REBIND		6
#define MSG_REPLY		7
#define MSG_RELEASE		8
#define MSG_DECLINE		9
#define MSG_RECONFIGURE	10
#define MSG_INFORMATION_REQUEST	11
#define MSG_RELAY_FORW			12
#define MSG_RELAY_REPL			13

#define OPTION_CLIENT_IDENT		1
#define OPTION_SERVER_IDENT		2
#define OPTION_IA_NA			3
#define OPTION_IA_TA			4
#define OPTION_IA_PD			25

#define MACADDR_LEN	6
#define IPADDR_LEN		16

typedef struct port_info{
	char valid;
	char ip_type;
	unsigned char mac[MACADDR_LEN];
	unsigned char ip[IPADDR_LEN];
	unsigned long leaveTime;
	struct list_head list;
}SportInfo;

typedef struct snoop_info{
	int port;
	int used;
	struct list_head port_list;
	struct list_head list;
}SsnoopInfo;

char check_port_state(int iport, unsigned char *Untrusted, unsigned char *Recover, unsigned char *entry);
unsigned char* get_option_info(unsigned char *dhcp_option, unsigned char option);
unsigned char* get_v6option_info(unsigned char* dhcp_option, long totalLen, unsigned char option);
void set_ip_table(char type, int iport, SportInfo *portInfo);
void update_snoop_table();
void snoop_thread();
