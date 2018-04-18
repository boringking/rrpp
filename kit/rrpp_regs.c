#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
//#include "typedef.h"
#include "ip1829.h"
#include "ip1829lib.h"
//#include "ip1829_reg_test.h"
#include "rrpp_regs.h"
#include "rrpp_frame.h"


int rrpp_regs_config (void) {
    uint16_t rd_reg;

    /* specify special tag value */
    if (singleRegSetting (ID_COMMON_MISC_SET_REG, 0x0C, 0x04,
                          SPECIAL_TAG_VAL) < 0) {
        goto WRITE_FAILED;
    }

    /* enable special tag function and CPU port(P29)*/
    if (singleRegGetting (ID_COMMON_MISC_GET_REG, 0x0C, 0x03, &rd_reg) < 0) {
        goto READ_FAILED;
    }

    if (singleRegSetting (ID_COMMON_MISC_SET_REG, 0x0C, 0x03,
                          rd_reg | (3ul << 0) ) < 0) {
        goto WRITE_FAILED;
    }

    /* disable BPDU per Port setting */
    if (singleRegGetting (ID_COMMON_MISC_GET_REG, 0x00, 0x33, &rd_reg) < 0) {
        goto READ_FAILED;
    }

    if (singleRegSetting (ID_COMMON_MISC_SET_REG, 0x00, 0x33,
                          rd_reg & (~ (1ul << 8) ) ) < 0) {
        goto WRITE_FAILED;
    }

    /* enable group 1 dst-macs */
    // bit[9:8]:for Group 0(01-80-C2-00-00-05,06,09~0C,0F)
    //bit[13:12]:for Group 1(01-80-C2-00-00-11~1F)
    if (singleRegGetting (ID_COMMON_MISC_GET_REG, 0x00, 0x06, &rd_reg) < 0) {
        goto READ_FAILED;
    }

    rd_reg &= ~ (3ul << 12);
    rd_reg |= 2ul << 12;  /* config as TO CPU */

    if (singleRegSetting (ID_COMMON_MISC_SET_REG, 0x00, 0x06, rd_reg) < 0) {
        goto WRITE_FAILED;
    }

    /* success */
    printf ("rrpp registers initialization succeed...\n");
    return 0;

READ_FAILED:
    printf ("read register FAILED!\n");
    return -1;
WRITE_FAILED:
    printf ("write register FAILED!\n");
    return -1;
}


