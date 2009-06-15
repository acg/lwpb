
#ifndef __LWPB_H__
#define __LWPB_H__

#include <stdint.h>
#include <stdlib.h>

#define LWPB_MAX_DEPTH      8   /**< Maximum depth of message embedding */

#define LWPB_ASSERT(expr, msg)                                              \
    do {                                                                    \
        if (!(expr)) {                                                      \
            printf(msg);                                                    \
            exit(0);                                                        \
        }                                                                   \
    } while (0)

/** Protocol buffer error codes */
typedef enum {
    LWPB_ERR_OK = 0,
    LWPB_ERR_UNKNOWN_FIELD,
} lwpb_err_t;

/** Protocol buffer types */
typedef enum {
    LWPB_DOUBLE,
    LWPB_FLOAT,
    LWPB_INT32,
    LWPB_INT64,
    LWPB_UINT32,
    LWPB_UINT64,
    LWPB_SINT32,
    LWPB_SINT64,
    LWPB_FIXED32,
    LWPB_FIXED64,
    LWPB_SFIXED32,
    LWPB_SFIXED64,
    LWPB_BOOL,
    LWPB_STRING,
    LWPB_BYTES,
    LWPB_MESSAGE,
} lwpb_typ_t;

/** Protocol buffer field descriptor */
struct lwpb_field_desc {
    char *name;
    uint32_t id;
    lwpb_typ_t typ;
};

/** Protocol buffer message descriptor */
struct lwpb_msg_desc {
    char *name;
    uint32_t num_fields;
    const struct lwpb_field_desc *fields;
};

/** Protocol buffer dictionary */
typedef const struct lwpb_msg_desc *lwpb_dict_t;

/** Protocol buffer value */
union lwpb_value {
    double _double;
    float _float;
    int32_t int32;
    int64_t int64;
    uint32_t uint32;
    uint64_t uint64;
    int bool;
    struct {
        char *str;
        size_t len;
    } string;
    struct {
        uint8_t *data;
        size_t len;
    } bytes;
} value;

/* Forward declaration */
struct lwpb_decoder;

// Event handlers

typedef void (*lwpb_decoder_msg_start_handler_t)
    (struct lwpb_decoder *decoder, const struct lwpb_msg_desc *msg_desc, void *arg);

typedef void (*lwpb_decoder_msg_end_handler_t)
    (struct lwpb_decoder *decoder, const struct lwpb_msg_desc *msg_desc, void *arg);

typedef void (*lwpb_decoder_field_handler_t)
    (struct lwpb_decoder *decoder, const struct lwpb_field_desc *field_desc,
     union lwpb_value *value, void *arg);

/** Protocol buffer decoder */
struct lwpb_decoder {
    lwpb_dict_t dict;
    void *arg;
    lwpb_decoder_msg_start_handler_t msg_start_handler;
    lwpb_decoder_msg_end_handler_t msg_end_handler;
    lwpb_decoder_field_handler_t field_handler;
};

void lwpb_decoder_init(struct lwpb_decoder *decoder, lwpb_dict_t dict);

void lwpb_decoder_arg(struct lwpb_decoder *decoder, void *arg);

void lwpb_decoder_msg_handler(struct lwpb_decoder *decoder,
                            lwpb_decoder_msg_start_handler_t msg_start_handler,
                            lwpb_decoder_msg_end_handler_t msg_end_handler);

void lwpb_decoder_field_handler(struct lwpb_decoder *decoder,
                              lwpb_decoder_field_handler_t field_handler);

void lwpb_decoder_use_debug_handlers(struct lwpb_decoder *decoder);

void lwpb_decoder_decode(struct lwpb_decoder *decoder, void *data, size_t len, int msg_id);

struct lwpb_encoder_stack_entry {
    char *buf;
    char *buf_end;
    const struct lwpb_msg_desc *msg_desc;
};

/** Protocol buffer encoder */
struct lwpb_encoder {
    lwpb_dict_t dict;
    void *data;
    size_t len;
    struct lwpb_encoder_stack_entry stack[LWPB_MAX_DEPTH];
    int depth;
};

void lwpb_encoder_init(struct lwpb_encoder *encoder, lwpb_dict_t dict);

void lwpb_encoder_start(struct lwpb_encoder *encoder, void *data, size_t len);

size_t lwpb_encoder_finish(struct lwpb_encoder *encoder);

void lwpb_encoder_msg_start(struct lwpb_encoder *encoder, int msg_id);

void lwpb_encoder_msg_end(struct lwpb_encoder *encoder);

lwpb_err_t lwpb_encoder_add_field(struct lwpb_encoder *encoder, int field_id, union lwpb_value *value);

lwpb_err_t lwpb_encoder_add_double(struct lwpb_encoder *encoder, int field_id, double _double);

lwpb_err_t lwpb_encoder_add_float(struct lwpb_encoder *encoder, int field_id, float _float);

lwpb_err_t lwpb_encoder_add_int32(struct lwpb_encoder *encoder, int field_id, int32_t int32);

lwpb_err_t lwpb_encoder_add_uint32(struct lwpb_encoder *encoder, int field_id, uint32_t uint32);

lwpb_err_t lwpb_encoder_add_int64(struct lwpb_encoder *encoder, int field_id, int64_t int64);

lwpb_err_t lwpb_encoder_add_uint64(struct lwpb_encoder *encoder, int field_id, uint64_t uint64);

lwpb_err_t lwpb_encoder_add_bool(struct lwpb_encoder *encoder, int field_id, int bool);

lwpb_err_t lwpb_encoder_add_string(struct lwpb_encoder *encoder, int field_id, char *str);

lwpb_err_t lwpb_encoder_add_bytes(struct lwpb_encoder *encoder, int field_id, uint8_t *data, size_t len);


#endif // __LWPB_H__
