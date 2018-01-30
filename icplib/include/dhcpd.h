#ifndef _DHCPD_H_
#define _DHCPD_H_
/******************************************************************************
 *
 *   Name:           dhcpd.h
 *
 *   Description:
 *
 *   Copyright:      (c) 2013-2050    IC Plus Corp.
 *                   All rights reserved.  By Chance
 *   Version: 1.0.0.20131030
 *******************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/time.h>//for struct timeval

#include "list.h"
//config api use
#include "libcfg.h"
//netapp api use
#include "libnetapp.h"
#include "libgmac.h"
//syslog api use
#include "libfacility.h"
//switch api use
#include "ip1829.h"
#include "ip1829ds.h"
#include "ip1829op.h"
#include "ip1829lib.h"
#include "libcommon.h"

#include "dhcp6.h"
#include "cc_dhcpd.h"


//define
#define DEBUG_TIME	0
#define VLAN_CONFIG			"vlan.config"
#define RELAYAGENT_CONFIG	"relayagent.config"
#define SNOOPING_CONFIG		"imp_table.config"
#define MAX_FILE_PATH 128
#define REMOTE_ID_LEN 6
#define NUM_OF_SERVER 5
#define IPADDRESS_LEN 4
#define DHCP_FILTER_LEN 4
#define NUM_OF_MULTI_RELAY_INFO 10
#define MAX_PACKET_LEN 1520

#define DHCP_SERVER_SOCK_FILE	"/tmp/dhcp_server_socket"
#define DHCP_DEAMON_SOCK_FILE	"/tmp/dhcp_daemon_socket"
#define	MAX_SOCK_SIZE	(2000)

#define REG_DHCP	1
#define REG_ARP		2
//debug log use
//#define DEBUG
#ifdef DEBUG
#define RELAY_DBG(lv, fmt, ...) printf("<%s:%04d> " fmt "\n", __FILE__,__LINE__, ##__VA_ARGS__)
#define DHCPD_DBG(lv,fmt,...) \
	do{\
		if(logflag){\
			syslog(lv,"<%s:%d>%s() " fmt,  __FILE__,__LINE__,__func__, ##__VA_ARGS__);\
		}\
		else\
			printf("<%s:%d>%s() " fmt "\n",  __FILE__,__LINE__,__func__, ##__VA_ARGS__);\
		usleep(1);\
	}while(0)
#else
#define RELAY_DBG(lv, fmt, ...)
#define DHCPD_DBG(...)
#endif

#define	DHCPD_ERR(fmt,...)	syslog(LOG_ERR, "<%s:%d>%s() " fmt,  __FILE__,__LINE__,__func__, ##__VA_ARGS__)

#if DEBUG_TIME
#define SZBUF_LEN	1000
extern char *time_prestr;
extern char szBuffer[SZBUF_LEN];
extern struct timeval cur, last, delta;
#define print_delta_timer()	\
{\
	gettimeofday(&cur, NULL);\
	timersub((struct timeval *)&cur, (struct timeval *)&last, (struct timeval *)&delta);\
	memcpy(&last, &cur, sizeof(struct timeval));\
	memset(&szBuffer, 0, SZBUF_LEN);\
	sprintf(szBuffer,"%stime(%s:%d):%ld.%ld(+%ld.%ld)\n", time_prestr, __func__, __LINE__, cur.tv_sec, cur.tv_usec, delta.tv_sec, delta.tv_usec);\
	syslog(LOG_DEBUG,szBuffer);\
}
#endif
enum {none,add,rem};
enum {broadcast,unicast};
enum vlanbase {portbase,tagbase};
enum {RELAYv4,RELAYv6,BOTH};
enum {client,server};
enum op82polic {drop,relpace,keep,Append};

typedef struct _CIRCUITID_{
	u_int8_t  subtype;//1
	u_int8_t  sublen;//6
	u_int8_t  idtype;//0
	u_int8_t  idlen;//4
	u_int16_t vlandid;
	u_int8_t  module;//always 0
	u_int8_t  port;
}m_CIRCUITID;
typedef struct _REMOTEID_{
	u_int8_t  subtype;//2
	u_int8_t  sublen;//8
	u_int8_t  idtype;//0
	u_int8_t  idlen;//6
	u_int8_t  module[6];//mac address
}m_REMOTEID;
typedef struct _option82_{
	u_int8_t optype;
	u_int8_t oplen;
	m_CIRCUITID circuitid;
	m_REMOTEID  remoteid;
}m_option82;
typedef struct OPTION82INFO{
	char state;
	char policy;
	//	char remoteid[REMOTE_ID_LEN];
}m_OPTION82INFO,*p_OPTION82INFO;
extern m_option82 info_82;

typedef struct SERVERINFO{
	u_int8_t ip[IPADDRESS_LEN];
	u_int8_t state;
	u_int8_t port[16]; //fid max number
	u_int8_t maxport;
	u_int8_t mac[REMOTE_ID_LEN];
}m_SERVERINFO,*p_SERVERINFO;
typedef struct SERVER6INFO{
	u_int8_t ipv6[IPV6_ADDRESS_LEN];
	u_int8_t state;
	u_int8_t port[16]; //fid max number
	u_int8_t maxport;
	u_int8_t mac[REMOTE_ID_LEN];
	
	// new machine
	u_int8_t link_layer_state; // 0:not-find 1:have found
	u_int8_t period;
	u_int8_t expires;
	u_int8_t count;
	u_int8_t source_port;
	
}m_SERVER6INFO,*p_SERVER6INFO;
typedef struct RELAYINFO{
	char state;
	char changed;
	char hops_limit;
	m_OPTION82INFO option82;
	m_SERVERINFO serverip[NUM_OF_SERVER];
	m_SERVER6INFO serveripv6[NUM_OF_SERVER];
	//char serverip[NUM_OF_SERVER][IPADDRESS_LEN];
}m_RELAYINFO,*p_RELAYINFO;

typedef struct DHCPDINFO{
	char state;
	char changed;
	char arpstate;
	char arpchanged;
}m_DHCPDINFO, *p_DHCPDINFO;

typedef struct SNOOPINFO{
	char state;
	char changed;
	char arpstate;
	char arpchanged;
}m_SNOOPINFO, *p_SNOOPINFO;

typedef struct DHCPSERVERINFO{
	char state;
	char changed;
	char bValidIP;
	struct in_addr stIP;
}m_DHCPSERVERINFO, *p_DHCPSERVERINFO;

//prototype
void dhcpd_process();
int set_switch_reg(char );
int register_DHCP_to_netapp(char flag);
int unregister_DHCP_from_netapp(char flag);
int dhcpd4_process(struct pro_buff *pb_ptr);
int dhcpd6_process(struct pro_buff *pb_ptr);
int checkipversion(unsigned char* ptr);
//u_int8_t* find_option(u_int8_t opcode,u_int8_t* inputp);
u_int8_t GET_PORT_ID(struct pro_buff *pb_ptr);
//u_int16_t GET_VLAN_ID(struct pro_buff *pb_ptr);
//int caculate_ipudp_checksum(struct pro_buff *pb_ptr);
u_int8_t* GET_MAC_FROM_FILE(FILE* file,u_int8_t* targetip,u_int8_t* macaddr);
//int getserverinfo();20140120 remove
//int getserverinfo(u_int8_t vlanbase,u_int16_t vid,u_int32_t vlanmem); //20140120 modify by chance for vlan arp
int getserverinfo(struct pro_buff *pb_ptr); //20140121 modify by chance for vlan arp
int getgatewayinfo(struct pro_buff *pb_ptr); //20160407 modify by flaze
u_int8_t GET_PORT_BY_MAC(u_int8_t* macptr,u_int8_t* port);
u_int8_t GET_LINK_STATUS(u_int8_t port);
u_int8_t Check_tagbase();
u_int8_t GET_MAX_PORT_NUM();
u_int32_t GET_VLAN_MEMBER_BY_PORT(u_int8_t port);
//global parameter
extern char config_file[MAX_FILE_PATH];

//register and unregister netapp use
extern int netapp_fd;
extern netapp_ifreq_qHandle	dhcpd_qHandle;
extern netapp_ifreq_fHandle  dhcpd_ipv4_dst_fHandle;
extern netapp_ifreq_fHandle  dhcpd_ipv4_src_fHandle;
extern netapp_ifreq_fHandle  dhcpd_ipv6_dst_fHandle;
extern netapp_ifreq_fHandle  dhcpd_ipv6_src_fHandle;
extern netapp_ifreq_fHandle  dhcpd_ipv6_neighbor_adver_fHandle;
extern netapp_ifreq_fHandle  arp_fHandle;

extern char dhcpd_version;
extern m_SERVERINFO gatewayinfo;
extern m_SERVER6INFO gateway6info;
extern m_RELAYINFO relay_current_info;
extern m_DHCPDINFO dhcpd_info;
extern m_SNOOPINFO snooping_info;
extern m_DHCPSERVERINFO dhcp_server_info;


extern u_int8_t sendbuff[MAX_PACKET_LEN];
extern int logflag;
extern struct netapp_netb netb;
extern struct pro_buff pb;
extern char logid[LOGID_SIZE];

#define DHCP_MAGIC_COOKIE_LEN 4
#define IP_HEADER_LEN         20
#define UDP_HEADER_LEN        8
#define DHCP_UDP_OVERHEAD	(IP_HEADER_LEN + /* IP header */			\
		UDP_HEADER_LEN)   /* UDP header */
#define DHCP_SNAME_LEN		64
#define DHCP_FILE_LEN		128
#define DHCP_FIXED_NON_UDP	236
#define DHCP_FIXED_LEN		(DHCP_FIXED_NON_UDP + DHCP_UDP_OVERHEAD)
/* Everything but options. */
#define BOOTP_MIN_LEN		300

#define DHCP_MTU_MAX		1500
#define DHCP_MTU_MIN		576

#define DHCP_MAX_OPTION_LEN	(DHCP_MTU_MAX - DHCP_FIXED_LEN)
#define DHCP_MIN_OPTION_LEN     (DHCP_MTU_MIN - DHCP_FIXED_LEN)

typedef struct dhcp_packet {
	u_int8_t  op;		/* 0: Message opcode/type */
	u_int8_t  htype;	/* 1: Hardware addr type (net/if_types.h) */
	u_int8_t  hlen;		/* 2: Hardware addr length */
	u_int8_t  hops;		/* 3: Number of relay agent hops from client */
	u_int32_t xid;		/* 4: Transaction ID */
	u_int16_t secs;		/* 8: Seconds since client started looking */
	u_int16_t flags;	/* 10: Flag bits */
	u_int32_t ciaddr;	/* 12: Client IP address (if already in use) */
	u_int32_t yiaddr;	/* 16: Client IP address */
	u_int32_t siaddr;	/* 20: IP address of next server to talk to */
	u_int32_t giaddr;	/* 24: DHCP relay agent IP address */
	unsigned char chaddr [16];	/* 28: Client hardware address */
	char sname [DHCP_SNAME_LEN];	/* 44: Server name */
	char file [DHCP_FILE_LEN];	/* 108: Boot filename */
	unsigned char options [DHCP_MAX_OPTION_LEN];
	/* 236: Optional parameters
	   (actual length dependent on MTU). */
}m_dhcp_packet,*p_dhcp_packet;

/* DHCP message types */
#define	BOOTREQUEST	1
#define BOOTREPLY	2

/*flags broadcast*/
#define DHCP_BROADCAST 32768L

/* DHCP message types */
#define DHCPDISCOVER		1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNAK			6
#define DHCPRELEASE		7
#define DHCPINFORM		8
#define DHCPLEASEQUERY		10
#define DHCPLEASEUNASSIGNED	11
#define DHCPLEASEUNKNOWN	12
#define DHCPLEASEACTIVE		13

/* DHCP Option */
#define DHCPO_PAD					0
#define DHCPO_SUBNET_MASK				1
#define DHCPO_TIME_OFFSET				2
#define DHCPO_ROUTERS				3
#define DHCPO_TIME_SERVERS			4
#define DHCPO_NAME_SERVERS			5
#define DHCPO_DOMAIN_NAME_SERVERS			6
#define DHCPO_LOG_SERVERS				7
#define DHCPO_COOKIE_SERVERS			8
#define DHCPO_LPR_SERVERS				9
#define DHCPO_IMPRESS_SERVERS			10
#define DHCPO_RESOURCE_LOCATION_SERVERS		11
#define DHCPO_HOST_NAME				12
#define DHCPO_BOOT_SIZE				13
#define DHCPO_MERIT_DUMP				14
#define DHCPO_DOMAIN_NAME				15
#define DHCPO_SWAP_SERVER				16
#define DHCPO_ROOT_PATH				17
#define DHCPO_EXTENSIONS_PATH			18
#define DHCPO_IP_FORWARDING			19
#define DHCPO_NON_LOCAL_SOURCE_ROUTING		20
#define DHCPO_POLICY_FILTER			21
#define DHCPO_MAX_DGRAM_REASSEMBLY		22
#define DHCPO_DEFAULT_IP_TTL			23
#define DHCPO_PATH_MTU_AGING_TIMEOUT		24
#define DHCPO_PATH_MTU_PLATEAU_TABLE		25
#define DHCPO_INTERFACE_MTU			26
#define DHCPO_ALL_SUBNETS_LOCAL			27
#define DHCPO_BROADCAST_ADDRESS			28
#define DHCPO_PERFORM_MASK_DISCOVERY		29
#define DHCPO_MASK_SUPPLIER			30
#define DHCPO_ROUTER_DISCOVERY			31
#define DHCPO_ROUTER_SOLICITATION_ADDRESS		32
#define DHCPO_STATIC_ROUTES			33
#define DHCPO_TRAILER_ENCAPSULATION		34
#define DHCPO_ARP_CACHE_TIMEOUT			35
#define DHCPO_IEEE802_3_ENCAPSULATION		36
#define DHCPO_DEFAULT_TCP_TTL			37
#define DHCPO_TCP_KEEPALIVE_INTERVAL		38
#define DHCPO_TCP_KEEPALIVE_GARBAGE		39
#define DHCPO_NIS_DOMAIN				40
#define DHCPO_NIS_SERVERS				41
#define DHCPO_NTP_SERVERS				42
#define DHCPO_VENDOR_ENCAPSULATED_OPTIONS		43
#define DHCPO_NETBIOS_NAME_SERVERS		44
#define DHCPO_NETBIOS_DD_SERVER			45
#define DHCPO_NETBIOS_NODE_TYPE			46
#define DHCPO_NETBIOS_SCOPE			47
#define DHCPO_FONT_SERVERS			48
#define DHCPO_X_DISPLAY_MANAGER			49
#define DHCPO_DHCP_REQUESTED_ADDRESS		50
#define DHCPO_DHCP_LEASE_TIME			51
#define DHCPO_DHCP_OPTION_OVERLOAD		52
#define DHCPO_DHCP_MESSAGE_TYPE			53
#define DHCPO_DHCP_SERVER_IDENTIFIER		54
#define DHCPO_DHCP_PARAMETER_REQUEST_LIST		55
#define DHCPO_DHCP_MESSAGE			56
#define DHCPO_DHCP_MAX_MESSAGE_SIZE		57
#define DHCPO_DHCP_RENEWAL_TIME			58
#define DHCPO_DHCP_REBINDING_TIME			59
#define DHCPO_VENDOR_CLASS_IDENTIFIER		60
#define DHCPO_DHCP_CLIENT_IDENTIFIER		61
#define DHCPO_NWIP_DOMAIN_NAME			62
#define DHCPO_NWIP_SUBOPTIONS			63
#define DHCPO_USER_CLASS				77
#define DHCPO_FQDN				81
#define DHCPO_DHCP_AGENT_OPTIONS			82
#define DHCPO_AUTHENTICATE			90  /* RFC3118, was 210 */
#define DHCPO_CLIENT_LAST_TRANSACTION_TIME	91
#define DHCPO_ASSOCIATED_IP			92
#define DHCPO_SUBNET_SELECTION			118 /* RFC3011! */
#define DHCPO_DOMAIN_SEARCH			119 /* RFC3397 */
#define DHCPO_VIVCO_SUBOPTIONS			124
#define DHCPO_VIVSO_SUBOPTIONS			125

#define DHCPO_END					255

#define IP_PROTO_UDP   17
/**
 * The Ethernet header.
 */
typedef struct {
	u_int8_t	dest[REMOTE_ID_LEN]; //length 6
	u_int8_t	src[REMOTE_ID_LEN];  //length 6
	u_int16_t	type;
}m_eth_hdr;
/* The UDP and IP headers. */
typedef struct {
	//ethernet header
	//m_eth_hdr ethernet_header;
	/* IP header. */
	u_int8_t vhl,
			 tos,
			 len[2],
			 ipid[2],
			 ipoffset[2],
			 ttl,
			 proto;
	u_int16_t ipchksum;
	u_int16_t srcipaddr[2],
			  destipaddr[2];

	/* UDP header. */
	u_int16_t srcport,
			  destport;
	u_int16_t udplen;
	u_int16_t udpchksum;
	u_int8_t udpload[1];//udp load data
} udpip_hdr;

/*The ARP body*/
typedef struct Sarp_msg{
	u_int16_t hw_type;
	u_int16_t protocol;
	u_int8_t hw_len;
	u_int8_t protocol_len;
	u_int16_t opcode;
	u_int8_t senderMAC[6];
	u_int8_t senderIP[4];
	u_int8_t targetMAC[6];
	u_int8_t targetIP[4];
}arp_msg, *parp_msg;

#define IPV4_VER			4
#define IPV6_VER			6
#define TYPE_ARP			1
#define IP_ETHTYPE_IP  0x0800
#define IP_ETHTYPE_IP6 0x86dd
#define ARP_ETHTYPE	   0x0806

#define IPV4_DHCP_SRC_PORT  68
#define IPV4_DHCP_DEST_PORT  67

#define IPV6_DHCP_CLIENT_PORT  546
#define IPV6_DHCP_SERVER_PORT  547

#define GET_U16_FROM_FRAME(ptr)     ((((*((char *)(ptr)))&0xff)<<8) | ((*(((char*)(ptr))+1))&0xff))
/*
 * RFC-4861
 * IPv6 Neighbor Discovery Defines
 */
#define ICMPV6_NEIGHBOR_ADVERTISEMENT		136
// Option Type
//#define ND_OT_TARGET_LINK_LAYER_ADDRESS	1
#define ND_OT_TARGET_LINK_LAYER_ADDRESS	2
//#define ND_OT_PREFIX_INFORMATION		3
//#define ND_OT_REDIRECTED_HEADER			4
//#define ND_OT_MTU						5
#define ETH_P_8021Q_S_TAG   0x88A8
#define ETH_P_8021Q_C_TAG   0x8100
#define SIZE_OF_IPV6_HEADER	40
#define SIZE_OF_NEIGHBOR_ADVERTISEMENT_HEADER	24
#define OFFSET_OF_IPV6_NEXT_HEADER	6
#define IP_PROTOCOL_ICMPV6	0x3A

typedef struct _optlink{
	u_int8_t optype;
	u_int8_t oplen;
	u_int8_t* data;
	//struct __optlink* next;
	struct list_head list;
}m_optlink;
typedef struct _CTAG{
	u_int8_t state;
	u_int16_t type;
	u_int8_t len;
	u_int16_t value;
}m_CTAG;
typedef struct _STAG{
	u_int8_t state;
	u_int16_t type;
	u_int8_t len;
	u_int32_t value;
}m_STAG;
typedef struct _RELAY_SEND_DATA{
	m_eth_hdr			ethernet_header;
	udpip_hdr relay4_send_header;
	m_dhcp_packet relay4_send_dhcp_header;
	m_optlink opt_hdr;
	m_CTAG ctaginfo;
	m_STAG staginfo;
}m_RELAY_SEND_DATA;
//for ipv6
typedef struct _RELAY6_SEND_DATA{
	m_eth_hdr				ethernet_header;
	m_IPV6_HEADER			ipv6_header;
	m_UDP_HDR				udp_header;
	m_dhcpv6_relay_packet	relay6_hdr;
	m_dhcpv6_op_ifaceid		relay6_iface;
	m_dhcpv6_relaymsg		relay6_msg;
	u_int8_t	relayflag;
	m_opt6link	opt_hdr;
	m_CTAG		ctaginfo;
	m_STAG		staginfo;
}m_RELAY6_SEND_DATA;

typedef struct _UDP_IPV6_HDR{
	m_eth_hdr		ethernet_header;
	m_IPV6_HEADER	ipv6_hdr;
	m_UDP_HDR		udp_hdr;
}m_UDP_IPV6_HDR;

typedef struct _UDP_IPV6_LOAD{
	m_IPV6_HEADER	ipv6_hdr;
	u_int8_t		udp_hdr[1];
}m_UDP_IPV6_LOAD;

typedef struct _MULTIR_ELAY6_INFO{
	u_int8_t	state;
	u_int8_t	ipv6[IPV6_ADDRESS_LEN];	//length 16
	u_int8_t	mac[REMOTE_ID_LEN];		//length 6
}m_MULTI_RELAY6_INFO;


extern m_RELAY_SEND_DATA relay4_send_data;
extern udpip_hdr relay4_send_header;
extern m_dhcp_packet relay4_send_dhcp_header;
extern m_RELAY6_SEND_DATA relay6_send_data;
extern m_MULTI_RELAY6_INFO multi_relay6_info[NUM_OF_MULTI_RELAY_INFO];

int generate_send_packet(u_int8_t seleted_port);
int send_packet(void *buf, int len);
//u_int8_t IS_INSERT_VLAN_TAG();
u_int16_t udp_sum_calc(u_int16_t len_udp, unsigned char src_addr[],unsigned char dest_addr[],unsigned char iplen, unsigned char padding, unsigned char buff[]);
u_int16_t ip_sum_calc(u_int16_t len_ip_header, unsigned char *buff);
u_int8_t dhcp_send_packet_init(u_int8_t* dataptr,u_int8_t gowere);
u_int32_t GET_VLAN_MEMBER_BY_VID(u_int16_t vid);
u_int16_t GET_VLAN_PVID(u_int8_t port);
u_int8_t* GET_SWITCH_IPv6();
u_int8_t dhcpv6_send_packet_init(u_int8_t* dataptr,u_int8_t gowere);
int generate_send6_packet(u_int8_t seleted_port);
u_int8_t GET_SWITCH_NETMASKv6(u_int8_t* inbuffer);
int ipv6_subnet_cmp(u_int8_t* firstip, u_int8_t* secondip);
u_int8_t GET_MAC_FROM_FILE6(FILE* file,u_int8_t* ipv6ptr,u_int8_t* macaddr);
u_int32_t str2map(char *buf);
u_int8_t dhcpd_parse_config_file();
int getaddrfamily(char *addr);
u_int16_t checkvlan(unsigned char* ptr);
u_int8_t GET_SWITCH_IPv6_prefix();
u_int8_t send_arp_packet(u_int16_t vid,u_int8_t* targetip,u_int8_t target_port);
u_int8_t Query_VLAN_TAG_MODE(u_int8_t target_port,u_int16_t vid);//target_port start with 1
u_int8_t tagging_process(struct pro_buff *pb_ptr,u_int8_t target_port,u_int8_t ipv);
u_int8_t send_neighbor_solicitation(u_int16_t vid,u_int8_t* targetip,u_int8_t target_port);

u_int8_t* GET_CLIENT6_MAC();
void option82_init();
u_int8_t parse_option(u_int8_t* ptr_opt);
m_optlink* find_option(u_int8_t delopt);
u_int8_t add_option(m_optlink * addopt);
u_int8_t delete_option(u_int8_t delopt);
u_int8_t free_option();
u_int16_t generate_dhcp_options(u_int8_t* bufptr);

int find_enpty_multi_relay_info();
int find_multi_relay_info(u_int8_t *ipv6_ptr);

u_int8_t parse_option6(u_int8_t* ptr_opt,u_int16_t optlen);
u_int8_t free_option6();

u_int8_t relay6_server_foward(struct pro_buff *pb_ptr);
u_int8_t relay6_client_foward(struct pro_buff *pb_ptr);
u_int8_t relay6_server_process(struct pro_buff *pb_ptr);
u_int8_t* strip_relay_message(u_int8_t* ptr,u_int8_t single);
int relay6_client_process(struct pro_buff *pb_ptr);
int relay6_process(struct pro_buff *pb_ptr);

void *timer_process(void *arg);

u_int8_t relay4_client_foward(struct pro_buff *pb_ptr);
u_int8_t relay4_server_foward(struct pro_buff *pb_ptr,u_int8_t flag);
int relay4_client_process(struct pro_buff *pb_ptr);
int relay4_server_process(struct pro_buff *pb_ptr);
int relay4_process(struct pro_buff *pb_ptr,	m_dhcp_packet* ptr_dhcp);

void snoop_init(char flag);
void update_snoop_table();
char snooping_ipv4_process(m_dhcp_packet* ptr_dhcp, int iport, unsigned char* srcMAC, unsigned char *dstMAC);
char snooping_ipv6_process(m_UDP_HDR* ptr_udp, int iport, unsigned char* srcMAC, unsigned char *dstMAC);
char snooping_arp4_process(parp_msg arpinfo, int iport, unsigned char* srcMAC, unsigned char *dstMAC);
extern struct list_head snoop_list;
#endif
