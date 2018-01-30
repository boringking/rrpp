#ifndef __NETAPP_API_H__
#define __NETAPP_API_H__

#include "ip211_netapp.h"
#include "types.h"
#include "port_config.h"

//#define DBG_NETAPP_QUEUE
#define NAME_DISCOVERY_D	"discovery_d"
#define NAME_ICP_MCP		"icp_mcp"
#define NAME_DHCPD			"dhcpd"
#define NAME_GVRP			"gvrp"
#define NAME_ICP_STP		"icp_stp"
#define NAME_ICP_LACP		"icp_lacp"
#define NAME_AUTODSCO		"autodsco"
#define NAME_POE_D			"poe_d"
#define NAME_HOSTAPD		"hostapd"
#define NAME_DHCPD_ICMPV6	"dchpd_icmpv6"

int netapp_open(void);
int netapp_close(int fd);
int netapp_rqueue(int fd, netapp_ifreq_qHandle* retP, char* pszName);
int netapp_dqueue(int fd, netapp_ifreq_qHandle qH, char* pszName);
int netapp_reg(int fd, netapp_ifreq_qHandle qH, struct netapp_netf* fP, netapp_ifreq_fHandle* retP);
int netapp_unreg(int fd, netapp_ifreq_qHandle qH, netapp_ifreq_fHandle fH);
int netapp_read(int fd, netapp_ifreq_qHandle qH, int maxLen, int maxWaitJiffies, struct netapp_netb* bP);
int netapp_read_until_finish(int fd, netapp_ifreq_qHandle qH, int maxLen, struct netapp_netb* bP);
int netapp_query(int fd, netapp_ifreq_qHandle qH, unsigned int* countP, unsigned int* jiffiesP);
int netapp_debug(int fd, unsigned int type);
// modify (YT, 20130322)
int netapp_drop_stop(int fd, netapp_ifreq_qHandle qH);
int netapp_restart(int fd, netapp_ifreq_qHandle qH);
int netapp_clean(int fd, netapp_ifreq_qHandle qH);
// end modify (YT, 20130322)
void select_port_to_send(void *spctag_addr, int port_no, int type);
int send_packet(void *buf, int len);

/*******************************************************************************
*	Structures
*******************************************************************************/
struct pro_buff
{
	unsigned char	*head;			// pointer to packet head
	unsigned char	*data;			// pointer to packet data
	unsigned char	*std;			// pointer to special tag data
	unsigned char	*sflow;			// pointer to sFlow info
	unsigned char	*ptp;			// pointer to PTP stamp info
	
	unsigned int	len;			// packet length
	unsigned int	data_len;
	int 			sp;				// source port
};
void icp_netb2pro(struct pro_buff *pb, struct netapp_netb *netb);
#endif

