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
#include "raw_socket.h"
//#include "ip1829.h"
//#include "ip1829_reg_read.h"
//#include "rrpp_frame.h"
//#include "pf_packet_recv.h"
//#include "rrpp_regs_config.h"





/* initialize raw_socket socket */
int raw_socket_init (struct raw_socket * raw_sock, const char * nic_name ) {
    // socket
    // htons(ETH_P_ALL)只对recv起作用
    raw_sock->fd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL) );

    if (raw_sock->fd < 0) {
        perror ("socket");
        return -1;
    }

    // get if index
    struct ifreq if_struct;
    strcpy (if_struct.ifr_name , nic_name);
    ioctl (raw_sock->fd , SIOCGIFINDEX, &if_struct);
    printf ("ifindex of nic \"%s\" equals %d\n" , nic_name, if_struct.ifr_ifindex);

    // fill sockaddr_ll struct
    memset (&raw_sock->addr, 0 , sizeof (raw_sock->addr) );
    raw_sock->addr.sll_halen = ETH_ALEN;
    raw_sock->addr.sll_ifindex = if_struct.ifr_ifindex;
    raw_sock->addr.sll_family = AF_PACKET;
    raw_sock->addr.sll_protocol = htons (ETH_P_ALL);

#if   1
    char cat_mac_addr[64];
    char mac_file_str[256];
    int mac_val[6];
    sprintf (mac_file_str , "/sys/class/net/%s/address" , nic_name);
    int cat_fd = open (mac_file_str , O_RDONLY);

    if (cat_fd < 0) {
        perror ("open mac file");
        return -1;
    }

    read (cat_fd , cat_mac_addr , sizeof (cat_mac_addr) );
    sscanf (cat_mac_addr , "%02x:%02x:%02x:%02x:%02x:%02x",
            mac_val,
            mac_val + 1,
            mac_val + 2,
            mac_val + 3,
            mac_val + 4,
            mac_val + 5);

    for (int i = 0 ; i < 6; ++i) {
        raw_sock->local_mac[i] = (uint8_t) mac_val[i];
    }

    printf ("local mac = ");

    for (int i = 0; i < 6; ++i) {
        if (5 == i) {
            printf ("%02x" , raw_sock->local_mac[i]);
        } else {
            printf ("%02x-" , raw_sock->local_mac[i]);
        }
    }

    printf ("\n");
#endif

    return 0;
}

/* 向指定端口发送报文(底层实现) */
int raw_socket_sendto (const struct raw_socket * raw_sock, const void * buf,
                       int len) {
    return
        sendto (raw_sock->fd, buf, len, 0, (struct sockaddr *) &raw_sock->addr,
                sizeof (struct sockaddr_ll) );
}

/* 接收报文 */
int raw_socket_recvfrom (const struct raw_socket * raw_sock, void * buf,
                         int len, struct sockaddr_ll * addr, socklen_t * addrlen) {
    return recvfrom (raw_sock->fd , buf, len, 0, (struct sockaddr * ) addr,
                     addrlen);
}


#if   0
static void __fill_rrpp_frame (struct rrpp_packet_without_vlan * frame) {
    printf ("sizeof rrpp packet without vlan field = %d\n" , sizeof (*frame) );
    // clear as 0
    memset (frame , 0 , sizeof (*frame) );
    // ethhdr field
    u8 dst[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff,};
    memcpy (frame->hdr.dst , dst , 6);
    memcpy (frame->hdr.src, local_mac , 6);

    // pre_rrpp_field
    frame->pre_rrpp.frame_length = htons (sizeof (struct pre_rrpp_field) + sizeof (
            struct rrpp_field) - 2);
    printf ("frame_length = %d\n" , frame->pre_rrpp.frame_length );
    frame->pre_rrpp.const_0x00bb = btols (0x00bb); // big-endian
    frame->pre_rrpp.const_0x0b99 = btols (0x0b99); // big-endian


    // rrpp field(不考虑大小端了，因为收发双方是相同架构)
    frame->rrpp.rrpp_length = sizeof (struct rrpp_field) - 2;
    printf ("rrpp_length = %d\n" , frame->rrpp.rrpp_length);
    frame->rrpp.domain_id = 1;
    frame->rrpp.ring_id = 1;
    memcpy (frame->rrpp.sys_mac_addr, local_mac , 6);
    frame->rrpp.hello_timer = 1;
    frame->rrpp.fail_timer = 3;
    frame->rrpp.hello_seq = 0x88;
#if  0

    for (int i = 0 ; i < sizeof (*frame); ++i) {
        printf ("%02x " , * ( ( (u8 *) frame) + i) );
    }

#endif
}


static void __fill_vlan_rrpp_frame (struct rrpp_packet_with_vlan * frame ,
                                    u16 vid) {
    printf ("sizeof rrpp packet with vlan field = %d\n" , sizeof (*frame) );
    // clear as 0
    memset (frame , 0 , sizeof (*frame) );
    // ethhdr field
    //u8 dst[6] = {0xff,0xff,0xff,0xff,0xff,0xff,};
    u8 dst[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x22,};
    //u8 dst[6]={0xBC,0x30,0x5B,0xCA,0x4E,0xA6};
    memcpy (frame->hdr.dst , dst , 6);
    memcpy (frame->hdr.src, local_mac , 6);

    // vlan field
    frame->vlan.tpid = htons (0x8100);
    frame->vlan.pri = 0;
    frame->vlan.cfi = 0; // std format
    frame->vlan.vid = vid; // 大端，直接赋值

    // pre_rrpp_field
    frame->pre_rrpp.frame_length = htons (sizeof (struct pre_rrpp_field) + sizeof (
            struct rrpp_field) - 2);
    printf ("frame_length = %d\n" , frame->pre_rrpp.frame_length );
    frame->pre_rrpp.const_0x00bb = btols (0x00bb); // big-endian
    frame->pre_rrpp.const_0x0b99 = btols (0x0b99); // big-endian

    // rrpp field
    frame->rrpp.rrpp_length = sizeof (struct rrpp_field) - 2;
    printf ("rrpp_length = %d\n" , frame->rrpp.rrpp_length);
    frame->rrpp.domain_id = 1;
    frame->rrpp.ring_id = 1;
    memcpy (frame->rrpp.sys_mac_addr, local_mac , 6);
    frame->rrpp.hello_timer = 1;
    frame->rrpp.fail_timer = 3;
    frame->rrpp.hello_seq = 0x88;
}


static void __fill_special_vlan_frame (struct special_vlan_packet * frame) {
    //printf("sizeof rrpp packet with vlan field = %d\n" , sizeof(*frame));
    // clear as 0
    memset (frame , 0 , sizeof (*frame) );
    // ethhdr field
    //u8 dst[6] = {0xff,0xff,0xff,0xff,0xff,0xff,};
    //u8 dst[6]={0xBC,0x30,0x5B,0xCA,0x4E,0xA6};
    u8 dst[6] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x11,};
    //u8 dst[6]={0x2C,0x06,0x23,0x00,0x00,0x01};
    memcpy (frame->hdr.dst , dst , 6);
    memcpy (frame->hdr.src, local_mac , 6);

    // special tag field
    frame->spc_tag.tag = SPECIAL_TAG_VAL;
    frame->spc_tag.pri = 0;
    frame->spc_tag.dfp_en = 1;
    //frame->spc_tag.dfp = 1ul<<10;
    frame->spc_tag.dfp = 1ul << 9;
    printf ("spc_tag = 0x%08x \r\n" ,
            * (u32 *) ( (short *) &frame->spc_tag + 1) );

    // vlan field
    frame->vlan.tpid = htons (0x8100);
    //  frame->vlan.tpid = htons(0x800);
    frame->vlan.pri = 0;
    frame->vlan.cfi = 0; // std format
    frame->vlan.vid = 10; // 大端，直接赋值

    frame->eth_type = htons (0x800);
}


static void __fill_test_udp_frame (char * buf , int len) {
    memset (buf, 0x00, len);
    //int j=0;
    // dst mac
    buf[0] = 0xff;
    buf[1] = 0xff;
    buf[2] = 0xff;
    buf[3] = 0xff;
    buf[4] = 0xff;
    buf[5] = 0xff;
    // src mac
    buf[6] = 0x00;
    buf[7] = 0x11;
    buf[8] = 0x22;
    buf[9] = 0x33;
    buf[10] = 0x00;
    buf[11] = 0x00;

    buf[12] = 0x08;
    buf[13] = 0x00; //protocal field
    buf[14] = 0x45;
    buf[15] = 0x00;
    buf[16] = 0x00;
    buf[17] = 0x25;
    buf[18] = 0x00;
    buf[19] = 0x00;
    buf[20] = 0x40;
    buf[21] = 0x00;
    buf[22] = 0x40;
    buf[23] = 0x11;
    buf[24] = 0x76;
    buf[25] = 0x4f;
    buf[26] = 0xc0;
    buf[27] = 0xa8;
    buf[28] = 0x03;
    buf[29] = 0xd1;
    buf[30] = 0xff;
    buf[31] = 0xff;
    buf[32] = 0xff;
    buf[33] = 0xff;
    buf[34] = 0x9e;
    buf[35] = 0xae;
    buf[36] = 0x1a;
    buf[37] = 0x0a;
    buf[38] = 0x00;
    buf[39] = 0x11;
    buf[40] = 0x78;
    buf[41] = 0xc5;
    buf[42] = 0x31;
    buf[43] = 0x32;
    buf[44] = 0x33;
    buf[45] = 0x34;
    buf[46] = 0x35;
    buf[47] = 0x36;
    buf[48] = 0x37;
    buf[49] = 0x38;
    buf[50] = 0x39;

}

int test_main (int argc, const char ** argv) {
    extern int recv_brdcst;

    if ( 2 == argc  ) {
        printf ("recv brdcst..\n");
        recv_brdcst = 1;
    } else {
        recv_brdcst = 0;
    }

#if  0
    int fd = 0;
    char buf[256] = {0};
    struct sockaddr_ll dest;
    int destlen = 0;
    int ret = 0;
    struct ifreq ifstruct;
    fd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL) );

    if (fd < 0) {
        printf ("ERR:socket was failed.\n");
        return -1;
    }

    destlen = sizeof (dest);
    strcpy (ifstruct.ifr_name, "eth0");
    ioctl (fd, SIOCGIFINDEX, &ifstruct);
    printf ("sll_ifindex = %d\n" , ifstruct.ifr_ifindex);

    memset ( (char *) &dest, 0x00, sizeof (dest) );
    dest.sll_ifindex = ifstruct.ifr_ifindex;
    dest.sll_family = AF_PACKET;
    unsigned char my_mac[] = {0x08, 0x00, 0x27, 0x93, 0x68, 0xe5};
    memcpy (dest.sll_addr, my_mac, 6);
    dest.sll_halen = ETH_ALEN;
    //dest.sll_protocol = htons(ETH_P_ALL);
#endif

#if   1

    if (rrpp_regs_config() < 0) {
        exit (1);
    }

#endif

    int fd;
    struct sockaddr_ll ll_addr;

    if ( (fd = raw_socket_init (&ll_addr , "eth0") ) < 0) {
        printf ("raw_socket_init: error\n");
        return -1;
    }

    //  char buf[128];
    //  __fill_test_udp_frame(buf,sizeof(buf));

    //  struct rrpp_packet_without_vlan rrpp_pkt;
    //  __fill_rrpp_frame(&rrpp_pkt);

    //  struct rrpp_packet_with_vlan rrpp_pkt;
    //  printf("vid = %d\n" , atoi(argv[1]));
    //  __fill_vlan_rrpp_frame(&rrpp_pkt , atoi(argv[1]));
#if  0
    struct rrpp_packet_qinq rrpp_pkt;
    printf ("cvid = %d, svid = %d\n" , atoi (argv[1]) , atoi (argv[2]) );
    __fill_qinq_rrpp_frame (&rrpp_pkt , atoi (argv[1]) , atoi (argv[2]) );
#endif
    struct special_vlan_packet pkt;
    __fill_special_vlan_frame (&pkt);

    // init receiver thread...
    recv_thread_init ( (void *) &fd);

    for (int i = 0 ; ; ++i) {
        int ret = raw_socket_sendto (fd, &pkt, sizeof (pkt),
                                     &ll_addr); //将构造的UDP帧发送到数据链路层

        if (ret < 0) {
            perror ("error");
            return -1;
        }

        printf ("%dth packet\n", i);
        sleep (1);
    }

    close (fd);
    return 0;
}

int main (int argc, const char ** argv) {
    return test_main (argc,  argv);
}

#endif
