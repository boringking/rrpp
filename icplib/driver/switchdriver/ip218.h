#ifndef IP218_H
#define IP218_H

#if 1
#define IP218_MAC_BASE			0xBC400000	//Virtual Address
#else
#define IP218_MAC_BASE			0x1C400000	//Physical Address
#endif

#define	IP218_MAC_SMICTRL0		0x0004
#define	IP218_MAC_SMICTRL1		0x0008

struct ip218_smictrl0{
	unsigned long wr_data:16;
	unsigned long en:1;
	unsigned long rdwr:1;
	unsigned long rev2:1;
	unsigned long phy:5;
	unsigned long rev1:3;
	unsigned long reg:5;
};

struct ip218_smictrl1{
	unsigned long mdc_clk_divisor:3;
	unsigned long rev:13;
	unsigned long rd_data:16;
};

#endif
