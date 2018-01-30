
#ifndef __PORT_CONFIG_H__
#define __PORT_CONFIG_H__
#include "portnumber.h"

#define PORT_SEQUENCE_FUNC
#define MAX_PORT_NUM		get_MaxPort()
#define GIGA_PORT_START		get_GPStart()
#define GIGA_PORT_END		get_GPEnd()

#define MAX_IPORT_CNT		SWITCH_MAX_IPORT_CNT
#define	ALL_PORTS_LIST		(~(-1 << MAX_PORT_NUM))
#define	GIGA_PORT_NUM		(GIGA_PORT_END - GIGA_PORT_START)
#define CPU_PORT_NUM		MAX_IPORT_CNT
#define MAX_TP_PORT_NUM		(GIGA_PORT_START -1)

#endif

