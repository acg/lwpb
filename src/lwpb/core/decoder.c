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

#include <lwpb/lwpb.h>

#include "private.h"


// Debug handlers

static int debug_indent;

static void debug_print_indent(void)
{
    int i;
    
    for (i = 0; i < debug_indent; i++)
        LWPB_DIAG_PRINTF("  ");
}

static void debug_msg_start_handler(struct lwpb_decoder *decoder,
                                    const struct lwpb_msg_desc *msg_desc,
                                    void *arg)
{
    const char *name;

#if LWPB_MESSAGE_NAMES
    name = msg_desc->name;
#else
    name = "<message>";
#endif
    
    debug_print_indent();
    LWPB_DIAG_PRINTF("%s:\n", name);
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
        "(enum)",
        "(string)",
        "(bytes)",
        "(message)",
    };
    
    const char *name;
    
#if LWPB_FIELD_NAMES
    name = field_desc->name;
#else
    name = "<field>";
#endif
    
    debug_print_indent();
    LWPB_DIAG_PRINTF("%-20s %-10s = ", name, typ_names[field_desc->opts.typ]);
    
    switch (field_desc->opts.typ) {
    case LWPB_DOUBLE:
        LWPB_DIAG_PRINTF("%f", value->double_);
        break;
    case LWPB_FLOAT:
        LWPB_DIAG_PRINTF("%f", value->float_);
        break;
    case LWPB_INT32:
    case LWPB_SINT32:
    case LWPB_SFIXED32:
        LWPB_DIAG_PRINTF("%d", value->int32);
        break;
    case LWPB_INT64:
    case LWPB_SINT64:
    case LWPB_SFIXED64:
        LWPB_DIAG_PRINTF("%lld", value->int64);
        break;
    case LWPB_UINT32:
    case LWPB_FIXED32:
        LWPB_DIAG_PRINTF("%u", value->int32);
        break;
    case LWPB_UINT64:
    case LWPB_FIXED64:
        LWPB_DIAG_PRINTF("%llu", value->int64);
        break;
    case LWPB_BOOL:
        LWPB_DIAG_PRINTF("%s", value->bool ? "true" : "false");
        break;
    case LWPB_ENUM:
        LWPB_DIAG_PRINTF("%d", value->enum_);
        break;
    case LWPB_STRING:
        while (value->string.len--)
            LWPB_DIAG_PRINTF("%c", *value->string.str++);
        break;
    case LWPB_BYTES:
        while (value->bytes.len--)
            LWPB_DIAG_PRINTF("%02x ", *value->bytes.data++);
        break;
    default:
        break;
    }
    
    LWPB_DIAG_PRINTF("\n");
}

// Decoder utilities

/**
 * Decodes a variable integer in base-128 format.
 * See http://code.google.com/apis/protocolbuffers/docs/encoding.html for more
 * information.
 * @param buf Memory buffer
 * @param varint Buffer to decode into
 * @return Returns LWPB_ERR_OK if successful or LWPB_ERR_END_OF_BUF if there
 * were not enough bytes in the memory buffer. 
 */
static lwpb_err_t decode_varint(struct lwpb_buf *buf, u64_t *varint)
{
    int bitpos;
    
    *varint = 0;
    for (bitpos = 0; *buf->pos & 0x80 && bitpos < 64; bitpos += 7, buf->pos++) {
        *varint |= (u64_t) (*buf->pos & 0x7f) << bitpos;
        if (buf->end - buf->pos < 2)
            return LWPB_ERR_END_OF_BUF;
    }
    *varint |= (u64_t) (*buf->pos & 0x7f) << bitpos;
    buf->pos++;
    
    return LWPB_ERR_OK;
}

/**
 * Decodes a 32 bit integer
 * @param buf Memory buffer
 * @param value Buffer to decode into
 * @return Returns LWPB_ERR_OK if successful or LWPB_ERR_END_OF_BUF if there
 * were not enough bytes in the memory buffer. 
 */
static lwpb_err_t decode_32bit(struct lwpb_buf *buf, u32_t *value)
{
    if (lwpb_buf_left(buf) < 4)
        return LWPB_ERR_END_OF_BUF;

    *value = buf->pos[0] | (buf->pos[1] << 8) |
             (buf->pos[2] << 16) | (buf->pos[3] << 24);
    buf->pos += 4;
    
    return LWPB_ERR_OK;
}

/**
 * Decodes a 64 bit integer
 * @param buf Memory buffer
 * @param value Buffer to decode into
 * @return Returns LWPB_ERR_OK if successful or LWPB_ERR_END_OF_BUF if there
 * were not enough bytes in the memory buffer. 
 */
static lwpb_err_t decode_64bit(struct lwpb_buf *buf, u64_t *value)
{
    int i;
    
    if (lwpb_buf_left(buf) < 8)
        return LWPB_ERR_END_OF_BUF;
    
    *value = 0;
    for (i = 7; i >= 0; i--)
        *value = (*value << 8) | buf->pos[i];
    buf->pos += 8;
    
    return LWPB_ERR_OK;
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
 * @param msg_desc Root message descriptor of the protocol buffer
 * @param data Data to decode
 * @param len Length of data to decode
 * @param used Returns the number of decoded bytes when not NULL.
 * @return Returns LWPB_ERR_OK when data was successfully decoded.
 */
lwpb_err_t lwpb_decoder_decode(struct lwpb_decoder *decoder,
                               const struct lwpb_msg_desc *msg_desc,
                               void *data, size_t len, size_t *used)
{
    lwpb_err_t ret;
    int i;
    struct lwpb_buf buf;
    u64_t key;
    int number;
    const struct lwpb_field_desc *field_desc = NULL;
    enum wire_type wire_type;
    union wire_value wire_value;
    union lwpb_value value;
    
    lwpb_buf_init(&buf, data, len);

    if (decoder->msg_start_handler)
        decoder->msg_start_handler(decoder, msg_desc, decoder->arg);
    
    while (lwpb_buf_left(&buf) > 0) {
        // Decode the field key
        ret = decode_varint(&buf, &key);
        if (ret != LWPB_ERR_OK)
            return ret;
        
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
            ret = decode_varint(&buf, &wire_value.varint);
            if (ret != LWPB_ERR_OK)
                return ret;
            break;
        case WT_64BIT:
            ret = decode_64bit(&buf, &wire_value.int64);
            if (ret != LWPB_ERR_OK)
                return ret;
            break;
        case WT_STRING:
            ret = decode_varint(&buf, &wire_value.string.len);
            if (ret != LWPB_ERR_OK)
                return ret;
            if (wire_value.string.len > lwpb_buf_left(&buf))
                return LWPB_ERR_END_OF_BUF;
            wire_value.string.data = buf.pos;
            buf.pos += wire_value.string.len;
            break;
        case WT_32BIT:
            ret = decode_32bit(&buf, &wire_value.int32);
            if (ret != LWPB_ERR_OK)
                return ret;
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
            *((u64_t *) &value.double_) = wire_value.int64;
            break;
        case LWPB_FLOAT:
            *((u32_t *) &value.float_) = wire_value.int32;
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
            // Zig-zag encoding
            value.int32 = (wire_value.varint >> 1) ^ -((s32_t) (wire_value.varint & 1));
            break;
        case LWPB_SINT64:
            // Zig-zag encoding
            value.int64 = (wire_value.varint >> 1) ^ -((s64_t) (wire_value.varint & 1));
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
        case LWPB_ENUM:
            value.enum_ = wire_value.varint;
            break;
        case LWPB_STRING:
            value.string.len = wire_value.string.len;
            value.string.str = wire_value.string.data;
            break;
        case LWPB_BYTES:
            value.bytes.len = wire_value.string.len;
            value.bytes.data = wire_value.string.data;
            break;
        case LWPB_MESSAGE:
        default:
            if (decoder->field_handler)
                decoder->field_handler(decoder, msg_desc, field_desc, NULL, decoder->arg);
            // Decode nested message
            lwpb_decoder_decode(decoder, field_desc->msg_desc, wire_value.string.data, wire_value.string.len, NULL);
            break;
        }
        
        if (field_desc->opts.typ < LWPB_MESSAGE)
            if (decoder->field_handler)
                decoder->field_handler(decoder, msg_desc, field_desc, &value, decoder->arg);
    }
    
    if (decoder->msg_end_handler)
        decoder->msg_end_handler(decoder, msg_desc, decoder->arg);
    
    if (used)
        *used = lwpb_buf_used(&buf);
    
    return LWPB_ERR_OK;
}
