project(RRPP)
set(src kit.c vlan.c vlan_raw.c link_status.c raw_socket.c sendto_port.c thread.c rrpp_regs.c)
add_library(kit STATIC ${src})
