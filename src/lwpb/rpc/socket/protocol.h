/**
 * @file protocol.h
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
    MSG_REQUEST,
    MSG_RESPONSE,
} rpc_message_t;

struct rpc_header {
    rpc_message_t msg;  /**< Message typ */
    uint32_t method;    /**< Method id */
    uint32_t len;       /**< Payload length */
};

#endif // __LWPB_RPC_SOCKET_PROTOCOL_H__
