#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "kit.h"
#include "vlan.h"
#include "ip1829.h"
#include "ip1829lib.h"

static int raw_vlan_read_member(int vid , uint32_t * portset){
	if(singleRegSetting(ID_COMMON_MISC_SET_REG,2,0xd2,0x8000|vid)<0)
		return -1;
	uint16_t data[3];
	for(int i=0;i<ARRAY_SIZE(data);++i){
		if(singleRegGetting(ID_COMMON_MISC_GET_REG,2,0xd2+i,data+i)<0)
			return -1;
	}
	*portset = data[1];
	*portset |= data[2]<<16;
	return 0;
}

static int raw_vlan_write_member(int vid , uint32_t portset){
	const uint16_t data[6] = {
		[0] = portset&0xffff,
		[1] = 0x1000 | (portset>>16),
		[5] = 0x8000,
	};
	for(int i=0;i<ARRAY_SIZE(data);++i){
		if(singleRegSetting(ID_COMMON_MISC_SET_REG,2,0xd3+i,data[i])<0)
			return -1;
	}
	if(singleRegSetting(ID_COMMON_MISC_SET_REG,2,0xd2,0xc000|vid)<0)
		return -1;
	return 0;
}

int raw_vlan_add_ports(int vid , uint32_t portset){
	uint32_t o_portset;
	if( raw_vlan_read_member(vid,&o_portset)<0)
		return -1;
	if( (o_portset & portset)==portset )
		/* no change */
		return 0;
	if( raw_vlan_write_member(vid,o_portset|portset)<0)
		return -1;
	return 0;
}

int raw_vlan_rm_ports(int vid , uint32_t portset){
	uint32_t o_portset;
	if( raw_vlan_read_member(vid,&o_portset)<0)
		return -1;
	if( (o_portset & portset)==0 )
		/* no change */
		return 0;
	if( raw_vlan_write_member(vid,o_portset&(~portset))<0)
		return -1;
	return 0;
}


