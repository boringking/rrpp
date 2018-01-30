#ifndef __PORTNUMBER_H__
#define __PORTNUMBER_H__

#define SWITCH_MAX_PORT_NUM		28
#define SWITCH_GIGA_PORT_START	25
#define SWITCH_GIGA_PORT_END	28

#define SWITCH_MAX_IPORT_CNT	29

unsigned char getPhysicalPort(unsigned log, unsigned char type);
unsigned char getLogicalPort(unsigned phy, unsigned char type);
unsigned long getPhysicalPortList(unsigned long log_ports);
unsigned long getLogicalPortList(unsigned long phy_ports);
unsigned char get_MaxPort(void);
unsigned char get_GPStart(void);
unsigned char get_GPEnd(void);
#endif
