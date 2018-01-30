#include "portnumber.h"
#define AUTO_GEN_PORT_SEQ

#ifdef PORT_SEQUENCE_FUNC
unsigned char physicalPortSeq[SWITCH_MAX_IPORT_CNT] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28};

#ifdef AUTO_GEN_PORT_SEQ
unsigned char logicalPortSeq[SWITCH_MAX_IPORT_CNT];
#else
unsigned char logicalPortSeq[SWITCH_MAX_IPORT_CNT] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
#endif
#endif//PORT_SEQUENCE_FUNC

/*===================================================================*/

/* Name: getPhysicalPort/getLogicalPort
 * Parameters:	log:	logcical port number
 *				phy:	physical port number
 *				type:	1: log value start from 1
 *						0: log value start from 0
 * */
unsigned char getPhysicalPort(unsigned log, unsigned char type)
{
#ifdef PORT_SEQUENCE_FUNC
	if(type==0)
		return physicalPortSeq[log];
	else
		return physicalPortSeq[log-1]+1;
#else//	PORT_SEQUENCE_FUNC
	return log;
#endif//PORT_SEQUENCE_FUNC
}

unsigned char getLogicalPort(unsigned phy, unsigned char type)
{
#ifdef PORT_SEQUENCE_FUNC
#ifdef AUTO_GEN_PORT_SEQ
	int i;
	for(i=0; i<SWITCH_MAX_IPORT_CNT; i++)
	{
		if((type==0) && (physicalPortSeq[i] == phy))
		{
			return i;
		}
		else
		if( (type==1) && (physicalPortSeq[i] == (phy-1)) )
		{
			return i+1;
		}
	}
#else//AUTO_GEN_PORT_SEQ
	if(type==0)
		return logicalPortSeq[phy];
	else
		return logicalPortSeq[phy-1]+1;
#endif//AUTO_GEN_PORT_SEQ
#else//	PORT_SEQUENCE_FUNC
	return phy;
#endif//PORT_SEQUENCE_FUNC
}

unsigned long getPhysicalPortList(unsigned long log_ports)
{
#ifdef PORT_SEQUENCE_FUNC
	unsigned long phy_ports = 0;
	int i;
	for(i=0; i<SWITCH_MAX_IPORT_CNT; i++)
		phy_ports |= ((log_ports >> i)&0x1) << physicalPortSeq[i];

	return phy_ports;
#else//	PORT_SEQUENCE_FUNC
	return log_ports;
#endif//PORT_SEQUENCE_FUNC
}

unsigned long getLogicalPortList(unsigned long phy_ports)
{
#ifdef PORT_SEQUENCE_FUNC
	unsigned long log_ports = 0;
	int i;
#ifdef AUTO_GEN_PORT_SEQ
	int j;
	for(i=0; i<SWITCH_MAX_IPORT_CNT; i++)
	{
		for(j=0; j<MAX_IPORT_CNT; j++)
		{
			if(physicalPortSeq[j]==i)
			{
				logicalPortSeq[i]=j;
				break;
			}
		}
	}
#endif
	for(i=0; i<SWITCH_MAX_IPORT_CNT; i++)
		log_ports |= ((phy_ports >> i)&0x1) << logicalPortSeq[i];

	return log_ports;
#else//	PORT_SEQUENCE_FUNC
	return phy_ports;
#endif//PORT_SEQUENCE_FUNC
}

unsigned char get_MaxPort(void){
	return SWITCH_MAX_PORT_NUM;
}

unsigned char get_GPStart(void){
	return SWITCH_GIGA_PORT_START;
}

unsigned char get_GPEnd(void){
	return SWITCH_GIGA_PORT_END;
}

