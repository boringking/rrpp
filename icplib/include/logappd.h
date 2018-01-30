#ifndef _LOGAPPD_H_
#define _LOGAPPD_H_
/******************************************************************************
*
*   Name:           relayagent.h
*
*   Description:
*
*   Copyright:      (c) 2013-2050    IC Plus Corp.
*                   All rights reserved.  By Chance
*   Version: 1.0.0.20131030
*******************************************************************************/
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <pthread.h>
#include <dirent.h>
#include <netdb.h>
#include "list.h"
//config api use
#include "libcfg.h"
//facility api use
#include "libfacility.h"
//netapp api use
#include "libnetapp.h"
//switch api use
#include "ip1829.h"
#include "ip1829ds.h"
#include "ip1829op.h"
#include "ip1829lib.h"
#include "libgmac.h"
#include "common_control.h"
//define
#define LOGAPPD_CONFIG "logappd.conf"
#define SYSLOGD_CONFIG "/tmp/syslog.conf"
#define LOGAPP_FIFO "/var/log/logappd.fifo"
#define FLASH_TMP_FILE_1 "/tmp/logfile1"
#define FLASH_TMP_FILE_2 "/tmp/logfile2"
#define FLASH_TMP_WRITEFILE "/tmp/writefile"
#define SYSLOG_MTD "/dev/mtd4"
#define MAX_FILE_PATH 128
#define MAX_LINE_SIZE 256
#define	LOGAPPD_ID	"logappd"
#define CC_SERVER	"logappd_server"
#define CC_CLIENT	"logappd_client"

#define CC_OP_LOG_REQ_FLASH_INFO	0x2001
#define CC_OP_LOG_ACK_FLASH_INFO	0x2002

//#define DEBUG
/*******************************************************************************
*	Structures
*******************************************************************************/
typedef struct flash_info{
	unsigned char	use_other;//1: 2 blocks are not empty. 0: block1 is empty
	unsigned short	blockid;
	unsigned short	line;//0~255 each block
}FLASHINFO;

typedef struct _levelcode {
	char	*c_name;
	int	c_val;
} LEVELCODE;
extern LEVELCODE levelnames[];
//extern cc_info senddata;
/*******************************************************************************
*	function prototype
*******************************************************************************/
int parse_config_file(char* config_path);
void logappd_process(void);
void cc_logappd(void);
void cc_logappd_cmd(void *buf, int len);
int cc_logappd_send_facility(char* dest,char* buf, int len);
int init_conf();
int open_fifo();
char * find_level_name(int level);
int set_facility(char* appname,char* facility);
void set_prink_level(void);
void runsyslogd();
void runklogd();
int get_flash_info();
int set_flash_info(char* pdata,int len);
void term_handler (int signal_number);
int find_facility_code(char* facility);
int get_facility_by_cfg();
int find_facility_code(char* facility);
int get_facility_by_cfg();
int cc_logappd_send_flashinfo(cc_info *cc);
#endif

