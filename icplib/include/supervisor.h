#ifndef _SUPERVISOR_H_
#define _SUPERVISOR_H_
#include <linux/ioctl.h>

#define SV_IOC_MAGIC		'x'

#define SV_MAX_COMM_SIZE	20
#define SV_MAX_PATH_SIZE	50
#define SV_MAX_ARGV_SIZE	50
#define SV_MAX_AGRV			10

typedef struct _supervisor_ioc{
	int ret;
	char comm[SV_MAX_COMM_SIZE];
	char path[SV_MAX_PATH_SIZE];
	char argv[SV_MAX_AGRV][SV_MAX_ARGV_SIZE];
}m_supervisor_ioc;

/* Add a daemon to daemon-list
 * Arguments:
 *	comm: daemon command
 *	path: binary file path from /
 *	argv: arguments include command
 * Return value:
 *	0: success
 *	-1:error
 */
#define SV_IOC_ADD		_IOWR(SV_IOC_MAGIC,  1, struct _supervisor_ioc)

/* Delete a daemon form daemon-list
 * Return value:
 *	1: success
 *	0: target not found
 *	-1:error
 */
#define SV_IOC_DEL		_IOC(_IOC_READ|_IOC_WRITE,SV_IOC_MAGIC,2,(sizeof(char)*SV_MAX_COMM_SIZE))

/* Search a daemon from daemon-list and record the process counter and disappear counter
 * Return value:
 * 1:	target exists
 * 0:	target not found
 * -1:	error
 */
#define SV_IOC_SEARCH	_IOC(_IOC_READ|_IOC_WRITE,SV_IOC_MAGIC,3,(sizeof(char)*SV_MAX_COMM_SIZE))

#define	SV_IOC_MAX_NUM	3

#endif//_SUPERVISOR_H_
