#include "gid.h"
#include "time.h"

#define INFO_GVRP	1	//for gvrp webpage
#define INFO_PORT	2	//for portsetting webpage
#define INFO_VLAN	3	//for vlan webpage
#define INFO_NULL	0

#define LEAVEALL	0
#define JOINEMPTY	1
#define JOININ		2
#define LEAVEEMPTY	3
#define LEAVEIN		4
#define EMPTY		5

#define OPTION_ENABLE	0
#define OPTION_DISABLE	1
#define OPTION_JOIN		2
#define OPTION_LEAVE	3
#define OPTION_LEAVEALL	4

#define GVRP_OK		0
#define GVRP_ERROR	-1

#define ADD_VID		0
#define RMV_VID		1

#define MAX_PACKET_SIZE	1500

extern struct list_head Gid_head;
extern char tmpbuf[100];
extern void db_print(char *buf);
unsigned int str2map(char *buf);
char *map2str(unsigned int map);

extern struct list_head Vid_head;
extern unsigned long gvrp_join_time;
extern unsigned long gvrp_leave_time;
extern unsigned long gvrp_leaveall_time;

void init_list();

void gvrp_main(unsigned long gdata, unsigned char flag);
void gvrp_port(unsigned char port, unsigned char flag);
void gvrp_vlan(unsigned short vid, unsigned char flag);
void gvrp_join_timeout(Gid *gid);
void gvrp_leaveAll_timeout(Gid *gid);
void gvrp_send(int port, unsigned int len);

void gvrp_thread();
void time_thread();
char netapp_regist();
char netapp_cancel();

void rcv_leaveall(Gid *gid);
void rcv_join(Gid *gid, unsigned short vid, unsigned char msg);
void rcv_leave(Gid *gid, unsigned short vid, unsigned char msg);
void rcv_empty(Gid *gid, unsigned short vid);
void propagate_join(int port, unsigned short vid);
void propagate_leave(int port, unsigned short vid);
void gvrp_set_vlan(Gid *gid, unsigned int vid);
char set_switch_vlansetting(unsigned long cmdid, unsigned int type, unsigned int data);

struct gvrp_pdu{
	unsigned char DA[6];
	unsigned char SA[6];
	unsigned char len[2];
	unsigned char llc[3];
	unsigned char protocol[2];
	unsigned char type;
	unsigned char attribute[1];
};

