#ifndef IP1829DS_H
#define IP1829DS_H

#include "portnumber.h"
#include "list.h"

/*typedef unsigned char 	u8;
typedef unsigned short  u16;
typedef unsigned long	u32;*/

struct GeneralSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int gdata;
};

struct PortMemberSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned long member;
};

struct ByPortSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int port;
	int pdata;
};

struct ByPortSetting32
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int port;
	unsigned long pdata;
};

struct AllPortsSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int apdata[SWITCH_MAX_IPORT_CNT];
};

struct PortmapSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned long portmap;
	int pmdata;
};

struct PortMaskSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned long portmap;
	int mask;
};
/*--------------- Special Data Structures for SMI ---------------*/
struct LinkStatusSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
    int port;
    unsigned int link;
	unsigned int speed;
    unsigned int duplex;
    unsigned int pause;
    unsigned int asym;
    unsigned int an;
};

/*--------------- Special Data Structures for L2 Protocol ---------------*/
struct CapActSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned int protocol;
	int act;
};

/*--------------- Special Data Structures for Storm ---------------*/
struct StormGeneralSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
    unsigned char storm;
	long sdata;
};

/*--------------- Special Data Structures for Loop Detect ---------------*/
struct LDDASetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned char da[6];
};

/*--------------- Special Data Structures for Special Tag ---------------*/
struct STagTypeLenSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned int length;
	unsigned int type;
};

/*--------------- Special Data Structures for MISC ---------------*/
struct RegSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned char page;
	unsigned char reg;
	unsigned short val;
};

/*--------------- Special Data Structures for STP ---------------*/
struct StpByFPSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int fid;
	int port;
	int pstate;
};

struct StpAllPortsSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int fid;
	int pstate[SWITCH_MAX_IPORT_CNT-1];
};

/*--------------- Special Data Structures for LACP ---------------*/
struct TrunkMemberSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
    unsigned long portmask;
	unsigned long tstate;
};

struct TrunkCombineSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned long tgrps;
	int cen;
};

/*--------------- Special Data Structures for LUT ---------------*/
struct IP1829LUTEntry
{
	unsigned char mac[6];
	unsigned char fid;
	unsigned char srcport;
	unsigned char aging;
	unsigned char priority;
	struct{
		unsigned short pri_en: 1;
		unsigned short drop: 1;
		unsigned short snif1: 1;
		unsigned short snif2: 1;
		unsigned short sflow: 1;
		unsigned short reserve: 11;
	}flag;
};

struct IP1829LUTSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned long retval;
	unsigned char action;
	unsigned long tarports;
	unsigned short index;
	unsigned char block;
	struct IP1829LUTEntry entry;
};

/*--------------- Special Data Structures for IGMP ---------------*/

struct mt_rule{
	unsigned char group[4];
	unsigned char fid;
	unsigned long port_mem;
	unsigned char slt_index;
	unsigned long port_filter_mode;
	unsigned char flag;
};

struct MtRuleSetting{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int index;
	struct mt_rule mt_data;
};

struct slt_rule{
	unsigned char type;	//4:ipv4, 6:ipv6, define in ip1829op.h
	union slt_para{
		struct slt_ipv4{
			unsigned char ip[10][4];
			unsigned long used_port[10];
		}ipv4;

		struct slt_ipv6{
			unsigned short ip[4][8];
			unsigned long used_port[4];
		}ipv6;
	}data;
};

struct SltRuleSetting{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int index;
	struct slt_rule slt_data;
};

struct IgmpPacketRule{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned int packet_type;
	int rule;
};

struct IgmpRouterListSetting{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned long portmask;
	unsigned long tstate;
};

/*--------------- Special Data Structures for IMP ---------------*/
struct IP1829IMPEntry
{
	unsigned char ip[16];
	unsigned char mac[6];
	unsigned char srcport;
	unsigned char priority;
	struct{
		unsigned short valid: 1;
		unsigned short ip_type: 1;
		unsigned short check_ip: 1;
		unsigned short check_mac: 1;
		unsigned short check_port: 1;
		unsigned short filter: 1;
		unsigned short sniff2: 1;
		unsigned short reserved: 9;
	}flag;
};

struct IP1829IMPSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned long retval;
	unsigned char action;
	unsigned char index;
	struct IP1829IMPEntry entry;
};

/*--------------- Special Data Structures for VLAN ---------------*/
struct VlanSetting{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned int vtype;
	unsigned int vdata;
};

/*--------------- Special Data Structures for MAC ---------------*/
struct MACSetting{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	char mac[6];
};

/*--------------- Special Data Structures for TCP Flag ---------------*/
struct TcpFlagSetting{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned int index;
	int fdata;
};

/*--------------- Special Data Structures for IPv6 ---------------*/
struct IPv6Setting{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned int header;
	int act;
};

/*--------------- Special Data Structures for MIB Counter ---------------*/
#define NUM_MIB_COUNTER_RX  24
#define NUM_MIB_COUNTER_TX  20

struct MIBCounterData{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int port;
	int dir;
	int idx;
	unsigned long counter;
};

struct MIBCounterEntry{
	unsigned long RX_counter[NUM_MIB_COUNTER_RX];
	unsigned long TX_counter[NUM_MIB_COUNTER_TX];
};

struct MIBCounterEntry_all{
	struct MIBCounterEntry entry[SWITCH_MAX_IPORT_CNT];
};

/*--------------- Special Data Structures for QOS ---------------*/
struct qos_dscp_setting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	unsigned char dscpentry;
	unsigned char dscpvalue;
	unsigned char dscpqueue;
};

struct qos_modesettings
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int groupnum;
	int queuenum;
	int modesettings;
};

struct qos_remap
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int port;
	unsigned char queue;
	unsigned char remap;
};
/*--------------- Special Data Structures for ACL ---------------*/
struct acl_man {
	struct list_head	rule_list;

	int num_used_rules;
	int num_used_entries;

	// IP1829 has 128 entries
	unsigned short	used_entry_mask[8];
};

struct acl_man_rule {
	struct list_head	rule_entry;
	int start_index;
	int num_entries;
};

enum {
	ACL_SELECT_MODE_0001 = 1,
	ACL_SELECT_MODE_0010,
	ACL_SELECT_MODE_0011,
	ACL_SELECT_MODE_0100,
	ACL_SELECT_MODE_0101,
	ACL_SELECT_MODE_0110,
	ACL_SELECT_MODE_0111,
	ACL_SELECT_MODE_1000,
	ACL_SELECT_MODE_1001,
	ACL_SELECT_MODE_1010,
	ACL_SELECT_MODE_1011,
	ACL_SELECT_MODE_1100,
	ACL_SELECT_MODE_1110 = 14,
	ACL_SELECT_MODE_1111,
};
#define		ACL_SELECT_MODE_BIT_0010			BIT(ACL_SELECT_MODE_0010)
#define		ACL_SELECT_MODE_BIT_0011			BIT(ACL_SELECT_MODE_0011)
#define		ACL_SELECT_MODE_BIT_0100			BIT(ACL_SELECT_MODE_0100)
#define		ACL_SELECT_MODE_BIT_0101			BIT(ACL_SELECT_MODE_0101)
#define		ACL_SELECT_MODE_BIT_0110			BIT(ACL_SELECT_MODE_0110)
#define		ACL_SELECT_MODE_BIT_0111			BIT(ACL_SELECT_MODE_0111)
#define		ACL_SELECT_MODE_BIT_1000			BIT(ACL_SELECT_MODE_1000)
#define		ACL_SELECT_MODE_BIT_1001			BIT(ACL_SELECT_MODE_1001)
#define		ACL_SELECT_MODE_BIT_1010			BIT(ACL_SELECT_MODE_1010)
#define		ACL_SELECT_MODE_BIT_1011			BIT(ACL_SELECT_MODE_1011)
#define		ACL_SELECT_MODE_BIT_1100			BIT(ACL_SELECT_MODE_1100)
#define		ACL_SELECT_MODE_BIT_1110			BIT(ACL_SELECT_MODE_1110)
#define		ACL_SELECT_MODE_BIT_1111			BIT(ACL_SELECT_MODE_1111)
enum {
	ACL_LINK_TYPE_00 = 0,
	ACL_LINK_TYPE_01,
	ACL_LINK_TYPE_10,
	ACL_LINK_TYPE_11,
};

#define		ACL_RULE_VALID_SMAC				BIT(0)
#define		ACL_RULE_VALID_SMAC_MASK		BIT(1)
#define		ACL_RULE_VALID_DMAC				BIT(2)
#define		ACL_RULE_VALID_DMAC_MASK		BIT(3)
#define		ACL_RULE_VALID_SIP				BIT(4)
#define		ACL_RULE_VALID_SIP_MASK			BIT(5)
#define		ACL_RULE_VALID_DIP				BIT(6)
#define		ACL_RULE_VALID_DIP_MASK			BIT(7)
#define		ACL_RULE_VALID_SIP6				BIT(8)
#define		ACL_RULE_VALID_SIP6_MASK		BIT(9)
#define		ACL_RULE_VALID_DIP6				BIT(10)
#define		ACL_RULE_VALID_DIP6_MASK		BIT(11)
#define		ACL_RULE_VALID_ETH_TYPE			BIT(12)
#define		ACL_RULE_VALID_VLAN				BIT(13)
#define		ACL_RULE_VALID_COS				BIT(14)
#define		ACL_RULE_VALID_SP				BIT(15)
#define		ACL_RULE_VALID_SP_R				BIT(16)
#define		ACL_RULE_VALID_DP				BIT(17)
#define		ACL_RULE_VALID_DP_R				BIT(18)
#define		ACL_RULE_VALID_TCP_FLAG			BIT(19)
#define		ACL_RULE_VALID_TCP_FLAG_MASK	BIT(20)
#define		ACL_RULE_VALID_DSCP				BIT(21)
#define		ACL_RULE_VALID_IP_PROT			BIT(22)
#define		ACL_RULE_VALID_INGRESS_PORT		BIT(23)
#define		ACL_RULE_VALID_ROUTE_PORT		BIT(24)

#define		ACL_ACT_VALID_REDIR				BIT(0)
#define		ACL_ACT_VALID_PRI				BIT(1)
#define		ACL_ACT_VALID_DSCP				BIT(2)
#define		ACL_ACT_VALID_CPU				BIT(3)
#define		ACL_ACT_VALID_SNIFFER			BIT(4)
#define		ACL_ACT_VALID_PTP				BIT(5)
#define		ACL_ACT_VALID_SFLOW				BIT(6)
#define		ACL_ACT_VALID_CTAG				BIT(7)
#define		ACL_ACT_VALID_STAG				BIT(8)
#define		ACL_ACT_VALID_BW				BIT(9)
#define		ACL_ACT_VALID_CTAG_VLAN			BIT(10)
#define		ACL_ACT_VALID_STAG_VLAN			BIT(11)
#define		ACL_ACT_VALID_MIB_COUNTER		BIT(12)

#define		ACL_ACT_TYPE_5					5

struct acl_rule
{
	// rule
	unsigned long	rule_valid;			// bit 0 : smac
										// bit 1 : smac_mask
										// bit 2 : dmac
										// bit 3 : dmac_mask
										// bit 4 : sip
										// bit 5 : sip_mask
										// bit 6 : dip
										// bit 7 : dip_mask
										// bit 8 : sip6
										// bit 9 : sip6_mask
										// bit 10: dip6
										// bit 11: dip6_mask
										// bit 12: eth_type
										// bit 13: vlan
										// bit 14: cos
										// bit 15: sp
										// bit 16: sp_range
										// bit 17: dp
										// bit 18: dp_range
										// bit 19: tcp_flag
										// bit 20: tcp_flag_mask
										// bit 21: dscp
										// bit 22: ip_prot
										// bit 23: ingress_port
										// bit 24: route_port
	// reverse
	unsigned long	rule_rvs;			// bit 0 : smac
										// bit 1 : smac_mask
										// bit 2 : dmac
										// bit 3 : dmac_mask
										// bit 4 : sip
										// bit 5 : sip_mask
										// bit 6 : dip
										// bit 7 : dip_mask
										// bit 8 : sip6
										// bit 9 : sip6_mask
										// bit 10: dip6
										// bit 11: dip6_mask
										// bit 12: eth_type
										// bit 13: vlan
										// bit 14: cos
										// bit 15: sp
										// bit 16: sp_range
										// bit 17: dp
										// bit 18: dp_range
										// bit 19: tcp_flag
										// bit 20: tcp_flag_mask
										// bit 21: dscp
										// bit 22: ip_prot
										// bit 23: ingress_port
										// bit 24: route_port

	unsigned char	smac[6];
	unsigned char	smac_mask;			// 0 - FF:FF:FF:FF:FF:FF
										// 2 - FF:FF:FF:00:00:00
										// 3 - FF:FF:00:00:00:00
	unsigned char	dmac[6];
	unsigned char	dmac_mask;

	union{
		unsigned long	sip4;
		unsigned short	sip6[8];
	}sip;
#define sip4_addr	sip.sip4
#define sip6_addr16	sip.sip6
	unsigned char	sip_mask;			// IPv4	0 - FF:FF:FF:FF		IPv6 0 - FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF
										// 		2 - FF:FF:FF:F0			 1 - FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:0000:0000
										// 		3 - FF:FF:FF:00			 3 - FFFF:FFFF:FFFF:0000:0000:0000:0000:0000
										// 		4 - FF:FF:F0:00			 7 - FFFF:0000:0000:0000:0000:0000:0000:0000
										// 		5 - FF:FF:00:00
										// 		6 - FF:00:00:00
										// 		7 - F0:00:00:00
	union{
		unsigned long	dip4;
		unsigned short	dip6[8];
	}dip;
#define dip4_addr	dip.dip4
#define dip6_addr16	dip.dip6
	unsigned char	dip_mask;

	unsigned short	eth_type;
	unsigned short	vlan;
	unsigned short	cos;
	unsigned short	sp_hi;
	unsigned short	sp_lo;
	unsigned short	dp_hi;
	unsigned short	dp_lo;
	unsigned char	tcp_flag;
	unsigned char	tcp_flag_mask;
	unsigned char	r_dscp;
	unsigned char	ip_prot;
	unsigned char	ingress_port;
	unsigned char	route_port;

	// action
	int				act_type;			// value = 0 ~ 5, type 5 need a acl entry
										// 0 for drop
										// 1 for Act_s = 2b'11
										// 2 for Act_s = 2b'10
										// 3 for Act_s = 2b'01
										// 4 for Act_s = 2b'00
	unsigned long	act_valid;			// bit 0 : redir
										// bit 1 : pri
										// bit 2 : dscp
										// bit 3 : cpu
										// bit 4 : mirror
										// bit 5 : ptp
										// bit 6 : sflow
										// bit 7 : ctag
										// bit 8 : stag
										// bit 9 : bw
										// bit 10: ctag vlan
										// bit 11: stag vlan
	unsigned char	redir:5;
	unsigned char	pri:3;
	unsigned char	a_dscp:3;
	unsigned char	cpu:1;
	unsigned char	mirror:1;
	unsigned char	ptp:1;
	unsigned char	sflow:1;
	unsigned char	ctag_vlan:1;
	unsigned char	stag_vlan:1;
	unsigned short	ctag;
	unsigned short	stag;
	unsigned short	bw;
	unsigned short	mib;
};

struct AclRuleSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int index;
	struct acl_rule rule;
	unsigned long reserved;			// show the used entry numbers for ip1829
};

struct AclGeneralSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int index;
	int data;
};

struct AclVlanGroupSetting
{
	void *nextcmd;
	unsigned long size_nextcmd;
	unsigned long cmdid;
	int index;
	unsigned long portmap;
};

#endif		/* IP1829DS_H */
