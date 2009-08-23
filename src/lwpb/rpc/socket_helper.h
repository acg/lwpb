/**
 * @file socket_helper.h
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

#ifndef __LWPB_RPC_SOCKET_HELPER_H__
#define __LWPB_RPC_SOCKET_HELPER_H__

#include <lwpb/lwpb.h>


typedef enum {
    MSG_TYPE_REQUEST = 0,
    MSG_TYPE_RESPONSE = 1,
} protocol_msg_type_t;

struct protocol_header_info {
    protocol_msg_type_t msg_type;
    const struct lwpb_service_desc *service_desc;
    const struct lwpb_method_desc *method_desc;
    size_t header_len;
    size_t msg_len;
    const struct lwpb_service_desc **_service_list;
};

int send_request(int socket, const struct lwpb_method_desc *method_desc,
                 void *req_buf, size_t req_len);

int send_response(int socket, const struct lwpb_method_desc *method_desc,
                  void *res_buf, size_t res_len);

typedef enum {
    PARSE_ERR_OK,
    PARSE_ERR_END_OF_BUF,
    PARSE_ERR_INVALID_MAGIC,
} protocol_parse_err_t;

protocol_parse_err_t parse_request(void *buf, size_t len,
                                   struct protocol_header_info *info,
                                   const struct lwpb_service_desc **service_list);

#endif // __LWPB_RPC_SOCKET_HELPER_H__
