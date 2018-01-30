#ifndef	__COMMON_CONTROL_H__
#define	__COMMON_CONTROL_H__

#define CC_SPEC_TAG		"iccc"
#define CC_FLODER		"/etc/common_control/proc_name/"
#define CC_BUF_SIZE		(512)

//common control ID
#define CC_DHCPD		"dhcpd"
#define CC_RELAYAGENT	"relayagent"
#define	CC_DHCPSNOOPING	"dhcpsnooping"

#define CC_LOGAPPD		"logappd"
#define CC_SYSLOG_MSG	"syslog_msg"
#define CC_CLILOG		"cli_logappd"

#define CC_MISC_APP		"misc_app"
#define CC_MIB_CNT		"mib_counter"

#define CC_MCP			"mcp"
#define CC_CGI_IGMP		"cgi_igmp"
#define CC_CLI_MCP		"cli_mcp"

#define CC_GVRP			"gvrp"
#define CC_CLI_GVRP		"cli_gvrp"
#define CC_CLI_VLAN		"cli_vlan"

#define CC_POE			"poe"
#define CC_CLI_POE		"cli_poe"

#define CC_STP_LACP		"cc_stp_lacp"
#define	CC_CLI_STP		"cli_stp"
#define	CC_CLI_LACP		"cli_lacp"

//common control reserved option ID(0xF000~0xFFFF)
#define CC_OP_REPLY				0xF000
#define CC_OP_REQ_LOG_FACILITY	0xF001
#define CC_OP_ACK_LOG_FACILITY	0xF002
#define CC_OP_REQ_RESTART		0xF003
#define CC_OP_ACK_RESTART		0xF004

//specific option ID
#define CC_OP_STP_LACP_RESTART	1
#define CC_OP_MCP_CONFIG_ENTRY	2


typedef struct cc_data_t{
	unsigned short op;
	unsigned char seq;
	char reserve;
	char buf[1];
}cc_data;

typedef struct cc_info_t{
	char tag[4];
	char src[16];
	short len;
	char reserve[10];
	cc_data data;
}cc_info;

int cc_send(char *src, char *target, unsigned char seq, short op, char* buf, int len);

int cc_bind(char *src);

int cc_recv(int fd, char *buf, int len);

#endif//__COMMON_CONTROL_H__
