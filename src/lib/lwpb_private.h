
#ifndef __LWPB_PRIVATE_H__
#define __LWPB_PRIVATE_H__

#include <stdint.h>

/** Protocol buffer wire types */
enum wire_type {
    WT_VARINT = 0,
    WT_64BIT  = 1,
    WT_STRING = 2,
    WT_32BIT  = 5
};

/** Protocol buffer wire values */
union wire_value {
    uint64_t varint;
    uint64_t int64;
    struct {
        uint64_t len;
        void *data;
    } string;
    uint32_t int32;
};

#endif // __LWPB_PRIVATE_H__
