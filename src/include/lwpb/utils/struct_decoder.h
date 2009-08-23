/**
 * @file struct_decoder.h
 * 
 * Lightweight protocol buffers struct decoder interface.
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

#ifndef __LWPB_UTILS_STRUCT_DECODER_H__
#define __LWPB_UTILS_STRUCT_DECODER_H__

#include <lwpb/lwpb.h>


/* Forward declaration */
struct lwpb_struct_decoder;

/**
 * This handler is called when the decoder encountered a new message.
 * @param decoder Decoder
 * @param msg_desc Message descriptor
 * @param arg User argument
 */
typedef void (*lwpb_struct_decoder_msg_start_handler_t)
    (struct lwpb_struct_decoder *decoder,
     const struct lwpb_msg_desc *msg_desc, void *arg);

/**
 * This handler is called when the decoder finished decoding a message.
 * @param decoder Decoder
 * @param msg_desc Message descriptor
 * @param arg User argument
 */
typedef void (*lwpb_struct_decoder_msg_end_handler_t)
    (struct lwpb_struct_decoder *decoder,
     const struct lwpb_msg_desc *msg_desc, void *arg);

/**
 * This handler is called when the decoder has decoded a field.
 * @param decoder Decoder
 * @param msg_desc Message descriptor of the message containing the field
 * @param field_desc Field descriptor
 * @param value Field value
 * @param arg User argument
 */
typedef void (*lwpb_struct_decoder_field_handler_t)
    (struct lwpb_struct_decoder *decoder,
     const struct lwpb_msg_desc *msg_desc,
     const struct lwpb_field_desc *field_desc,
     union lwpb_value *value, void *arg);


struct lwpb_struct_decoder_stack_frame {
    const struct lwpb_struct_map *map;
    void *base;
    const struct lwpb_struct_map_field *last_field;
    int field_index;
};

/** Protocol buffer struct decoder */
struct lwpb_struct_decoder {
    struct lwpb_decoder decoder;
    void *arg;
    lwpb_struct_decoder_msg_start_handler_t msg_start_handler;
    lwpb_struct_decoder_msg_end_handler_t msg_end_handler;
    lwpb_struct_decoder_field_handler_t field_handler;
    struct lwpb_struct_decoder_stack_frame stack[LWPB_MAX_DEPTH];
    int depth;
};

void lwpb_struct_decoder_init(struct lwpb_struct_decoder *sdecoder);

void lwpb_struct_decoder_arg(struct lwpb_struct_decoder *sdecoder, void *arg);

void lwpb_struct_decoder_msg_handler(struct lwpb_struct_decoder *sdecoder,
                                     lwpb_struct_decoder_msg_start_handler_t msg_start_handler,
                                     lwpb_struct_decoder_msg_end_handler_t msg_end_handler);

void lwpb_struct_decoder_field_handler(struct lwpb_struct_decoder *sdecoder,
                                       lwpb_struct_decoder_field_handler_t field_handler);

lwpb_err_t lwpb_struct_decoder_decode(struct lwpb_struct_decoder *sdecoder,
                                      const struct lwpb_struct_map *struct_map,
                                      void *struct_base,
                                      void *data, size_t len, size_t *used);


#endif // __LWPB_UTILS_STRUCT_DECODER_H__
