
#ifndef __LWPB_H__
#define __LWPB_H__

#include <stdint.h>
#include <stdlib.h>

/* Maximum depth of message embedding */
#ifndef LWPB_MAX_DEPTH
#define LWPB_MAX_DEPTH 8
#endif

/* Provide field names as strings */
#ifndef LWPB_FIELD_NAMES
#define LWPB_FIELD_NAMES 1
#endif

/* Provide field default values */
#ifndef LWPB_FIELD_DEFAULTS
#define LWPB_FIELD_DEFAULTS 1
#endif

/* Provide message names as strings */
#ifndef LWPB_MESSAGE_NAMES
#define LWPB_MESSAGE_NAMES 1
#endif

/* Simple assert macro */
#ifndef LWPB_ASSERT
#define LWPB_ASSERT(expr, msg)                                              \
    do {                                                                    \
        if (!(expr)) {                                                      \
            printf(msg);                                                    \
            exit(0);                                                        \
        }                                                                   \
    } while (0)
#endif

/** Protocol buffer error codes */
typedef enum {
    LWPB_ERR_OK,
    LWPB_ERR_UNKNOWN_FIELD,
} lwpb_err_t;

/**
 * Protocol buffer field types. The value is split into several bitfields:
 * 0-1: Field label
 * 2-5: Field value type
 * 6-32: Flags
 */
typedef enum {
    /* Field label */
    LWPB_REQUIRED       = (0 << 0),
    LWPB_OPTIONAL       = (1 << 0),
    LWPB_REPEATED       = (2 << 0),
    /* Field value types */
    LWPB_DOUBLE         = (0 << 2),
    LWPB_FLOAT          = (1 << 2),
    LWPB_INT32          = (2 << 2),
    LWPB_INT64          = (3 << 2),
    LWPB_UINT32         = (4 << 2),
    LWPB_UINT64         = (5 << 2),
    LWPB_SINT32         = (6 << 2),
    LWPB_SINT64         = (7 << 2),
    LWPB_FIXED32        = (8 << 2),
    LWPB_FIXED64        = (9 << 2),
    LWPB_SFIXED32       = (10 << 2),
    LWPB_SFIXED64       = (11 << 2),
    LWPB_BOOL           = (12 << 2),
    LWPB_STRING         = (13 << 2),
    LWPB_BYTES          = (14 << 2),
    LWPB_MESSAGE        = (15 << 2),
    /* Field flags */
    LWPB_HAS_DEFAULT    = ((1 << 0) << 6),
    LWPB_IS_PACKED      = ((1 << 1) << 6),
    LWPB_IS_DEPRECATED  = ((1 << 2) << 6),
} lwpb_typ_t;

#define LWPB_TYP_RULE(typ)  ((typ) & (0x3 << 0))
#define LWPB_TYP_VALUE(typ) ((typ) & (0xf << 2))

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
struct lwpb_msg_desc;

/** Protocol buffer field descriptor */
struct lwpb_field_desc {
    uint32_t id;                /**< Field number */
    lwpb_typ_t typ;             /**< Field type */
    struct lwpb_msg_desc *msg_desc; /**< Message descriptor, if field is message */
#ifdef LWPB_FIELD_NAMES
    char *name;                 /**< Field name */
#endif
#ifdef LWPB_FIELD_DEFAULTS
    union lwpb_value def;       /**< Field default value */
#endif
};

/** Protocol buffer message descriptor */
struct lwpb_msg_desc {
    uint32_t num_fields;        /**< Number of fields */
    const struct lwpb_field_desc *fields; /**< Array of field descriptors */
#ifdef LWPB_MESSAGE_NAMES
    char *name;
#endif
};

/** Protocol buffer dictionary */
typedef const struct lwpb_msg_desc *lwpb_dict_t;


/* Forward declaration */
struct lwpb_decoder;

// Event handlers

/**
 * This handler is called when the decoder encountered a new message.
 * @param decoder Decoder
 * @param msg_desc Message descriptor
 * @param arg User argument
 */
typedef void (*lwpb_decoder_msg_start_handler_t)
    (struct lwpb_decoder *decoder,
     const struct lwpb_msg_desc *msg_desc, void *arg);

/**
 * This handler is called when the decoder finished decoding a message.
 * @param decoder Decoder
 * @param msg_desc Message descriptor
 * @param arg User argument
 */
typedef void (*lwpb_decoder_msg_end_handler_t)
    (struct lwpb_decoder *decoder,
     const struct lwpb_msg_desc *msg_desc, void *arg);

/**
 * This handler is called when the decoder has decoded a field.
 * @param decoder Decoder
 * @param msg_desc Message descriptor of the message containing the field
 * @param field_desc Field descriptor
 * @param value Field value
 * @param arg User argument
 */
typedef void (*lwpb_decoder_field_handler_t)
    (struct lwpb_decoder *decoder,
     const struct lwpb_msg_desc *msg_desc,
     const struct lwpb_field_desc *field_desc,
     union lwpb_value *value, void *arg);

/** Protocol buffer decoder */
struct lwpb_decoder {
    void *arg;
    lwpb_decoder_msg_start_handler_t msg_start_handler;
    lwpb_decoder_msg_end_handler_t msg_end_handler;
    lwpb_decoder_field_handler_t field_handler;
};

void lwpb_decoder_init(struct lwpb_decoder *decoder);

void lwpb_decoder_arg(struct lwpb_decoder *decoder, void *arg);

void lwpb_decoder_msg_handler(struct lwpb_decoder *decoder,
                            lwpb_decoder_msg_start_handler_t msg_start_handler,
                            lwpb_decoder_msg_end_handler_t msg_end_handler);

void lwpb_decoder_field_handler(struct lwpb_decoder *decoder,
                              lwpb_decoder_field_handler_t field_handler);

void lwpb_decoder_use_debug_handlers(struct lwpb_decoder *decoder);

void lwpb_decoder_decode(struct lwpb_decoder *decoder,
                         void *data, size_t len,
                         const struct lwpb_msg_desc *desc);

struct lwpb_encoder_stack_entry {
    char *buf;
    char *buf_end;
    const struct lwpb_msg_desc *msg_desc;
};

/** Protocol buffer encoder */
struct lwpb_encoder {
    void *data;
    size_t len;
    struct lwpb_encoder_stack_entry stack[LWPB_MAX_DEPTH];
    int depth;
};

void lwpb_encoder_init(struct lwpb_encoder *encoder);

void lwpb_encoder_start(struct lwpb_encoder *encoder, void *data, size_t len);

size_t lwpb_encoder_finish(struct lwpb_encoder *encoder);

void lwpb_encoder_msg_start(struct lwpb_encoder *encoder,
                            const struct lwpb_msg_desc *msg_desc);

void lwpb_encoder_msg_end(struct lwpb_encoder *encoder);

lwpb_err_t lwpb_encoder_add_field(struct lwpb_encoder *encoder,
                                  int field_id, union lwpb_value *value);

lwpb_err_t lwpb_encoder_add_double(struct lwpb_encoder *encoder,
                                   int field_id, double _double);

lwpb_err_t lwpb_encoder_add_float(struct lwpb_encoder *encoder,
                                  int field_id, float _float);

lwpb_err_t lwpb_encoder_add_int32(struct lwpb_encoder *encoder,
                                  int field_id, int32_t int32);

lwpb_err_t lwpb_encoder_add_uint32(struct lwpb_encoder *encoder,
                                   int field_id, uint32_t uint32);

lwpb_err_t lwpb_encoder_add_int64(struct lwpb_encoder *encoder,
                                  int field_id, int64_t int64);

lwpb_err_t lwpb_encoder_add_uint64(struct lwpb_encoder *encoder,
                                   int field_id, uint64_t uint64);

lwpb_err_t lwpb_encoder_add_bool(struct lwpb_encoder *encoder,
                                 int field_id, int bool);

lwpb_err_t lwpb_encoder_add_string(struct lwpb_encoder *encoder,
                                   int field_id, char *str);

lwpb_err_t lwpb_encoder_add_bytes(struct lwpb_encoder *encoder,
                                  int field_id, uint8_t *data, size_t len);


#endif // __LWPB_H__
