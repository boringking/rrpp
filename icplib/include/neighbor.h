#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/socket.h"
#include "unistd.h"
#include "libnetapp.h"
#include "libcfg.h"
#include "libcgi.h"
#include "libuac.h"
#include "list.h"
#include "types.h"
#include "port_config.h"
#include "ip1829.h"
#include "ip1829ds.h"
#include "ip1829op.h"
#include "ip1829lib.h"
#include "libcommon.h"
#include "errno.h"
#include <signal.h>

#define MAX_PACKET_SIZE			1500
#define MAX_NETCMD_CONNECT		10
#define NETCMD_DEFAULT_AGING	3600
#define NEIGHBOR_REQUEST		0x0001
#define NEIGHBOR_ACK			0x0002

u8 neighbor_init();
u8 neighbor_process(struct pro_buff *pb);
void neighbor_timer();
void neighbor_timeout(int sig);

typedef struct dsneighbor_frame{
	u8 da[6];
	u8 sa[6];
	u16 type;
	u16 len;
	u16 op;
	u16 oplen;
	u8 mac[6];
}neighbor_frame, *pneighbor_frame;

typedef struct dsmac_info{
	u8 again;
	u8 mac[6];
	struct list_head list;
}mac_info, *pmac_info;

typedef struct dsneighbor_info{
	u8 port;
	struct list_head mac_list;
	struct list_head list;
}neighbor_info, *pneighbor_info;

typedef struct Sconnectinfo{
	u8 ip[4];
	u32 connectid;
	u16 agingtime;
}connectinfo, *pconnectinfo;

extern unsigned char pkt_buf[MAX_PACKET_SIZE];
extern void get_switch_mac(u8 *mac);
extern struct list_head neighbor_list;
extern connectinfo loginInfo[MAX_NETCMD_CONNECT]; 
