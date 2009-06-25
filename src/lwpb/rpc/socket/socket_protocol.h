/**
 * @file socket_protocol.h
 * 
 * Definitions for the socket RPC protocol.
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

#ifndef __LWPB_RPC_SOCKET_PROTOCOL_H__
#define __LWPB_RPC_SOCKET_PROTOCOL_H__

#include <stdint.h>

#include <lwpb/lwpb.h>

typedef enum {
    SOCKET_PROTOCOL_REQUEST = 0,
    SOCKET_PROTOCOL_RESPONSE = 1,
} socket_protocol_msg_type_t;

struct socket_protocol_header_info {
    socket_protocol_msg_type_t msg_type;
    struct lwpb_service_desc *service_desc;
    struct lwpb_method_desc *method_desc;
    size_t len;
};

int send_request(int socket, const struct lwpb_method_desc *method_desc,
                 void *req_buf, size_t req_len);

int parse_request(void *buf, size_t len,
                  struct socket_protocol_header_info *header_info);

#endif // __LWPB_RPC_SOCKET_PROTOCOL_H__
