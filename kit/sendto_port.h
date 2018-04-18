#ifndef __SENDTO_PORT_H
#define __SENDTO_PORT_H


#include "types.h"
#include "rrpp_regs.h"
#include "raw_socket.h"

struct special_tag_tx {
    uint16_t tag;
    unsigned int pri: 3;
    unsigned int dfp_en: 1;
    unsigned int dfp: 28;
} __attr_packed__ ;


//#define SPECIAL_TAG_VAL  0x9126



int sendto_port (const struct raw_socket * raw_sock, int port, const void * src,
                 int len);


#endif

