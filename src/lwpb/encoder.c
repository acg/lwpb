/**
 * @file encoder.c
 * 
 * Implementation of the protocol buffers encoder.
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lwpb/lwpb.h>

#include "private.h"

// Encoder utilities

static void encode_varint(uint64_t varint, char *buf, char **end)
{
    while (varint) {
        if (varint > 127) {
            *buf = 0x80 | (varint & 0x7F);
        } else {
            *buf = (varint & 0x7F);
        }
        varint >>= 7;
        buf++;
    }
    *end = buf;
}

static void encode_32bit(uint32_t value, char *buf, char **end)
{
    buf[0] = (value) & 0xff;
    buf[1] = (value >> 8) & 0xff;
    buf[2] = (value >> 16) & 0xff;
    buf[3] = (value >> 24) & 0xff;
    *end = buf + 4;
}

// Encoder

/**
 * Initializes the encoder.
 * @param encoder Encoder
 */
void lwpb_encoder_init(struct lwpb_encoder *encoder)
{
}

void lwpb_encoder_start(struct lwpb_encoder *encoder, void *data, size_t len)
{
    encoder->data = data;
    encoder->len = len;
    encoder->depth = 0;
    encoder->stack[0].buf = data;
    encoder->stack[0].buf_end = &encoder->stack[0].buf[len];
}

size_t lwpb_encoder_finish(struct lwpb_encoder *encoder)
{
    return encoder->stack[0].buf - (char *) encoder->data;
}

void lwpb_encoder_msg_start(struct lwpb_encoder *encoder,
                            const struct lwpb_msg_desc *msg_desc)
{
    struct lwpb_encoder_stack_entry *entry;
    
    encoder->depth++;
    LWPB_ASSERT(encoder->depth < LWPB_MAX_DEPTH, "Message stacking too deep");
    
    entry = &encoder->stack[encoder->depth - 1];
    entry->msg_desc = msg_desc;
    
}

void lwpb_encoder_msg_end(struct lwpb_encoder *encoder)
{
    struct lwpb_encoder_stack_entry *entry;
    
    encoder->depth--;
    LWPB_ASSERT(encoder->depth >= 0, "Message stacking too shallow");
    
    entry = &encoder->stack[encoder->depth - 1];
}

lwpb_err_t lwpb_encoder_add_field(struct lwpb_encoder *encoder,
                                  const struct lwpb_field_desc *field_desc,
                                  union lwpb_value *value)
{
    struct lwpb_encoder_stack_entry *entry;
    int i;
    uint64_t key;
    enum wire_type wire_type;
    union wire_value wire_value;
    
    LWPB_ASSERT(encoder->depth > 0, "Fields can only be added inside a message");
    
    entry = &encoder->stack[encoder->depth - 1];
    
    // Check that field belongs to the current message
    for (i = 0; i < entry->msg_desc->num_fields; i++)
        if (field_desc == &entry->msg_desc->fields[i])
            break;
    if (i == entry->msg_desc->num_fields)
        return LWPB_ERR_UNKNOWN_FIELD;
    
    // Encode wire value
    switch (field_desc->opts.typ) {
    case LWPB_DOUBLE:
        wire_type = WT_64BIT;
        wire_value.int64 = *((uint64_t *) &value->double_);
        break;
    case LWPB_FLOAT:
        wire_type = WT_32BIT;
        wire_value.int32 = *((uint32_t *) &value->float_);
        break;
    case LWPB_INT32:
    case LWPB_UINT32:
    case LWPB_SINT32:
        wire_type = WT_VARINT;
        wire_value.varint = value->int32;
        break;
    case LWPB_INT64:
    case LWPB_UINT64:
    case LWPB_SINT64:
        wire_type = WT_VARINT;
        wire_value.varint = value->int64;
        break;
    case LWPB_FIXED32:
        wire_type = WT_32BIT;
        wire_value.int32 = value->uint32;
        break;
    case LWPB_FIXED64:
        wire_type = WT_64BIT;
        wire_value.int32 = value->uint64;
        break;
    case LWPB_SFIXED32:
        wire_type = WT_32BIT;
        wire_value.int32 = value->int32;
        break;
    case LWPB_SFIXED64:
        wire_type = WT_64BIT;
        wire_value.int32 = value->int64;
        break;
    case LWPB_BOOL:
        wire_type = WT_VARINT;
        wire_value.varint = value->bool;
        break;
    case LWPB_STRING:
        wire_type = WT_STRING;
        wire_value.string.data = value->string.str;
        wire_value.string.len = value->string.len;
        break;
    case LWPB_BYTES:
        wire_type = WT_STRING;
        wire_value.string.data = value->bytes.data;
        wire_value.string.len = value->bytes.len;
        break;
    case LWPB_ENUM:
        wire_type = WT_VARINT;
        wire_value.varint = value->enum_;
        break;
    case LWPB_MESSAGE:
        break;
    }
    
    key = wire_type | (field_desc->number << 3);
    encode_varint(key, entry->buf, &entry->buf);
    
    switch (wire_type) {
    case WT_VARINT:
        encode_varint(wire_value.varint, entry->buf, &entry->buf);
        break;
    case WT_64BIT:
        encode_32bit(wire_value.int64 & 0xffffffff, entry->buf, &entry->buf);
        encode_32bit(wire_value.int64 >> 32, entry->buf, &entry->buf);
        break;
    case WT_STRING:
        encode_varint(wire_value.string.len, entry->buf, &entry->buf);
        memcpy(entry->buf, wire_value.string.data, wire_value.string.len);
        entry->buf += wire_value.string.len;
        break;
    case WT_32BIT:
        encode_32bit(wire_value.int32, entry->buf, &entry->buf);
        break;
    default:
        LWPB_ASSERT(1, "Unknown wire type");
        break;
    }
    
    return LWPB_ERR_OK;
}

lwpb_err_t lwpb_encoder_add_double(struct lwpb_encoder *encoder,
                                   const struct lwpb_field_desc *field_desc,
                                   double double_)
{
    union lwpb_value value;
    value.double_ = double_;
    return lwpb_encoder_add_field(encoder, field_desc, &value);
}

lwpb_err_t lwpb_encoder_add_float(struct lwpb_encoder *encoder,
                                  const struct lwpb_field_desc *field_desc,
                                  float float_)
{
    union lwpb_value value;
    value.float_ = float_;
    return lwpb_encoder_add_field(encoder, field_desc, &value);
}

lwpb_err_t lwpb_encoder_add_int32(struct lwpb_encoder *encoder,
                                  const struct lwpb_field_desc *field_desc,
                                  int32_t int32)
{
    union lwpb_value value;
    value.int32 = int32;
    return lwpb_encoder_add_field(encoder, field_desc, &value);
}

lwpb_err_t lwpb_encoder_add_uint32(struct lwpb_encoder *encoder,
                                   const struct lwpb_field_desc *field_desc,
                                   uint32_t uint32)
{
    union lwpb_value value;
    value.uint32 = uint32;
    return lwpb_encoder_add_field(encoder, field_desc, &value);
}

lwpb_err_t lwpb_encoder_add_int64(struct lwpb_encoder *encoder,
                                  const struct lwpb_field_desc *field_desc,
                                  int64_t int64)
{
    union lwpb_value value;
    value.int64 = int64;
    return lwpb_encoder_add_field(encoder, field_desc, &value);
}

lwpb_err_t lwpb_encoder_add_uint64(struct lwpb_encoder *encoder,
                                   const struct lwpb_field_desc *field_desc,
                                   uint64_t uint64)
{
    union lwpb_value value;
    value.uint64 = uint64;
    return lwpb_encoder_add_field(encoder, field_desc, &value);
}

lwpb_err_t lwpb_encoder_add_bool(struct lwpb_encoder *encoder,
                                 const struct lwpb_field_desc *field_desc,
                                 int bool)
{
    union lwpb_value value;
    value.bool = bool;
    return lwpb_encoder_add_field(encoder, field_desc, &value);
}

lwpb_err_t lwpb_encoder_add_string(struct lwpb_encoder *encoder,
                                   const struct lwpb_field_desc *field_desc,
                                   char *str)
{
    union lwpb_value value;
    value.string.str = str;
    value.string.len = strlen(str);
    return lwpb_encoder_add_field(encoder, field_desc, &value);
}

lwpb_err_t lwpb_encoder_add_bytes(struct lwpb_encoder *encoder,
                                  const struct lwpb_field_desc *field_desc,
                                  uint8_t *data, size_t len)
{
    union lwpb_value value;
    value.string.str = (char *) data;
    value.string.len = len;
    return lwpb_encoder_add_field(encoder, field_desc, &value);
}

lwpb_err_t lwpb_encoder_add_enum(struct lwpb_encoder *encoder,
                                 const struct lwpb_field_desc *field_desc,
                                 int enum_)
{
    union lwpb_value value;
    value.enum_ = enum_;
    return lwpb_encoder_add_field(encoder, field_desc, &value);
}
