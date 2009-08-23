/**
 * @file socket_helper.c
 * 
 * Helper functions for the socket RPC protocol.
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

#include <string.h>

#include <lwpb/lwpb.h>

#include "socket_helper.h"
#include "socket_protocol_pb2.h"


#define PROTOCOL_MAGIC 0xdeadbeaf

struct pre_header {
    u32_t magic;
    u32_t header_len;
    u32_t msg_len;
};


int send_request(int socket, const struct lwpb_method_desc *method_desc,
                 void *req_buf, size_t req_len)
{
    struct lwpb_encoder encoder;
    struct pre_header pre_header;
    u8_t header[128];
    size_t len;
    
    lwpb_encoder_init(&encoder);
    lwpb_encoder_start(&encoder, socket_protocol_Header, header, sizeof(header));
    lwpb_encoder_add_enum(&encoder, socket_protocol_Header_type, SOCKET_PROTOCOL_REQUEST);
    lwpb_encoder_add_string(&encoder, socket_protocol_Header_service, (char *) method_desc->service->name);
    lwpb_encoder_add_string(&encoder, socket_protocol_Header_method, (char *) method_desc->name);
    len = lwpb_encoder_finish(&encoder);
    
    pre_header.magic = htonl(PROTOCOL_MAGIC);
    pre_header.header_len = htonl(len);
    pre_header.msg_len = htonl(req_len);

    send(socket, &pre_header, sizeof(pre_header), 0);
    send(socket, header, len, 0);
    send(socket, req_buf, req_len, 0);
}

int send_response(int socket, const struct lwpb_method_desc *method_desc,
                  void *res_buf, size_t res_len)
{
    struct lwpb_encoder encoder;
    struct pre_header pre_header;
    u8_t header[128];
    size_t len;
    
    lwpb_encoder_init(&encoder);
    lwpb_encoder_start(&encoder, socket_protocol_Header, header, sizeof(header));
    lwpb_encoder_add_enum(&encoder, socket_protocol_Header_type, SOCKET_PROTOCOL_RESPONSE);
    len = lwpb_encoder_finish(&encoder);
    
    pre_header.magic = htonl(PROTOCOL_MAGIC);
    pre_header.header_len = htonl(len);
    pre_header.msg_len = htonl(res_len);

    send(socket, &pre_header, sizeof(pre_header), 0);
    send(socket, header, len, 0);
    send(socket, res_buf, res_len, 0);
}

static void parse_request_field_handler(struct lwpb_decoder *decoder,
                                        const struct lwpb_msg_desc *msg_desc,
                                        const struct lwpb_field_desc *field_desc,
                                        union lwpb_value *value, void *arg)
{
    struct protocol_header_info *header_info = arg;
    int i;
    
    if (msg_desc != socket_protocol_Header)
        return;

    if (field_desc == socket_protocol_Header_type)
        header_info->msg_type = value->enum_;
    
    if (field_desc == socket_protocol_Header_service && header_info->_service_list) {
        // Try to identify the service from the services list
        const struct lwpb_service_desc **service;
        for (service = header_info->_service_list; *service != NULL; *service++) {
            if (value->string.len != strlen((*service)->name))
                continue;
            if (strncmp(value->string.str, (*service)->name, value->string.len) == 0) {
                header_info->service_desc = *service;
                break;
            }
        }
    }
    
    if (field_desc == socket_protocol_Header_method && header_info->_service_list) {
        if (header_info->service_desc) {
            // Try to identify the method
            for (i = 0; i < header_info->service_desc->num_methods; i++) {
                const struct lwpb_method_desc *method = &header_info->service_desc->methods[i];
                if (value->string.len != strlen(method->name))
                    continue;
                if (strncmp(value->string.str, method->name, value->string.len) == 0) {
                    header_info->method_desc = method;
                    break;
                }
            }
        }
    }
}

protocol_parse_err_t parse_request(void *buf, size_t len,
                                   struct protocol_header_info *info,
                                   const struct lwpb_service_desc **service_list)
{
    struct lwpb_decoder decoder;
    struct pre_header *pre_header;
    u32_t header_len;
    
    if (len < sizeof(struct pre_header))
        return PARSE_ERR_END_OF_BUF;
    
    pre_header = buf;

    // Check magic
    if (ntohl(pre_header->magic) != PROTOCOL_MAGIC)
        return PARSE_ERR_INVALID_MAGIC;

    // Check header and message length
    header_len = ntohl(pre_header->header_len);
    info->header_len = header_len + sizeof(struct pre_header);
    info->msg_len = ntohl(pre_header->msg_len);
    if (len < info->header_len + info->msg_len)
        return PARSE_ERR_END_OF_BUF;
    
    // Decode header
    buf += sizeof(struct pre_header);
    
    info->_service_list = service_list;
    info->msg_type = 0;
    info->service_desc = NULL;
    info->method_desc = NULL;
    
    lwpb_decoder_init(&decoder);
    lwpb_decoder_arg(&decoder, info);
    lwpb_decoder_field_handler(&decoder, parse_request_field_handler);
    return lwpb_decoder_decode(&decoder, socket_protocol_Header, buf, header_len, NULL);
}
