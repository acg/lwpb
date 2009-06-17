/**
 * @file decoder.c
 * 
 * Implementation of the protocol buffers decoder.
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

// Debug handlers

static int debug_indent;

static void debug_print_indent(void)
{
    int i;
    
    for (i = 0; i < debug_indent; i++)
        printf("  ");
}

static void debug_msg_start_handler(struct lwpb_decoder *decoder,
                                    const struct lwpb_msg_desc *msg_desc,
                                    void *arg)
{
    char *name;

#if LWPB_MESSAGE_NAMES
    name = msg_desc->name;
#else
    name = "<message>";
#endif
    
    debug_print_indent();
    printf("%s:\n", name);
    debug_indent++;
}

static void debug_msg_end_handler(struct lwpb_decoder *decoder,
                                  const struct lwpb_msg_desc *msg_desc,
                                  void *arg)
{
    debug_indent--;
}

static void debug_field_handler(struct lwpb_decoder *decoder,
                                const struct lwpb_msg_desc *msg_desc,
                                const struct lwpb_field_desc *field_desc,
                                union lwpb_value *value, void *arg)
{
    static char *typ_names[] = {
        "(double)",
        "(float)",
        "(int32)",
        "(int64)",
        "(uint32)",
        "(uint64)",
        "(sint32)",
        "(sint64)",
        "(fixed32)",
        "(fixed64)",
        "(sfixed32)",
        "(sfixed64)",
        "(bool)",
        "(string)",
        "(bytes)",
        "(message)",
        "(enum)",
    };
    
    char *name;
    
#if LWPB_FIELD_NAMES
    name = field_desc->name;
#else
    name = "<field>";
#endif
    
    debug_print_indent();
    printf("%-20s %-10s = ", name, typ_names[field_desc->opts.typ]);
    
    switch (field_desc->opts.typ) {
    case LWPB_DOUBLE:
        printf("%f", value->double_);
        break;
    case LWPB_FLOAT:
        printf("%f", value->float_);
        break;
    case LWPB_INT32:
    case LWPB_SINT32:
    case LWPB_SFIXED32:
        printf("%d", value->int32);
        break;
    case LWPB_INT64:
    case LWPB_SINT64:
    case LWPB_SFIXED64:
        printf("%lld", value->int64);
        break;
    case LWPB_UINT32:
    case LWPB_FIXED32:
        printf("%u", value->int32);
        break;
    case LWPB_UINT64:
    case LWPB_FIXED64:
        printf("%llu", value->int64);
        break;
    case LWPB_BOOL:
        printf("%s", value->bool ? "true" : "false");
        break;
    case LWPB_STRING:
        while (value->string.len--)
            printf("%c", *value->string.str++);
        break;
    case LWPB_BYTES:
        while (value->bytes.len--)
            printf("%02x ", *value->bytes.data++);
        break;
    case LWPB_ENUM:
        printf("%d", value->enum_);
        break;
    default:
        break;
    }
    
    printf("\n");
}

// Decoder utilities

static uint64_t decode_varint(char *buf, char **end)
{
    uint64_t ret = 0;
    
    int bitpos = 0;
    for (bitpos = 0; *buf & 0x80 && bitpos < 64; bitpos += 7, buf++)
        ret |= (*buf & 0x7F) << bitpos;
    ret |= (*buf & 0x7F) << bitpos;
    *end = buf + 1;
    return ret;
}

static uint32_t decode_32bit(char *buf, char **end)
{
    *end = buf + 4;
    return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

// Decoder

/**
 * Initializes the decoder.
 * @param decoder Decoder
 */
void lwpb_decoder_init(struct lwpb_decoder *decoder)
{
    decoder->arg = NULL;
    decoder->msg_start_handler = NULL;
    decoder->msg_end_handler = NULL;
    decoder->field_handler = NULL;
}

/**
 * Sets the user argument to be passed back with the handlers.
 * @param decoder Decoder
 * @param arg User argument
 */
void lwpb_decoder_arg(struct lwpb_decoder *decoder, void *arg)
{
    decoder->arg = arg;
}

/**
 * Sets the message start and end handlers.
 * @param decoder Decoder
 * @param msg_start_handler Message start handler
 * @param msg_end_handler Message end handler
 */
void lwpb_decoder_msg_handler(struct lwpb_decoder *decoder,
                            lwpb_decoder_msg_start_handler_t msg_start_handler,
                            lwpb_decoder_msg_end_handler_t msg_end_handler)
{
    decoder->msg_start_handler = msg_start_handler;
    decoder->msg_end_handler = msg_end_handler;
}

/**
 * Sets the field handler.
 * @param decoder Decoder
 * @param field_handler Field handler
 */
void lwpb_decoder_field_handler(struct lwpb_decoder *decoder,
                              lwpb_decoder_field_handler_t field_handler)
{
    decoder->field_handler = field_handler;
}

/**
 * Setups the decoder to use the verbose debug handlers which output the
 * message contents to the console.
 * @param decoder Decoder 
 */
void lwpb_decoder_use_debug_handlers(struct lwpb_decoder *decoder)
{
    lwpb_decoder_msg_handler(decoder,
                           debug_msg_start_handler, debug_msg_end_handler);
    lwpb_decoder_field_handler(decoder, debug_field_handler);
}

/**
 * Decodes a protocol buffer.
 * @param decoder Decoder
 * @param data Data to decode
 * @param len Length of data to decode
 * @param msg_desc Root message descriptor of the protocol buffer
 */
void lwpb_decoder_decode(struct lwpb_decoder *decoder,
                         void *data, size_t len,
                         const struct lwpb_msg_desc *msg_desc)
{
    int i;
    char *buf = data;
    char *buf_end = &buf[len];
    uint64_t key;
    int number;
    const struct lwpb_field_desc *field_desc = NULL;
    enum wire_type wire_type;
    union wire_value wire_value;
    union lwpb_value value;

    if (decoder->msg_start_handler)
        decoder->msg_start_handler(decoder, msg_desc, decoder->arg);
    
    while(buf < buf_end)
    {
        key = decode_varint(buf, &buf);
        number = key >> 3;
        wire_type = key & 0x07;
        
        // Find the field descriptor
        for (i = 0; i < msg_desc->num_fields; i++)
            if (msg_desc->fields[i].number == number) {
                field_desc = &msg_desc->fields[i];
                break;
            }

        // Decode field's wire value
        switch(wire_type) {
        case WT_VARINT:
            wire_value.varint = decode_varint(buf, &buf);
            break;
        case WT_64BIT:
            wire_value.int64 = decode_32bit(buf, &buf);
            wire_value.int64 |= (uint64_t) decode_32bit(buf, &buf) << 32;
            break;
        case WT_STRING:
            wire_value.string.len = decode_varint(buf, &buf);
            wire_value.string.data = buf;
            buf += wire_value.string.len;
            break;
        case WT_32BIT:
            wire_value.int32 = decode_32bit(buf, &buf);
            buf += 4;
            break;
        default:
            LWPB_ASSERT(1, "Unknown wire type");
            break;
        }
        
        // Skip unknown fields
        if (!field_desc)
            continue;
        
        switch (field_desc->opts.typ) {
        case LWPB_DOUBLE:
            *((uint64_t *) &value.double_) = wire_value.int64;
            break;
        case LWPB_FLOAT:
            *((uint32_t *) &value.float_) = wire_value.int32;
            break;
        case LWPB_INT32:
            value.int32 = wire_value.varint;
            break;
        case LWPB_INT64:
            value.int64 = wire_value.varint;
            break;
        case LWPB_UINT32:
            value.uint32 = wire_value.varint;
            break;
        case LWPB_UINT64:
            value.uint64 = wire_value.varint;
            break;
        case LWPB_SINT32:
            value.int32 = wire_value.varint;
            break;
        case LWPB_SINT64:
            value.int64 = wire_value.varint;
            break;
        case LWPB_FIXED32:
            value.uint32 = wire_value.int32;
            break;
        case LWPB_FIXED64:
            value.uint64 = wire_value.int64;
            break;
        case LWPB_SFIXED32:
            value.int32 = wire_value.int32;
            break;
        case LWPB_SFIXED64:
            value.int64 = wire_value.int64;
            break;
        case LWPB_BOOL:
            value.bool = wire_value.varint;
            break;
        case LWPB_STRING:
            value.string.len = wire_value.string.len;
            value.string.str = wire_value.string.data;
            break;
        case LWPB_BYTES:
            value.bytes.len = wire_value.string.len;
            value.bytes.data = wire_value.string.data;
            break;
        case LWPB_ENUM:
            value.enum_ = wire_value.varint;
            break;
        case LWPB_MESSAGE:
        default:
            if (decoder->field_handler)
                decoder->field_handler(decoder, msg_desc, field_desc, NULL, decoder->arg);
            // Decode nested message
            lwpb_decoder_decode(decoder, wire_value.string.data, wire_value.string.len, field_desc->msg_desc);
            break;
        }
        
        if (field_desc->opts.typ < LWPB_MESSAGE)
            if (decoder->field_handler)
                decoder->field_handler(decoder, msg_desc, field_desc, &value, decoder->arg);
    }
    
    if (decoder->msg_end_handler)
        decoder->msg_end_handler(decoder, msg_desc, decoder->arg);
}
