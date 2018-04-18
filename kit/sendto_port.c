#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "sendto_port.h"
#include "raw_socket.h"
//#include "base_msg.h"



/* 构造带special tag的帧 */
static int fill_special_frame (void * dst , const void * src , int len ,
                               int port) {
    memcpy (dst , src , 12);
    struct special_tag_tx * p_tag = (struct special_tag_tx *) ( (char *) dst + 12);
    memset (p_tag , 0 , sizeof * p_tag);
    /* fill special tag */
    p_tag->tag = SPECIAL_TAG_VAL;
    p_tag->dfp_en = 1;
    p_tag->dfp = 1ul << (port - 1);

    memcpy (dst + 12 + sizeof (struct special_tag_tx) , src + 12 , len - 12);
    return len + sizeof (struct special_tag_tx);
}

int sendto_port (const struct raw_socket * raw_sock, int port, const void * src,
                 int len) {
    char buf[1024];
    int send_len = fill_special_frame (buf , src, len, port);
    //hexdump(buf, send_len);
    return raw_socket_sendto (raw_sock, buf, send_len);
}


