#ifndef _LIBFACILITY_H_
#define _LIBFACILITY_H_
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include "libcfg.h"
#define LOGAPPD_CONFIG			"logappd.conf"
#define LOGID_SIZE 20
//log config use
int find_facility_code(char* facility);
int get_facility_by_cfg(char* ptype);
typedef struct _facilitycode {
	char	*c_name;
	int	c_val;
} m_facilitycode;
extern m_facilitycode facilitynames[];
#endif

