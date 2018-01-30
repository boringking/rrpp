#ifndef IP1829OP_H
#define IP1829OP_H

#define OP_FUNC_DISABLE		0
#define OP_FUNC_ENABLE		1

#define OP_SMI_DUPLEX_HALF	0
#define OP_SMI_DUPLEX_FULL	1

#define OP_SMI_SPEED_1000	1000
#define OP_SMI_SPEED_100	100
#define OP_SMI_SPEED_10		10

#define OP_CAP_PTCL_BPDU	0
#define OP_CAP_PTCL_SLOW	1
#define OP_CAP_PTCL_802_1X	2
#define OP_CAP_PTCL_LLDP	3
#define OP_CAP_PTCL_GRP0	4
#define OP_CAP_PTCL_BRIDGE	5
#define OP_CAP_PTCL_GRP1	6
#define OP_CAP_PTCL_GARP	7
#define OP_CAP_PTCL_GRP2	8
#define OP_CAP_PTCL_GRP3	9

#define OP_CAP_ACT_FORWARD	0
#define OP_CAP_ACT_ALL_PORT	1
#define OP_CAP_ACT_TO_CPU	2
#define OP_CAP_ACT_DROP		3

enum{
	OP_CAT_INBAND_ARP,	//0
	OP_CAT_INBAND_IPV4,
	OP_CAT_INBAND_IPV6,
	OP_CAT_INBAND_PPPOE,
	OP_CAT_INBAND_ICMP,
	OP_CAT_INBAND_TCP,
	OP_CAT_INBAND_UDP,
	OP_CAT_INBAND_USER_ETH,
	OP_CAT_INBAND_USER_IP1,	//8
	OP_CAT_INBAND_USER_IP2,
	OP_CAT_INBAND_ICMPV6,
	OP_CAT_INBAND_FTP,
	OP_CAT_INBAND_SSH,
	OP_CAT_INBAND_TELNET,
	OP_CAT_INBAND_SMTP,
	OP_CAT_INBAND_DNS,
	OP_CAT_INBAND_BOOTP,	//16
	OP_CAT_INBAND_TFTP,
	OP_CAT_INBAND_HTTP,
	OP_CAT_INBAND_POP3,
	OP_CAT_INBAND_NEWS,
	OP_CAT_INBAND_SNTP,
	OP_CAT_INBAND_NETBIOS,
	OP_CAT_INBAND_IMAP,
	OP_CAT_INBAND_SNMP,	//24
	OP_CAT_INBAND_HTTPS,
	OP_CAT_INBAND_USER_TCPUDP_A,
	OP_CAT_INBAND_USER_TCPUDP_B,
	OP_CAT_INBAND_USER_TCPUDP_C,
	OP_CAT_INBAND_USER_TCPUDP_D,
	OP_CAT_INBAND_USER_TCPUDP_E,
	OP_CAT_INBAND_TOTALNUM	//31
};

enum{
	OP_CAT_L3_ICMP,	//0
	OP_CAT_L3_TCP,
	OP_CAT_L3_UDP,
	OP_CAT_L3_OSPF,
	OP_CAT_L3_USR1,
	OP_CAT_L3_USR2,
	OP_CAT_L3_IPV4_OTHER,
	OP_CAT_L3_TOTALNUM	//7
};

enum{
	OP_TCPUDP_PTCL_FTP,	//0
	OP_TCPUDP_PTCL_SSH,
	OP_TCPUDP_PTCL_TELNET,
	OP_TCPUDP_PTCL_SMTP,
	OP_TCPUDP_PTCL_DNS,
	OP_TCPUDP_PTCL_DHCP,
	OP_TCPUDP_PTCL_TFTP,
	OP_TCPUDP_PTCL_HTTP,
	OP_TCPUDP_PTCL_POP3,
	OP_TCPUDP_PTCL_NEWS,
	OP_TCPUDP_PTCL_SNTP,	//10
	OP_TCPUDP_PTCL_NETBIOS,
	OP_TCPUDP_PTCL_IMAP,
	OP_TCPUDP_PTCL_SNMP,
	OP_TCPUDP_PTCL_HTTPS,
	OP_TCPUDP_PTCL_USR_A,
	OP_TCPUDP_PTCL_USR_B,
	OP_TCPUDP_PTCL_USR_C,
	OP_TCPUDP_PTCL_USR_D,
	OP_TCPUDP_PTCL_USR_E,
	OP_TCPUDP_PTCL_TOTALNUM	//20
};

enum{
	OP_TCPUDP_USER_A,
	OP_TCPUDP_USER_B,
	OP_TCPUDP_USER_C_UPPER,
	OP_TCPUDP_USER_C_LOWER,
	OP_TCPUDP_USER_D_UPPER,
	OP_TCPUDP_USER_D_LOWER,
	OP_TCPUDP_USER_E_UPPER,
	OP_TCPUDP_USER_E_LOWER,
	OP_TCPUDP_USER_TOTALNUM
};

enum{
	OP_TCPUDP_ACT_Q0,	//0
	OP_TCPUDP_ACT_Q1,
	OP_TCPUDP_ACT_Q2,
	OP_TCPUDP_ACT_Q3,
	OP_TCPUDP_ACT_Q4,
	OP_TCPUDP_ACT_Q5,
	OP_TCPUDP_ACT_Q6,
	OP_TCPUDP_ACT_Q7,

	OP_TCPUDP_ACT_TO_CPU = 9,	//9
	OP_TCPUDP_ACT_DROP,
	OP_TCPUDP_ACT_ALL_PORT
};

enum{
	OP_TCPFLAG_FLAG0,
	OP_TCPFLAG_FLAG1,
	OP_TCPFLAG_FLAG2,
	OP_TCPFLAG_FLAG3
};

enum{
	OP_TCPFLAG_ACT_NONE,
	OP_TCPFLAG_ACT_STORMCTRL,
	OP_TCPFLAG_ACT_TO_CPU,
	OP_TCPFLAG_ACT_DROP
};

enum{
	OP_IPV6_HEADER_FRAG,
	OP_IPV6_HEADER_ENACP,
	OP_IPV6_HEADER_AUTH,
	OP_IPV6_HEADER_ICMPV6,
	OP_IPV6_HEADER_ICMPV6_MLD,
	OP_IPV6_HEADER_ICMPV6_NDP,
	OP_IPV6_HEADER_USER1,
	OP_IPV6_HEADER_USER2,
	OP_IPV6_HEADER_ICMPV6_USER1,
	OP_IPV6_HEADER_ICMPV6_USER2,
	OP_IPV6_HEADER_TOTALNUM
};

enum{
	OP_IPV6_HEADER_ICMPV6_USER1_HIGH,
	OP_IPV6_HEADER_ICMPV6_USER1_LOW,
	OP_IPV6_HEADER_ICMPV6_USER2_HIGH,
	OP_IPV6_HEADER_ICMPV6_USER2_LOW
};

#define OP_SNIFFER1_METHOD_DISABLE	0
#define OP_SNIFFER1_METHOD_EGRESS	1
#define OP_SNIFFER1_METHOD_INGRESS	2
#define OP_SNIFFER1_METHOD_BOTHDIR	3

#define OP_SNIFFER1_PKT_MODIFY	0
#define OP_SNIFFER1_PKT_KEEP	1

#define OP_SNIFFER1_TAG_KEEP	0
#define OP_SNIFFER1_TAG_MODIFY	1

#define OP_SNIFFER2_LUT_TRIGGER_TARGET_DA	0
#define OP_SNIFFER2_LUT_TRIGGER_TARGET_SA	1

#define OP_STORM_BCST		0x01
#define OP_STORM_MCST		0x02
#define OP_STORM_DLF		0x04
#define OP_STORM_ARP		0x08
#define OP_STORM_ICMP		0x10

#define OP_EOC_STATUS_NORMAL		0
#define OP_EOC_STATUS_LOOP_DETECTED	1

#define OP_EOC_RELEASE_TIME_1MIN	1
#define OP_EOC_RELEASE_TIME_10MIN	10

#define OP_LD_TIME_UNIT_500MS		0
#define OP_LD_TIME_UNIT_1S			1

#define OP_LD_STATUS_NORMAL			0
#define OP_LD_STATUS_LOOP_DETECTED	1

#define OP_WOL_MODE_SLAVE		0
#define OP_WOL_MODE_MASTER		1

#define OP_WOL_IDLE_UNIT_DISABLE	0
#define OP_WOL_IDLE_UNIT_10s		1
#define OP_WOL_IDLE_UNIT_1min		2
#define OP_WOL_IDLE_UNIT_10min		3

#define OP_WOL_STATUS_NORMAL		0
#define OP_WOL_STATUS_SLEEPING		1
#define OP_WOL_STATUS_RDY4SLEEP		2
#define OP_WOL_STATUS_SLEEP			3

#define OP_CPU_PORT_NORMAL	0
#define OP_CPU_PORT_CPU		1

#define OP_MAC_SELF_TEST_PKT_NO_32768	32768
#define OP_MAC_SELF_TEST_PKT_NO_4096	4096
#define OP_MAC_SELF_TEST_PKT_NO_256		256
#define OP_MAC_SELF_TEST_PKT_NO_16		16

#define OP_MAC_SELF_TEST_RESULT_FAIL	0
#define OP_MAC_SELF_TEST_RESULT_PASS	1

#define OP_BPDU_CMODE_GLOBAL	0
#define OP_BPDU_CMODE_BY_PORT	1

#define OP_STP_STATE_DISCARD	0
#define OP_STP_STATE_BLOCK	1
#define OP_STP_STATE_LEARN	2
#define OP_STP_STATE_FORWARD	3

#define OP_TRUNK_HASH_METHOD_PORT_ID	0
#define OP_TRUNK_HASH_METHOD_SA		1
#define OP_TRUNK_HASH_METHOD_DA		2
#define OP_TRUNK_HASH_METHOD_DA_SA	3
#define OP_TRUNK_HASH_METHOD_DIP	4
#define OP_TRUNK_HASH_METHOD_SIP	5
#define OP_TRUNK_HASH_METHOD_DP		6
#define OP_TRUNK_HASH_METHOD_SP		7

#define OP_TRUNK_COMBINE_G1_G2		5
#define OP_TRUNK_COMBINE_G3_G4		6
#define OP_TRUNK_COMBINE_G5_G6		7

#define OP_LUT_LEARN_MODE_ALL_BY_AGING_TIME	0x0
#define OP_LUT_LEARN_MODE_NEVER_OVERWRITE	0x1
#define	OP_LUT_LEARN_MODE_L2_BY_AGING_TIME	0x3

#define	OP_HASH_DIRECT	0
#define OP_HASH_CRC		1

#define	OP_LUT_UNKNOWNU_SA_DROP			0x0
#define OP_LUT_UNKNOWNU_SA_FWD_2_CPU	0x1
#define	OP_LUT_UNKNOWN_SA_FWD			0x2

#define OP_ENTRY_CREATE	0
#define	OP_ENTRY_CONFIG 1
#define	OP_ENTRY_DELETE	2

#define OP_ENTRY_GET_BY_INDEX	3
#define OP_ENTRY_GET_BY_INFO	4
#define OP_ENTRY_GET_VALID_ONE	5

enum{
	/* target entry is valid and index values are matched */
	OP_ENTRY_EXISTS=0xE0,
	/* target entry is valid but index values are not matched */
	OP_ENTRY_NOT_MATCH,
	/* target entry is invalid */
	OP_ENTRY_NOT_FOUND,
};

#define OP_IGMP_PACKET_QUERY			0
#define OP_IGMP_PACKET_LEAVE			4
#define OP_IGMP_PACKET_UN_REG_DATA		8
#define OP_IGMP_PACKET_UN_DEFINED		12
#define OP_IGMP_PACKET_REPORT			1
#define OP_IGMP_PACKET_GROUP_SPECIFIC_QUERY	6
#define OP_IGMP_PACKET_REG_DATA			11

#define OP_IGMP_RULE_BCST			0x01
#define OP_IGMP_RULE_CPU			0x02
#define OP_IGMP_RULE_ROUTER			0x04
#define OP_IGMP_RULE_DROP			0x08
#define OP_IGMP_RULE_GROUP_MEM			0x10

#define OP_IGMP_SLT_IPV4			4
#define OP_IGMP_SLT_IPV6			6

#define OP_MLD_SEND_TO_PORTS			0
#define OP_MLD_SEND_TO_PORTS_AND_CPU		1
#define OP_MLD_SEND_TO_CPU			2
#define OP_MLD_DROP				3

#define	OP_IMP_DROP_IP_MISMATCH	0
#define	OP_IMP_DROP_IP_MATCH	1

#define	OP_IMP_IPTYPE_4	0
#define	OP_IMP_IPTYPE_6	1

#define OP_VLAN_EGRESS_UNI_FRAME	0x1
#define OP_VLAN_EGRESS_MULTI_FRAME	0x2
#define OP_VLAN_EGRESS_ARP_FRAME	0x4

#define OP_VLAN_TAGGING_BY_PORT		0x0
#define OP_VLAN_TAGGING_BY_VID		0x1

#define OP_VLAN_TYPE_GROUP	0x0
#define	OP_VLAN_TYPE_TAG	0x1

#define	OP_VLAN_PROTOCOL_INVALID	0x0
#define	OP_VLAN_PROTOCOL_ETHER		0x1
#define	OP_VLAN_PROTOCOL_LLC		0x2
#define	OP_VLAN_PROTOCOL_1042		0x3

#define OP_QOS_8021PEDTION_2005		0	//2005 edition
#define OP_QOS_8021PEDTION_2005_EX		1	//2005 + exchange
#define OP_QOS_8021PEDTION_EARLY		2 	//early

#define OP_QOS_GROUP1   0
#define OP_QOS_GROUP2   1

#define OP_CPU_IF_SPEED_HIGH   0
#define OP_CPU_IF_SPEED_NORMAL   1

enum{
  OP_QOS_MODE_FIFO,
  OP_QOS_MODE_WWBBT,
  OP_QOS_MODE_SP1_WWBBT7,
  OP_QOS_MODE_SP2_WWBBT6,
  OP_QOS_MODE_SP4_WWBBT4,
  OP_QOS_MODE_SP8 
};

enum{
  OP_QOS_NUM_Q0,
  OP_QOS_NUM_Q1,
  OP_QOS_NUM_Q2,
  OP_QOS_NUM_Q3,
  OP_QOS_NUM_Q4,
  OP_QOS_NUM_Q5,
  OP_QOS_NUM_Q6,
  OP_QOS_NUM_Q7
};
enum{
  OP_QOS_UNIT_64KBS,
  OP_QOS_UNIT_1MBS,
  OP_QOS_UNIT_2MBS,
  OP_QOS_UNIT_4MBS  
};
enum{
  OP_QOS_REMAP_RX_Q0,
  OP_QOS_REMAP_RX_Q1,
  OP_QOS_REMAP_RX_Q2,
  OP_QOS_REMAP_RX_Q3,
  OP_QOS_REMAP_RX_Q4,
  OP_QOS_REMAP_RX_Q5,
  OP_QOS_REMAP_RX_Q6,
  OP_QOS_REMAP_RX_Q7,
  OP_QOS_REMAP_TX_Q0,
  OP_QOS_REMAP_TX_Q1,
  OP_QOS_REMAP_TX_Q2,
  OP_QOS_REMAP_TX_Q3,
  OP_QOS_REMAP_TX_Q4,
  OP_QOS_REMAP_TX_Q5,
  OP_QOS_REMAP_TX_Q6,
  OP_QOS_REMAP_TX_Q7
};
enum{
	OP_QOS_METHOD_WRR,
	OP_QOS_METHOD_BW,
	OP_QOS_METHOD_WFQ,
	OP_QOS_METHOD_TWRR
};

#define OP_QOS_QBASE_DBM  0
#define OP_QOS_QBASE_SBM  1  

enum{
	OP_BW_PERIOD_4KB_500MS,
	OP_BW_PERIOD_2KB_250MS,
	OP_BW_PERIOD_1KB_125MS,
	OP_BW_PERIOD_512B_63MS,
	OP_BW_PERIOD_256B_31MS,
	OP_BW_PERIOD_128B_16MS,
	OP_BW_PERIOD_64B_8MS,
	OP_BW_PERIOD_32B_4MS
};

#endif		/* IP1829OP_H */
