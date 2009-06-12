
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lwpb_private.h"
#include "lwpb.h"

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
 * @param dict Message dictionary
 */
void pb_encoder_init(struct pb_encoder *encoder, pb_dict_t dict)
{
    encoder->dict = dict;
}

void pb_encoder_start(struct pb_encoder *encoder, void *data, size_t len)
{
    encoder->data = data;
    encoder->len = len;
    encoder->depth = 0;
    encoder->stack[0].buf = data;
    encoder->stack[0].buf_end = &encoder->stack[0].buf[len];
}

size_t pb_encoder_finish(struct pb_encoder *encoder)
{
    return encoder->stack[0].buf - (char *) encoder->data;
}

void pb_encoder_msg_start(struct pb_encoder *encoder, int msg_id)
{
    struct pb_encoder_stack_entry *entry;
    
    encoder->depth++;
    PB_ASSERT(encoder->depth < PB_MAX_DEPTH, "Message stacking too deep");
    
    entry = &encoder->stack[encoder->depth - 1];
    entry->msg_desc = &encoder->dict[msg_id];
    
}

void pb_encoder_msg_end(struct pb_encoder *encoder)
{
    struct pb_encoder_stack_entry *entry;
    
    encoder->depth--;
    PB_ASSERT(encoder->depth >= 0, "Message stacking too shallow");
    
    entry = &encoder->stack[encoder->depth - 1];
}

pb_err_t pb_encoder_add_field(struct pb_encoder *encoder, int field_id, union pb_value *value)
{
    struct pb_encoder_stack_entry *entry;
    int i;
    const struct pb_field_desc *field_desc = NULL;
    uint64_t key;
    enum wire_type wire_type;
    union wire_value wire_value;
    
    PB_ASSERT(encoder->depth > 0, "Fields can only be added inside a message");
    
    entry = &encoder->stack[encoder->depth - 1];
    
    // Find field descriptor
    for (i = 0; i < entry->msg_desc->num_fields; i++)
        if (field_id == entry->msg_desc->fields[i].id) {
            field_desc = &entry->msg_desc->fields[i];
            break;
        }
    if (!field_desc)
        return PB_ERR_UNKNOWN_FIELD;
    
    // Encode wire value
    switch (field_desc->typ) {
    case PB_DOUBLE:
        wire_type = WT_64BIT;
        wire_value.int64 = *((uint64_t *) &value->_double);
        break;
    case PB_FLOAT:
        wire_type = WT_32BIT;
        wire_value.int32 = *((uint32_t *) &value->_float);
        break;
    case PB_INT32:
    case PB_UINT32:
    case PB_SINT32:
        wire_type = WT_VARINT;
        wire_value.varint = value->int32;
        break;
    case PB_INT64:
    case PB_UINT64:
    case PB_SINT64:
        wire_type = WT_VARINT;
        wire_value.varint = value->int64;
        break;
    case PB_FIXED32:
        wire_type = WT_32BIT;
        wire_value.int32 = value->uint32;
        break;
    case PB_FIXED64:
        wire_type = WT_64BIT;
        wire_value.int32 = value->uint64;
        break;
    case PB_SFIXED32:
        wire_type = WT_32BIT;
        wire_value.int32 = value->int32;
        break;
    case PB_SFIXED64:
        wire_type = WT_64BIT;
        wire_value.int32 = value->int64;
        break;
    case PB_BOOL:
        wire_type = WT_VARINT;
        wire_value.varint = value->bool;
        break;
    case PB_STRING:
        wire_type = WT_STRING;
        wire_value.string.data = value->string.str;
        wire_value.string.len = value->string.len;
        break;
    case PB_BYTES:
        wire_type = WT_STRING;
        wire_value.string.data = value->bytes.data;
        wire_value.string.len = value->bytes.len;
        break;
    case PB_MESSAGE:
        break;
    }
    
    key = wire_type | (field_id << 3);
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
        PB_ASSERT(1, "Unknown wire type");
        break;
    }
    
    return PB_ERR_OK;
}

pb_err_t pb_encoder_add_double(struct pb_encoder *encoder, int field_id, double _double)
{
    union pb_value value;
    value._double = _double;
    return pb_encoder_add_field(encoder, field_id, &value);
}

pb_err_t pb_encoder_add_float(struct pb_encoder *encoder, int field_id, float _float)
{
    union pb_value value;
    value._float = _float;
    return pb_encoder_add_field(encoder, field_id, &value);
}

pb_err_t pb_encoder_add_int32(struct pb_encoder *encoder, int field_id, int32_t int32)
{
    union pb_value value;
    value.int32 = int32;
    return pb_encoder_add_field(encoder, field_id, &value);
}

pb_err_t pb_encoder_add_uint32(struct pb_encoder *encoder, int field_id, uint32_t uint32)
{
    union pb_value value;
    value.uint32 = uint32;
    return pb_encoder_add_field(encoder, field_id, &value);
}

pb_err_t pb_encoder_add_int64(struct pb_encoder *encoder, int field_id, int64_t int64)
{
    union pb_value value;
    value.int64 = int64;
    return pb_encoder_add_field(encoder, field_id, &value);
}

pb_err_t pb_encoder_add_uint64(struct pb_encoder *encoder, int field_id, uint64_t uint64)
{
    union pb_value value;
    value.uint64 = uint64;
    return pb_encoder_add_field(encoder, field_id, &value);
}

pb_err_t pb_encoder_add_bool(struct pb_encoder *encoder, int field_id, int bool)
{
    union pb_value value;
    value.bool = bool;
    return pb_encoder_add_field(encoder, field_id, &value);
}

pb_err_t pb_encoder_add_string(struct pb_encoder *encoder, int field_id, char *str)
{
    union pb_value value;
    value.string.str = str;
    value.string.len = strlen(str);
    return pb_encoder_add_field(encoder, field_id, &value);
}

pb_err_t pb_encoder_add_bytes(struct pb_encoder *encoder, int field_id, uint8_t *data, size_t len)
{
    union pb_value value;
    value.string.str = (char *) data;
    value.string.len = len;
    return pb_encoder_add_field(encoder, field_id, &value);
}
