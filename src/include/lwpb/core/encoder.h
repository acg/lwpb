/** @file encoder.h
 * 
 * Lightweight protocol buffers encoder interface.
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

#ifndef __LWPB_CORE_ENCODER_H__
#define __LWPB_CORE_ENCODER_H__

#include <lwpb/lwpb.h>

/** Encoder stack frame */
struct lwpb_encoder_stack_frame {
    struct lwpb_buf buf;
    const struct lwpb_field_desc *field_desc;
    const struct lwpb_msg_desc *msg_desc;
};

/** Protocol buffer encoder */
struct lwpb_encoder {
    struct lwpb_encoder_stack_frame stack[LWPB_MAX_DEPTH];
    int depth;
    lwpb_bool_t packed;
};

void lwpb_encoder_init(struct lwpb_encoder *encoder);

void lwpb_encoder_start(struct lwpb_encoder *encoder,
                        const struct lwpb_msg_desc *msg_desc,
                        void *data, size_t len);

size_t lwpb_encoder_finish(struct lwpb_encoder *encoder);

lwpb_err_t lwpb_encoder_nested_start(struct lwpb_encoder *encoder,
                                     const struct lwpb_field_desc *field_desc);

lwpb_err_t lwpb_encoder_nested_end(struct lwpb_encoder *encoder);

lwpb_err_t lwpb_encoder_packed_repeated_start(struct lwpb_encoder *encoder,
                                              const struct lwpb_field_desc *field_desc);

lwpb_err_t lwpb_encoder_packed_repeated_end(struct lwpb_encoder *encoder);

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
                                  s32_t int32);

lwpb_err_t lwpb_encoder_add_uint32(struct lwpb_encoder *encoder,
                                   const struct lwpb_field_desc *field_desc,
                                   u32_t uint32);

lwpb_err_t lwpb_encoder_add_int64(struct lwpb_encoder *encoder,
                                  const struct lwpb_field_desc *field_desc,
                                  s64_t int64);

lwpb_err_t lwpb_encoder_add_uint64(struct lwpb_encoder *encoder,
                                   const struct lwpb_field_desc *field_desc,
                                   u64_t uint64);

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
                                  u8_t *data, size_t len);


#endif // __LWPB_CORE_ENCODER_H__
