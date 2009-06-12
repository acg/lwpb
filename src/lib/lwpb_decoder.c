
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lwpb_private.h"
#include "lwpb.h"

// Debug handlers

static int debug_indent;

static void debug_print_indent(void)
{
    int i;
    
    for (i = 0; i < debug_indent; i++)
        printf("  ");
}

static void debug_msg_start_handler(struct pb_decoder *decoder,
                                    const struct pb_msg_desc *msg_desc,
                                    void *arg)
{
    debug_print_indent();
    printf("%s:\n", msg_desc->name);
    debug_indent++;
}

static void debug_msg_end_handler(struct pb_decoder *decoder,
                                  const struct pb_msg_desc *msg_desc,
                                  void *arg)
{
    debug_indent--;
}

static void debug_field_handler(struct pb_decoder *decoder,
                                const struct pb_field_desc *field_desc,
                                union pb_value *value, void *arg)
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
    };
    pb_typ_t typ;
    
    typ = field_desc->typ <= PB_MESSAGE ? field_desc->typ : PB_MESSAGE;
    
    debug_print_indent();
    printf("%-20s %-10s = ", field_desc->name, typ_names[typ]);
    
    switch (field_desc->typ) {
    case PB_DOUBLE:
        printf("%f", value->_double);
        break;
    case PB_FLOAT:
        printf("%f", value->_float);
        break;
    case PB_INT32:
    case PB_SINT32:
    case PB_SFIXED32:
        printf("%d", value->int32);
        break;
    case PB_INT64:
    case PB_SINT64:
    case PB_SFIXED64:
        printf("%lld", value->int64);
        break;
    case PB_UINT32:
    case PB_FIXED32:
        printf("%u", value->int32);
        break;
    case PB_UINT64:
    case PB_FIXED64:
        printf("%llu", value->int64);
        break;
    case PB_BOOL:
        printf("%s", value->bool ? "true" : "false");
        break;
    case PB_STRING:
        while (value->string.len--)
            printf("%c", *value->string.str++);
        break;
    case PB_BYTES:
        while (value->bytes.len--)
            printf("%02x ", *value->bytes.data++);
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
 * @param dict Message dictionary
 */
void pb_decoder_init(struct pb_decoder *decoder, pb_dict_t dict)
{
    decoder->dict = dict;
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
void pb_decoder_arg(struct pb_decoder *decoder, void *arg)
{
    decoder->arg = arg;
}

/**
 * Sets the message start and end handlers.
 * @param decoder Decoder
 * @param msg_start_handler Message start handler
 * @param msg_end_handler Message end handler
 */
void pb_decoder_msg_handler(struct pb_decoder *decoder,
                            pb_decoder_msg_start_handler_t msg_start_handler,
                            pb_decoder_msg_end_handler_t msg_end_handler)
{
    decoder->msg_start_handler = msg_start_handler;
    decoder->msg_end_handler = msg_end_handler;
}

/**
 * Sets the field handler.
 * @param decoder Decoder
 * @param field_handler Field handler
 */
void pb_decoder_field_handler(struct pb_decoder *decoder,
                              pb_decoder_field_handler_t field_handler)
{
    decoder->field_handler = field_handler;
}

/**
 * Setups the decoder to use the verbose debug handlers which output the
 * message contents to the console.
 * @param decoder Decoder 
 */
void pb_decoder_use_debug_handlers(struct pb_decoder *decoder)
{
    pb_decoder_msg_handler(decoder,
                           debug_msg_start_handler, debug_msg_end_handler);
    pb_decoder_field_handler(decoder, debug_field_handler);
}

/**
 * Decodes a protocol buffer.
 * @param decoder Decoder
 * @param data Data to decode
 * @param len Length of data to decode
 * @param msg_id Root message id of the protocol buffer
 */
void pb_decoder_decode(struct pb_decoder *decoder, void *data, size_t len, int msg_id)
{
    int i;
    char *buf = data;
    char *buf_end = &buf[len];
    uint64_t key;
    int field_id;
    const struct pb_msg_desc *msg_desc;
    const struct pb_field_desc *field_desc = NULL;
    enum wire_type wire_type;
    union wire_value wire_value;
    union pb_value value;

    // Get message description
    // FIXME bounds check
    msg_desc = &decoder->dict[msg_id];
    
    if (decoder->msg_start_handler)
        decoder->msg_start_handler(decoder, msg_desc, decoder->arg);
    
    while(buf < buf_end)
    {
        key = decode_varint(buf, &buf);
        field_id = key >> 3;
        wire_type = key & 0x07;
        
        // Find the field descriptor
        for (i = 0; i < msg_desc->num_fields; i++)
            if (msg_desc->fields[i].id == field_id) {
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
            PB_ASSERT(1, "Unknown wire type");
            break;
        }
        
        // Skip unknown fields
        if (!field_desc)
            continue;
        
        switch (field_desc->typ) {
        case PB_DOUBLE:
            *((uint64_t *) &value._double) = wire_value.int64;
            break;
        case PB_FLOAT:
            *((uint32_t *) &value._float) = wire_value.int32;
            break;
        case PB_INT32:
            value.int32 = wire_value.varint;
            break;
        case PB_INT64:
            value.int64 = wire_value.varint;
            break;
        case PB_UINT32:
            value.uint32 = wire_value.varint;
            break;
        case PB_UINT64:
            value.uint64 = wire_value.varint;
            break;
        case PB_SINT32:
            value.int32 = wire_value.varint;
            break;
        case PB_SINT64:
            value.int64 = wire_value.varint;
            break;
        case PB_FIXED32:
            value.uint32 = wire_value.int32;
            break;
        case PB_FIXED64:
            value.uint64 = wire_value.int64;
            break;
        case PB_SFIXED32:
            value.int32 = wire_value.int32;
            break;
        case PB_SFIXED64:
            value.int64 = wire_value.int64;
            break;
        case PB_BOOL:
            value.bool = wire_value.varint;
            break;
        case PB_STRING:
            value.string.len = wire_value.string.len;
            value.string.str = wire_value.string.data;
            break;
        case PB_BYTES:
            value.bytes.len = wire_value.string.len;
            value.bytes.data = wire_value.string.data;
            break;
        case PB_MESSAGE:
        default:
            if (decoder->field_handler)
                decoder->field_handler(decoder, field_desc, NULL, decoder->arg);
            // Decode nested message
            pb_decoder_decode(decoder, wire_value.string.data, wire_value.string.len, field_desc->typ - PB_MESSAGE);
            break;
        }
        
        if (field_desc->typ < PB_MESSAGE)
            if (decoder->field_handler)
                decoder->field_handler(decoder, field_desc, &value, decoder->arg);
    }
    
    if (decoder->msg_end_handler)
        decoder->msg_end_handler(decoder, msg_desc, decoder->arg);
}
