
#ifndef __LWPB_H__
#define __LWPB_H__

#include <stdint.h>
#include <stdlib.h>

#define PB_MAX_DEPTH    8       /**< Maximum depth of message embedding */

#define PB_ASSERT(expr, msg)                    \
    do {                                        \
        if (!(expr)) {                          \
            printf(msg);                        \
            exit(0);                            \
        }                                       \
    } while (0)

/** Protocol buffer error codes */
typedef enum {
    PB_ERR_OK = 0,
    PB_ERR_UNKNOWN_FIELD,
} pb_err_t;

/** Protocol buffer types */
typedef enum {
    PB_DOUBLE,
    PB_FLOAT,
    PB_INT32,
    PB_INT64,
    PB_UINT32,
    PB_UINT64,
    PB_SINT32,
    PB_SINT64,
    PB_FIXED32,
    PB_FIXED64,
    PB_SFIXED32,
    PB_SFIXED64,
    PB_BOOL,
    PB_STRING,
    PB_BYTES,
    PB_MESSAGE,
} pb_typ_t;

/** Protocol buffer field descriptor */
struct pb_field_desc {
    char *name;
    uint32_t id;
    pb_typ_t typ;
};

/** Protocol buffer message descriptor */
struct pb_msg_desc {
    char *name;
    uint32_t num_fields;
    const struct pb_field_desc *fields;
};

/** Protocol buffer dictionary */
typedef const struct pb_msg_desc *pb_dict_t;

/** Protocol buffer value */
union pb_value {
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
struct pb_decoder;

// Event handlers

typedef void (*pb_decoder_msg_start_handler_t)
    (struct pb_decoder *decoder, const struct pb_msg_desc *msg_desc, void *arg);

typedef void (*pb_decoder_msg_end_handler_t)
    (struct pb_decoder *decoder, const struct pb_msg_desc *msg_desc, void *arg);

typedef void (*pb_decoder_field_handler_t)
    (struct pb_decoder *decoder, const struct pb_field_desc *field_desc,
     union pb_value *value, void *arg);

/** Protocol buffer decoder */
struct pb_decoder {
    pb_dict_t dict;
    void *arg;
    pb_decoder_msg_start_handler_t msg_start_handler;
    pb_decoder_msg_end_handler_t msg_end_handler;
    pb_decoder_field_handler_t field_handler;
};

void pb_decoder_init(struct pb_decoder *decoder, pb_dict_t dict);

void pb_decoder_arg(struct pb_decoder *decoder, void *arg);

void pb_decoder_msg_handler(struct pb_decoder *decoder,
                            pb_decoder_msg_start_handler_t msg_start_handler,
                            pb_decoder_msg_end_handler_t msg_end_handler);

void pb_decoder_field_handler(struct pb_decoder *decoder,
                              pb_decoder_field_handler_t field_handler);

void pb_decoder_use_debug_handlers(struct pb_decoder *decoder);

void pb_decoder_decode(struct pb_decoder *decoder, void *data, size_t len, int msg_id);

struct pb_encoder_stack_entry {
    char *buf;
    char *buf_end;
    const struct pb_msg_desc *msg_desc;
};

/** Protocol buffer encoder */
struct pb_encoder {
    pb_dict_t dict;
    void *data;
    size_t len;
    struct pb_encoder_stack_entry stack[PB_MAX_DEPTH];
    int depth;
};

void pb_encoder_init(struct pb_encoder *encoder, pb_dict_t dict);

void pb_encoder_start(struct pb_encoder *encoder, void *data, size_t len);

size_t pb_encoder_finish(struct pb_encoder *encoder);

void pb_encoder_msg_start(struct pb_encoder *encoder, int msg_id);

void pb_encoder_msg_end(struct pb_encoder *encoder);

pb_err_t pb_encoder_add_field(struct pb_encoder *encoder, int field_id, union pb_value *value);

pb_err_t pb_encoder_add_double(struct pb_encoder *encoder, int field_id, double _double);

pb_err_t pb_encoder_add_float(struct pb_encoder *encoder, int field_id, float _float);

pb_err_t pb_encoder_add_int32(struct pb_encoder *encoder, int field_id, int32_t int32);

pb_err_t pb_encoder_add_uint32(struct pb_encoder *encoder, int field_id, uint32_t uint32);

pb_err_t pb_encoder_add_int64(struct pb_encoder *encoder, int field_id, int64_t int64);

pb_err_t pb_encoder_add_uint64(struct pb_encoder *encoder, int field_id, uint64_t uint64);

pb_err_t pb_encoder_add_bool(struct pb_encoder *encoder, int field_id, int bool);

pb_err_t pb_encoder_add_string(struct pb_encoder *encoder, int field_id, char *str);

pb_err_t pb_encoder_add_bytes(struct pb_encoder *encoder, int field_id, uint8_t *data, size_t len);


#endif // __LWPB_H__
