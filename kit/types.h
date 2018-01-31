#ifndef  __TYPES_H
#define  __TYPES_H


#include <stdbool.h>

typedef signed char   int8_t;
typedef unsigned char uint8_t;
typedef short   int16_t;
typedef unsigned short uint16_t;
typedef int   int32_t;
typedef unsigned int uint32_t;

typedef unsigned short vid_t;


#define  __attr_packed__   __attribute__((packed))


#define  ARRAY_SIZE(x)   (sizeof(x)/sizeof((x)[0]))

#define  CLI_PATH    "/home/user/callback/"

#endif

