/**
 * @file lwpb.h
 * 
 * Lightweight protocol buffers.
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

#ifndef __LWPB_H__
#define __LWPB_H__

#include <stdint.h>
#include <stdlib.h>

/* Maximum depth of message embedding */
#ifndef LWPB_MAX_DEPTH
#define LWPB_MAX_DEPTH 8
#endif

/* Maximum number of required fields in a message */
#ifndef LWPB_MAX_REQUIRED_FIELDS
#define LWPB_MAX_REQUIRED_FIELDS 16
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

/* Provide method names as strings */
#ifndef LWPB_METHOD_NAMES
#define LWPB_METHOD_NAMES 1
#endif

/* Provide service names as strings */
#ifndef LWPB_SERVICE_NAMES
#define LWPB_SERVICE_NAMES 1
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
    LWPB_ERR_END_OF_BUF,
} lwpb_err_t;

/* Field labels */
#define LWPB_REQUIRED       0
#define LWPB_OPTIONAL       1
#define LWPB_REPEATED       2

/* Field value types */
#define LWPB_DOUBLE         0
#define LWPB_FLOAT          1
#define LWPB_INT32          2
#define LWPB_INT64          3
#define LWPB_UINT32         4
#define LWPB_UINT64         5
#define LWPB_SINT32         6
#define LWPB_SINT64         7
#define LWPB_FIXED32        8
#define LWPB_FIXED64        9
#define LWPB_SFIXED32       10
#define LWPB_SFIXED64       11
#define LWPB_BOOL           12
#define LWPB_ENUM           13
#define LWPB_STRING         14
#define LWPB_BYTES          15
#define LWPB_MESSAGE        16

/* Field flags */
#define LWPB_HAS_DEFAULT    (1 << 0)
#define LWPB_IS_PACKED      (1 << 1)
#define LWPB_IS_DEPRECATED  (1 << 2)

/** Protocol buffer field options */
typedef struct {
    unsigned int label : 2;
    unsigned int typ : 6;
    unsigned int flags : 8;
} lwpb_field_opts_t;

/** Protocol buffer bool type */
typedef int lwpb_bool_t;

/** Protocol buffer enum type */
typedef int lwpb_enum_t;

/** Protocol buffer value */
union lwpb_value {
    double double_;
    float float_;
    int32_t int32;
    int64_t int64;
    uint32_t uint32;
    uint64_t uint64;
    lwpb_bool_t bool;
    struct {
        char *str;
        size_t len;
    } string;
    struct {
        uint8_t *data;
        size_t len;
    } bytes;
    struct {
        void *data;
        size_t len;
    } message;
    lwpb_enum_t enum_;
    int null;
} value;

/* Forward declaration */
struct lwpb_msg_desc;

/** Protocol buffer field descriptor */
struct lwpb_field_desc {
    uint32_t number;            /**< Field number */
    lwpb_field_opts_t opts;     /**< Field options (label, value type, flags) */
    const struct lwpb_msg_desc *msg_desc; /**< Message descriptor, if field is message */
#if LWPB_FIELD_NAMES
    const char *name;           /**< Field name */
#endif
#if LWPB_FIELD_DEFAULTS
    union lwpb_value def;       /**< Field default value */
#endif
};

/** Protocol buffer message descriptor */
struct lwpb_msg_desc {
    uint32_t num_fields;        /**< Number of fields */
    const struct lwpb_field_desc *fields; /**< Array of field descriptors */
#if LWPB_MESSAGE_NAMES
    const char *name;
#endif
};

/* Forward declaration */
struct lwpb_service_desc;

/** Protocol buffer method descriptor */
struct lwpb_method_desc {
    const struct lwpb_service_desc *service; /**< Service descriptor */
    const struct lwpb_msg_desc *input; /**< Input message descriptor */ 
    const struct lwpb_msg_desc *output; /**< Output message descriptor */
#if LWPB_METHOD_NAMES
    const char *name;           /**< Method name */
#endif
};

/** Protocol buffer service descriptor */
struct lwpb_service_desc {
    const uint32_t num_methods; /**< Number of methods */
    const struct lwpb_method_desc *methods; /**< Array of method descriptors */
#if LWPB_SERVICE_NAMES
    const char *name;           /**< Service name */
#endif
};

/** Simple memory buffer */
struct lwpb_buf {
    uint8_t *base;  /**< Buffers base address */
    uint8_t *pos;   /**< Buffers current position */
    uint8_t *end;   /**< Buffers end address (first invalid byte) */
};

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

lwpb_err_t lwpb_decoder_decode(struct lwpb_decoder *decoder,
                               const struct lwpb_msg_desc *msg_desc,
                               void *data, size_t len);


struct lwpb_encoder_stack_frame {
    struct lwpb_buf buf;
    const struct lwpb_field_desc *field_desc;
    const struct lwpb_msg_desc *msg_desc;
};

/** Protocol buffer encoder */
struct lwpb_encoder {
    struct lwpb_encoder_stack_frame stack[LWPB_MAX_DEPTH];
    int depth;
    
};

void lwpb_encoder_init(struct lwpb_encoder *encoder);

void lwpb_encoder_start(struct lwpb_encoder *encoder,
                        const struct lwpb_msg_desc *msg_desc,
                        void *data, size_t len);

size_t lwpb_encoder_finish(struct lwpb_encoder *encoder);

lwpb_err_t lwpb_encoder_nested_start(struct lwpb_encoder *encoder,
                                     const struct lwpb_field_desc *field_desc);

lwpb_err_t lwpb_encoder_nested_end(struct lwpb_encoder *encoder);

lwpb_err_t lwpb_encoder_add_field(struct lwpb_encoder *encoder,
                                  const struct lwpb_field_desc *field_desc,
                                  union lwpb_value *value);

lwpb_err_t lwpb_encoder_add_double(struct lwpb_encoder *encoder,
                                   const struct lwpb_field_desc *field_desc,
                                   double double_);

lwpb_err_t lwpb_encoder_add_float(struct lwpb_encoder *encoder,
                                  const struct lwpb_field_desc *field_desc,
                                  float float_);

lwpb_err_t lwpb_encoder_add_int32(struct lwpb_encoder *encoder,
                                  const struct lwpb_field_desc *field_desc,
                                  int32_t int32);

lwpb_err_t lwpb_encoder_add_uint32(struct lwpb_encoder *encoder,
                                   const struct lwpb_field_desc *field_desc,
                                   uint32_t uint32);

lwpb_err_t lwpb_encoder_add_int64(struct lwpb_encoder *encoder,
                                  const struct lwpb_field_desc *field_desc,
                                  int64_t int64);

lwpb_err_t lwpb_encoder_add_uint64(struct lwpb_encoder *encoder,
                                   const struct lwpb_field_desc *field_desc,
                                   uint64_t uint64);

lwpb_err_t lwpb_encoder_add_bool(struct lwpb_encoder *encoder,
                                 const struct lwpb_field_desc *field_desc,
                                 lwpb_bool_t bool);

lwpb_err_t lwpb_encoder_add_enum(struct lwpb_encoder *encoder,
                                 const struct lwpb_field_desc *field_desc,
                                 lwpb_enum_t enum_);

lwpb_err_t lwpb_encoder_add_string(struct lwpb_encoder *encoder,
                                   const struct lwpb_field_desc *field_desc,
                                   char *str);

lwpb_err_t lwpb_encoder_add_bytes(struct lwpb_encoder *encoder,
                                  const struct lwpb_field_desc *field_desc,
                                  uint8_t *data, size_t len);


const char *lwpb_err_text(lwpb_err_t err);

#endif // __LWPB_H__
