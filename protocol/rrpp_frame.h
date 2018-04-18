#ifndef __RRPP_FRAME_H
#define __RRPP_FRAME_H


#include "types.h"
#include "vlan.h"


struct eth_hdr {
    uint8_t dst[6];             //目的MAC
    uint8_t src[6];             //源MAC
} __attr_packed__ ;


/* 没什么用,rrpp标准帧格式 */
struct pre_rrpp_field {
    uint16_t frame_length;      //以太网帧长度
    uint16_t dsap_ssap;
    uint8_t  control;
    uint8_t  oui[3];
    uint16_t const_0x00bb;
    uint16_t const_0x0b99;
} __attr_packed__ ;


/* rrpp帧 */
struct rrpp_field {
    uint16_t rrpp_length;

    uint8_t  rrpp_ver;
    uint8_t  rrpp_type;
    uint16_t domain_id;
    uint16_t ring_id;
    uint16_t const_0x0000;
    uint8_t  sys_mac_addr[6];
    uint16_t hello_timer;
    uint16_t fail_timer;
    uint8_t  const_0x00;
    uint8_t  level;
    uint16_t hello_seq;
    uint16_t const_wtf_0x0000;

    uint8_t  reserved_tail[36];
} __attr_packed__ ;


/* complete rrpp packet(without vlan and special tag) */
/* 完整rrpp帧，不包含vlan和special tag字段 */
struct rrpp_packet {
    struct eth_hdr hdr;
    struct pre_rrpp_field pre_rrpp;
    struct rrpp_field rrpp;
} __attr_packed__;

/* 包含vlan字段的rrpp帧 */
struct rrpp_vlan_packet {
    struct eth_hdr hdr;
    struct vlan_field vlan;
    struct pre_rrpp_field pre_rrpp;
    struct rrpp_field rrpp;
} __attr_packed__;

/* 接收的special tag，用以判断接收端口 */
struct special_tag_rx {
    uint16_t tag;
    uint32_t __reserved: 27;
    uint32_t src_port: 5;
} __attr_packed__;

/* 包含vlan字段+special tag的rrpp帧 */
struct rrpp_special_vlan_packet {
    struct eth_hdr hdr;
    struct special_tag_rx special;
    struct vlan_field vlan;
    struct pre_rrpp_field pre_rrpp;
    struct rrpp_field rrpp;
} __attr_packed__ ;


#define  RRPP_DST_MAC {0x01,0x80,0xc2,0x00,0x00,0x00}
/* rrpp mac地址byte5范围 0x11~0x1F */
#define  RRPP_DST_MAC_B5_BEGIN  0x11
#define  RRPP_DST_MAC_B5_END    0x1F

/* rrpp packet types defination */
#define  RRPP_TYPE_NOP                   0
#define  RRPP_TYPE_HELLO                 0x11
#define  RRPP_TYPE_LINK_UP               0x12
#define  RRPP_TYPE_LINK_DOWN             0x13
#define  RRPP_TYPE_COMMON_FLUSH_FDB      0x14
#define  RRPP_TYPE_COMPLETE_FLUSH_FDB    0x15


int is_rrpp_dst_mac (const uint8_t * mac);
void fill_rrpp_hello_frame (
    struct rrpp_vlan_packet * frame ,
    const uint8_t * local_mac ,
    uint16_t vid,
    uint16_t did,
    uint16_t rid );
void fill_rrpp_linkup_frame (
    struct rrpp_vlan_packet * frame ,
    const uint8_t * local_mac ,
    uint16_t vid,
    uint16_t did,
    uint16_t rid );
void fill_rrpp_linkdown_frame (
    struct rrpp_vlan_packet * frame ,
    const uint8_t * local_mac ,
    uint16_t vid,
    uint16_t did,
    uint16_t rid );
void fill_rrpp_common_flush_fdb_frame (
    struct rrpp_vlan_packet * frame ,
    const uint8_t * local_mac ,
    uint16_t vid,
    uint16_t did,
    uint16_t rid );
void fill_rrpp_complete_flush_fdb_frame (
    struct rrpp_vlan_packet * frame ,
    const uint8_t * local_mac ,
    uint16_t vid,
    uint16_t did,
    uint16_t rid );
const void * cp_remove_special_tag (struct rrpp_vlan_packet * sendbuf,
                                    const char * buf);




#endif

