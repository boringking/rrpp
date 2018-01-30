#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "sys/un.h"
#include "unistd.h"
#include "libcfg.h"
#include "ip1829.h"
#include "ip1829ds.h"
#include "ip1829op.h"
#include "ip1829lib.h"
#include "libnetapp.h"
#include "list.h"

//#define DB_JOE	1
#define JOIN_TIME_EXPIRE		2		//s
#define LEAVE_TIME_EXPIRE		6		//s
#define HOLD_TIME_EXPIRE		1		//s
#define LEAVEALL_TIME_EXPIRE	20		//s
#define GVRP_DEFAULT_CONFIG "gvrp.config"
#define VLAN_CONFIG	"vlan.config"
#define GVRP_CONFIG "gvrp.current"
#define STATIC	0
#define DYNAMIC	1

extern unsigned char applicant_array[11][12];
extern unsigned char registrar_array[3][13];
extern void icp_netb2pro(struct pro_buff *pb, struct netapp_netb *netb);
extern int send_packet(void *buf, int len);

typedef enum{
	VA, VP, VO, AA, AP, AO, QA, QP, QO, LA, LO
}applicant_event;

typedef enum{
	IN, LV, MT
}registrar_event;

typedef enum{
	tx_joinempty, tx_joinin, tx_leaveEmpty, tx_empty,
	rx_joinin, rx_joinempty, rx_empty, rx_leavein, rx_leaveEmpty,
	leaveall, reqjoin, reqleave, leaveall_time_expired, Gid_null
}Gid_event;

typedef struct SGid_machine{
	unsigned short vid;
	unsigned char state;	//1:Static	0:Dynamic
	unsigned char leave_time_on;
	unsigned long leave_time;
	unsigned char applicant;	//Machine_event 
	unsigned char registrar;
	unsigned char frbden;	//1:This vid in this port is forbidden.
	unsigned char fword;	//1:This vid in this port can't forward msg to another port.
	struct list_head list;
}Gid_machine;

typedef struct SGid{
	int port;
	unsigned long join_time;		//always count for send join request
	unsigned char hold_time_on;
	unsigned long hold_time;
	unsigned long leaveAll_time;	//always count for send leaveall request
	struct list_head Gid_machine_head;
	struct list_head list;
}Gid;

//----------------------
void Gid_init(Gid *gid, unsigned char port);
char gid_find_port(unsigned char port, Gid **gid);
char gid_create_port(unsigned char port);
char gid_destroy_port(unsigned char port);
void delete_config_info(Gid_machine* machine, unsigned char port);
Gid_event gid_status(Gid_machine *machine);
