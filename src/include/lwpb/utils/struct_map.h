/**
 * @file struct_map.h
 * 
 * Lightweight protocol buffers struct map interface.
 * 
 * Copyright 2009 Simon Kallweit
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 *     
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __LWPB_UTILS_STRUCT_MAP_H__
#define __LWPB_UTILS_STRUCT_MAP_H__

#include <lwpb/lwpb.h>


#define LWPB_STRUCT_MAP_BEGIN(_name_, _msg_desc_, _struct_)                 \
const struct lwpb_struct_map _name_ = {                                     \
    .msg_desc = _msg_desc_,                                                 \
    .struct_size = sizeof(_struct_),                                        \
    .fields = {

#define LWPB_STRUCT_MAP_DOUBLE(_field_desc_, _struct_, _field_, _count_)    \
    LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, sizeof(double), _count_)

#define LWPB_STRUCT_MAP_FLOAT(_field_desc_, _struct_, _field_, _count_)     \
    LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, sizeof(float)), _count_)

#define LWPB_STRUCT_MAP_INT32(_field_desc_, _struct_, _field_, _count_)     \
    LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, sizeof(s32_t), _count_)

#define LWPB_STRUCT_MAP_UINT32(_field_desc_, _struct_, _field_, _count_)    \
    LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, sizeof(u32_t), _count_)

#define LWPB_STRUCT_MAP_INT64(_field_desc_, _struct_, _field_, _count_)     \
    LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, sizeof(s64_t), _count_)

#define LWPB_STRUCT_MAP_UINT64(_field_desc_, _struct_, _field_, _count_)    \
    LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, sizeof(u64_t), _count_)

#define LWPB_STRUCT_MAP_BOOL(_field_desc_, _struct_, _field_, _count_)  \
    LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, sizeof(lwpb_bool_t), _count_)

#define LWPB_STRUCT_MAP_ENUM(_field_desc_, _struct_, _field_, _count_)  \
    LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, sizeof(lwpb_enum_t), _count_)

#define LWPB_STRUCT_MAP_STRING(_field_desc_, _struct_, _field_, _len_, _count_) \
    LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, _len_, _count_)
    
#define LWPB_STRUCT_MAP_BYTES(_field_desc_, _struct_, _field_, _len_, _count_) \
    LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, _len_, _count_)

#define LWPB_STRUCT_MAP_MESSAGE(_field_desc_, _struct_, _field_, _struct_map_, _count_) \
    LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, (size_t) (_struct_map_), _count_)

    
#define LWPB_STRUCT_MAP_FIELD(_field_desc_, _struct_, _field_, _len_, _count_) \
        {                                                                   \
            .field_desc = _field_desc_,                                     \
            .ofs = (unsigned int) &((_struct_ *) 0)->_field_,               \
            .len = _len_,                                                   \
            .count = _count_,                                               \
        },

#define LWPB_STRUCT_MAP_END                                                 \
        {                                                                   \
            .field_desc = NULL,                                             \
        },                                                                  \
    },                                                                      \
};


struct lwpb_struct_map_field {
    const struct lwpb_field_desc *field_desc;
    unsigned int ofs;
    size_t len;
    size_t count;
};

struct lwpb_struct_map {
    const struct lwpb_msg_desc *msg_desc;
    size_t struct_size;
    const struct lwpb_struct_map_field fields[];
};

#endif // __LWPB_UTILS_STRUCT_MAP_H__
