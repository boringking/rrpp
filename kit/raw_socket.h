#ifndef  __PF_PACKET_H
#define  __PF_PACKET_H



#include <sys/types.h>          /* See NOTES */
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <unistd.h>




struct raw_socket {
    int    fd;
    struct sockaddr_ll addr;
    uint8_t local_mac[6];
};




int raw_socket_init (struct raw_socket * raw_sock, const char * nic_name );
int raw_socket_sendto (const struct raw_socket * raw_sock, const void * buf,
                       int len);
int raw_socket_recvfrom (const struct raw_socket * raw_sock, void * buf,
                         int len, struct sockaddr_ll * addr, socklen_t * addrlen);





#endif

