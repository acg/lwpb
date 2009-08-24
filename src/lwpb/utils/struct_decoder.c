/** @file struct_decoder.c
 * 
 * Implementation of the protocol buffers struct decoder.
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

#include <lwpb/lwpb.h>

#include "private.h"


static const struct lwpb_struct_map_field *find_map_field(
        const struct lwpb_struct_map *map,
        const struct lwpb_field_desc *field_desc)
{
    const struct lwpb_struct_map_field *field;
    
    for (field = map->fields; field->field_desc; field++)
        if (field->field_desc == field_desc)
            return field;
}

#define FIELD_BASE(_field_, _base_, _index_) \
    ((_base_) + (_field_)->ofs + ((_field_)->len * (_index_)))

static void unpack_field(struct lwpb_struct_decoder *sdecoder,
                         const struct lwpb_struct_map_field *field,
                         union lwpb_value *value)
{
    size_t len;
    struct lwpb_struct_decoder_stack_frame *frame;
    
    frame = &sdecoder->stack[sdecoder->depth];
    
    // Reset field index if a new field is encountered
    if (field != frame->last_field)
        frame->field_index = 0;
    frame->last_field = field;
    
    if (field->count >= field->count)

    switch (field->field_desc->opts.typ) {
    case LWPB_DOUBLE:
        LWPB_ASSERT(field->len == sizeof(double), "Field type mismatch");
        *((double *) FIELD_BASE(field, frame->base, frame->field_index)) = value->double_;
        frame->field_index++;
        break;
    case LWPB_FLOAT:
        LWPB_ASSERT(field->len == sizeof(float), "Field type mismatch");
        *((float *) FIELD_BASE(field, frame->base, frame->field_index)) = value->float_;
        frame->field_index++;
        break;
    case LWPB_INT32:
    case LWPB_SINT32:
    case LWPB_SFIXED32:
        LWPB_ASSERT(field->len == sizeof(s32_t), "Field type mismatch");
        *((s32_t *) FIELD_BASE(field, frame->base, frame->field_index)) = value->int32;
        frame->field_index++;
        break;
    case LWPB_UINT32:
    case LWPB_FIXED32:
        LWPB_ASSERT(field->len == sizeof(u32_t), "Field type mismatch");
        *((u32_t *) FIELD_BASE(field, frame->base, frame->field_index)) = value->uint32;
        frame->field_index++;
        break;
    case LWPB_INT64:
    case LWPB_SINT64:
    case LWPB_SFIXED64:
        LWPB_ASSERT(field->len == sizeof(s64_t), "Field type mismatch");
        *((s64_t *) FIELD_BASE(field, frame->base, frame->field_index)) = value->int64;
        frame->field_index++;
        break;
    case LWPB_UINT64:
    case LWPB_FIXED64:
        LWPB_ASSERT(field->len == sizeof(u64_t), "Field type mismatch");
        *((u64_t *) FIELD_BASE(field, frame->base, frame->field_index)) = value->uint64;
        frame->field_index++;
        break;
    case LWPB_BOOL:
        LWPB_ASSERT(field->len == sizeof(lwpb_bool_t), "Field type mismatch");
        *((lwpb_bool_t *) FIELD_BASE(field, frame->base, frame->field_index)) = value->bool;
        frame->field_index++;
        break;
    case LWPB_ENUM:
        LWPB_ASSERT(field->len == sizeof(lwpb_enum_t), "Field type mismatch");
        *((lwpb_enum_t *) FIELD_BASE(field, frame->base, frame->field_index)) = value->enum_;
        frame->field_index++;
        break;
    case LWPB_STRING:
        len = field->len < value->string.len + 1 ? field->len : value->string.len + 1;
        LWPB_MEMCPY(FIELD_BASE(field, frame->base, frame->field_index), value->string.str, len);
        ((char *) FIELD_BASE(field, frame->base, frame->field_index))[len - 1] = '\0';
        frame->field_index++;
        break;
    case LWPB_BYTES:
        len = field->len < value->bytes.len ? field->len : value->bytes.len;
        LWPB_MEMCPY(FIELD_BASE(field, frame->base, frame->field_index), value->bytes.data, len);
        frame->field_index++;
        break;
    case LWPB_MESSAGE:
        LWPB_DIAG_PRINTF("submessage\n");
        break;
    }
    
}

static void sdecoder_msg_start_handler(struct lwpb_decoder *decoder,
                                    const struct lwpb_msg_desc *msg_desc,
                                    void *arg)
{
    struct lwpb_struct_decoder *sdecoder = arg;
    struct lwpb_struct_decoder_stack_frame *frame, *last_frame;

    LWPB_DIAG_PRINTF("msg start\n");

    sdecoder->depth++;
    frame = &sdecoder->stack[sdecoder->depth];
    
    if (sdecoder->depth > 0) {
        last_frame = &sdecoder->stack[sdecoder->depth - 1];
        frame->map = (const struct lwpb_struct_map *) last_frame->last_field->len;
        frame->base = last_frame->base + last_frame->last_field->ofs +
            (frame->map->struct_size * last_frame->field_index);
        frame->last_field = NULL;
        frame->field_index = 0;
        last_frame->field_index++;
    }
    
    LWPB_ASSERT(frame->map->msg_desc == msg_desc, "Message type mismatch");
    
    if (sdecoder->msg_start_handler)
        sdecoder->msg_start_handler(sdecoder, msg_desc, sdecoder->arg);
}

static void sdecoder_msg_end_handler(struct lwpb_decoder *decoder,
                                  const struct lwpb_msg_desc *msg_desc,
                                  void *arg)
{
    struct lwpb_struct_decoder *sdecoder = arg;
    struct lwpb_struct_decoder_stack_frame *frame;
    
    LWPB_DIAG_PRINTF("msg end\n");
    
    sdecoder->depth--;
    frame = &sdecoder->stack[sdecoder->depth];

    if (sdecoder->msg_end_handler)
        sdecoder->msg_end_handler(sdecoder, msg_desc, sdecoder->arg);
}

static void sdecoder_field_handler(struct lwpb_decoder *decoder,
                                const struct lwpb_msg_desc *msg_desc,
                                const struct lwpb_field_desc *field_desc,
                                union lwpb_value *value, void *arg)
{
    struct lwpb_struct_decoder *sdecoder = arg;
    struct lwpb_struct_decoder_stack_frame *frame = &sdecoder->stack[sdecoder->depth];
    const struct lwpb_struct_map_field *field;
    
    field = find_map_field(frame->map, field_desc);
    if (field)
        unpack_field(sdecoder, field, value);

    if (sdecoder->field_handler)
        sdecoder->field_handler(sdecoder, msg_desc, field_desc, value, sdecoder->arg);
}



// Struct decoder

/**
 * Initializes the struct decoder.
 * @param sdecoder Struct decoder
 */
void lwpb_struct_decoder_init(struct lwpb_struct_decoder *sdecoder)
{
    // Initialize decoder
    lwpb_decoder_init(&sdecoder->decoder);
    lwpb_decoder_arg(&sdecoder->decoder, sdecoder);
    lwpb_decoder_msg_handler(&sdecoder->decoder, 
            sdecoder_msg_start_handler, sdecoder_msg_end_handler);
    lwpb_decoder_field_handler(&sdecoder->decoder,
            sdecoder_field_handler);
    
    // Initialize internals
    sdecoder->arg = NULL;
    sdecoder->msg_start_handler = NULL;
    sdecoder->msg_end_handler = NULL;
    sdecoder->field_handler = NULL;
}

/**
 * Sets the user argument to be passed back with the handlers.
 * @param sdecoder Struct decoder
 * @param arg User argument
 */
void lwpb_struct_decoder_arg(struct lwpb_struct_decoder *sdecoder, void *arg)
{
    sdecoder->arg = arg;
}

/**
 * Sets the message start and end handlers.
 * @param sdecoder Struct decoder
 * @param msg_start_handler Message start handler
 * @param msg_end_handler Message end handler
 */
void lwpb_struct_decoder_msg_handler(struct lwpb_struct_decoder *sdecoder,
                                     lwpb_struct_decoder_msg_start_handler_t msg_start_handler,
                                     lwpb_struct_decoder_msg_end_handler_t msg_end_handler)
{
    sdecoder->msg_start_handler = msg_start_handler;
    sdecoder->msg_end_handler = msg_end_handler;
}

/**
 * Sets the field handler.
 * @param sdecoder Struct decoder
 * @param field_handler Field handler
 */
void lwpb_struct_decoder_field_handler(struct lwpb_struct_decoder *sdecoder,
                                       lwpb_struct_decoder_field_handler_t field_handler)
{
    sdecoder->field_handler = field_handler;
}

/**
 * Decodes a protocol buffer into a struct.
 * @param sdecoder Struct decoder
 * @param struct_map Struct map used for decoding
 * @param struct_base Base of the struct to decode into
 * @param data Data to decode
 * @param len Length of data to decode
 * @param used Returns the number of decoded bytes when not NULL.
 * @return Returns LWPB_ERR_OK when data was successfully decoded.
 */
lwpb_err_t lwpb_struct_decoder_decode(struct lwpb_struct_decoder *sdecoder,
                                      const struct lwpb_struct_map *struct_map,
                                      void *struct_base,
                                      void *data, size_t len, size_t *used)
{
    sdecoder->depth = -1;
    sdecoder->stack[0].map = struct_map;
    sdecoder->stack[0].base = struct_base;
    sdecoder->stack[0].last_field = NULL;
    sdecoder->stack[0].field_index = 0;
    
    return lwpb_decoder_decode(&sdecoder->decoder, struct_map->msg_desc, data, len, used);
}
