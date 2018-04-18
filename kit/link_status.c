#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "ip1829.h"
#include "ip1829lib.h"
#include "link_status.h"


/*
读取端口link状态
返回值:
    true:  link-up
    false: link-down
*/
bool read_port_link_status (int port) {
    struct LinkStatusSetting port_status_buf = {
        .cmdid = ID_COMMON_GET_PORT_LINK_STATUS,
        .port  = port,
    };

    //port_status_buf.port = port;
    if ( singleLinkStatusSetting (ID_COMMON_GET_PORT_LINK_STATUS ,
                                  &port_status_buf) ) {
        fprintf (stderr, "singleLinkStatusSetting failed\n");
        return -1;
    }

    return port_status_buf.link ? true : false;
}


