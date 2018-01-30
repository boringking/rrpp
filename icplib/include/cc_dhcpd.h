#ifndef _CC_DHCPD_H_
#define _CC_DHCPD_H_

enum{
		CC_DHCP_SET_RELAY_STATE,
		CC_DHCP_SET_HOPS_LIMIT,
		CC_DHCP_SET_SERVER,
		CC_DHCP_SET_OPTION_82,
		//for dhcp snooping
		CC_DHCP_SNOOP_STATE,
		CC_DHCP_SNOOP_UPDATE,
		CC_DHCP_SNOOP_ARPINSPECT
};
typedef struct cc_relay_server{
	int type;
	u_int8_t ip[IPV6_ADDRESS_LEN];
	u_int8_t state;
	u_int8_t index;
}m_cc_relay_server, *p_cc_relay_server;

void cc_dhcpd(void);
void cc_dhcpd_cmd(void *buf, int len);
#endif
