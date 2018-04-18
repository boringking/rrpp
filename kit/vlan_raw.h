#ifndef  __VLAN_RAW_H
#define  __VLAN_RAW_H



#define PBIT(port)  (1ul<<((port)-1))


int raw_vlan_add_ports (int vid , uint32_t portset);
int raw_vlan_rm_ports (int vid , uint32_t portset);




#endif

