#ifndef IP1829LIB_H
#define IP1829LIB_H
#include <stdbool.h>
#include "port_config.h"
#include "ip1829ds.h"
#include "libcommon.h"

#define IP1829_IOC_TYPE		0xE0
#define IP1829_ERR_PLIST	0x80000000

//#define VIRTUAL_TEST	//Debug Mode: for virtual test on PC
#ifdef	VIRTUAL_TEST
#define SwitchOpen()			1
#define	SwitchRequest(a,b,c)	1	
#define	SwitchClose(a)			{}
#else
/* switch driver functions */
int SwitchOpen(void);
int SwitchRequest(int fd, void *dptr, int size);
int SwitchClose(int fd);
#endif

/* common functions */

bool isAlldigit(char *str);
bool isAllxdigit(char *str);

/************************************************************
 * Name:	putnchar
 * Description:  
 * Parameters:
 * Return value:
 * **********************************************************/
void putnchar(char c, unsigned int n);

/************************************************************
 * Name:	IsMacEqual
 * Description: Check whether the same between two MAC address
 * Parameters:	mac1:	first MAC address
 *				mac2:	second MAC address
 * Return value:	1:	these MAC address is equal
 *					0:	these MAC address is different
 * **********************************************************/
int IsMacEqual(const unsigned char mac1[], const unsigned char mac2[]);

/************************************************************
 * Name:	IsMacFormat
 * Description: check if target string is MAC string 
 * Parameters:	mac:	pointer of target string
 * Return value:OK:		TRUE
 *				ERROR:	FLASE
 * **********************************************************/
bool IsMacFormat(const char *mac);

/************************************************************
 * Name:	macString2array
 * Description: Convert a MAC address string to a character array 
 * Parameters:	ary:	target MAC array
 *				str:	pointer of source string
 * Return value:OK:		successed
 *				ERROR:	failed
 * **********************************************************/
int macString2array(unsigned char *ary, char *str);

/************************************************************
 * Name:	macArray2string
 * Description: Convert a MAC address character array to a string
 * Parameters:	str: pointer of target string
 *				ary: source MAC array
 * Return value:the end of target string
 * **********************************************************/
char *macArray2string(char *str, unsigned char *ary);

/************************************************************
 * Name:	portList2val
 * Description: Convert a port-list srting to a 32-bits value 
 * Parameters:	str:	pointer of soure port-list srting
 * Return value: 32-bits port-list value
 * **********************************************************/
unsigned long portList2val(char *str);

/************************************************************
 * Name:	portVal2list
 * Description: Convert a 32-bits value to a port-list string
 * Parameters:	val:	32-bits source port-list value
 *				portnum:number of max. port
 * Return value: pointer of new port-list string.
 * **********************************************************/
char *portVal2list(const unsigned long val, const int portnum);

/************************************************************
 * Name:	checkIPType
 * Description: check IP format is IPv4 or IPv6
 * Parameters:	addr: pointer of string of address
 * Return value:AF_INET:	IPv4
 *				AF_INT6:	IPv6
 *				other:		Unknown type
 * **********************************************************/
int checkIPType(const char *addr);

/*===================================================================*/
int singleDriverSetting(void *indata, const int len);
int singleRegSetting(const unsigned long cmdid, const unsigned char page, const unsigned char reg, const unsigned short val);
int singleRegGetting(const unsigned long cmdid, const unsigned char page, const unsigned char reg, unsigned short *val);
int singleGeneralSetting(const unsigned long cmdid, const int gdata);
int singleGeneralGetting(const unsigned long cmdid, int *gdata);
int singlePortMemberSetting(const unsigned long cmdid, const unsigned long member);
int singlePortMemberGetting(const unsigned long cmdid, unsigned long  *member);
int singleByPortSetting(const unsigned long cmdid, const int port, const int pdata);
int singleByPortSetting32(const unsigned long cmdid, const int port, const int pdata);
int singleByPortGetting(const unsigned long cmdid, const int port, int *pdata);
int singlePortmapSetting(const unsigned long cmdid, const unsigned long portmap, const int pmdata);
int singlePortmapGetting(const unsigned long cmdid, unsigned long *portmap, int *pmdata);
int singlePortmapGetting2(const unsigned long cmdid, unsigned long *portmap, const int pmdata);
int singlePortMaskSetting(const unsigned long cmdid, const unsigned long portmap, const int mask);
int singlePortMaskGetting(const unsigned long cmdid, const unsigned long portmap, int *mask);
int singlePortmapMapping(const unsigned long cmdid, unsigned long *portmap, int *pmdata);
int singleCapActSetting(const unsigned long cmd, const unsigned int protocol, const int act);
int singleCapActGetting(const unsigned long cmd, const unsigned int protocol, int *act);
int singleIMPSetting(const unsigned long cmdid, const struct IP1829IMPEntry *indata, const unsigned char action, unsigned long *retval);
int singleIMPGetting(const unsigned long cmdid, struct IP1829IMPEntry *indata, const unsigned char action, const unsigned char index, unsigned long *retval);
int singleVLANSetting(const unsigned long cmd, const unsigned int vtype, const unsigned int vdata);
int singleVLANGetting(const unsigned long cmd, const unsigned int vtype, unsigned int *vdata);
int singleMACSetting(const unsigned long cmd, const char *mac);
int singleMACGetting(const unsigned long cmd, char *mac);
int singleTcpFlagSetting(const unsigned long cmd, const unsigned int index, const int fdata);
int singleTcpFlagGetting(const unsigned long cmd, const unsigned int index, int *fdata);
int singleIPv6Setting(const unsigned long cmd, const unsigned int header, const int act);
int singleIPv6Getting(unsigned long cmd, unsigned int header, int *act);
int singleStormGeneralSetting(const unsigned long cmdid, const unsigned char storm, const long sdata);
int singleStormGeneralGetting(const unsigned long cmdid, const unsigned char storm, long *sdata);
int singleLUTSetting(const unsigned long cmdid, struct IP1829LUTSetting *luts);
int singleQOSModeSetting(const unsigned long cmdid, int group,int queue,int mode);
int singleQOSRemapSetting(const unsigned long cmdid, int port,int queue,int remap);
int singleDSCPSetting(const unsigned long cmdid, unsigned char entry, unsigned char value, unsigned char queue);
int singleTCPUDPQSetting(const unsigned long cmdid, unsigned int protocol, unsigned int act);
int singleLinkStatusSetting(const unsigned long cmdid, struct LinkStatusSetting *lss);
int singleAclRuleSetting(const unsigned long cmdid, struct AclRuleSetting *ars);
int singleAclGeneralSetting(const unsigned long cmdid, struct AclGeneralSetting *ags);
int singleAclVlanGroupSetting(const unsigned long cmdid, struct AclVlanGroupSetting *avg);
int singleLDDASetting(const unsigned long cmdid, unsigned char *mac);
int singleLDDAGetting(const unsigned long cmdid, unsigned char *mac);
int singleSTAGTypeLenSetting(const unsigned long cmdid, const unsigned int length, const unsigned int type);
int singleSTAGTypeLenGetting(const unsigned long cmdid, unsigned int *length, unsigned int *type);
int singleAllPortsSetting(const unsigned long cmdid, int *apdata);
int singleAllPortsGetting(const unsigned long cmdid, int *apdata);
int singleTrunkCombineSetting(const unsigned long cmdid, unsigned long tgrps, int cen);
int singleTrunkMemberSetting(const unsigned long cmdid, unsigned long portmask, unsigned long tstate);
int singleStpByFPSetting(const unsigned long cmdid, int fid, int port, int pstate);
int singleIgmpPacketRule(const unsigned long cmdid, unsigned int type, int rule);
int singleMtRuleSetting(const unsigned long cmdid, int index, struct mt_rule *mt_data);
int singleMtSltRuleSetting(const unsigned long cmdid, int index, struct slt_rule *slt_data);
int singleMIBCounterDataGetting(const unsigned long cmdid, const int port, const int dir, const int idx, unsigned long *counter);
int eepromReadByte(const unsigned short addr, unsigned char* const pReadBackData);
int eepromWriteByte(const unsigned char addr, unsigned char data2write);

#endif		/* IP1829LIB_H */
