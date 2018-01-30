#ifndef __H_IP1829DBS__
#define __H_IP1829DBS__

#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/file.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include "list.h"
#include "dbsfunc.h"

#define IP1829_DBSNAME	"ip1829_dbsdev"
#define FLASH_CURRENT	"/etc/config/flash/current/"

typedef struct _subdata{
	char *name;
	char *value;
	struct list_head list;
}m_subdata, *p_subdata;

typedef struct _entrydata{
	char *name;
	char *value;
	unsigned short entry_num;
	struct list_head sub_head;
	struct list_head list;
}m_entrydata, *p_entrydata;

typedef struct _filelist{
	char *filename;
	char need_save;
	struct list_head entry_head;
	struct list_head list;
}filelist, *p_filelist;

extern struct list_head file_head;
#endif
