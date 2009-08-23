/**
 * @file server.h
 * 
 * Lightweight protocol buffers RPC server interface.
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

#ifndef __LWPB_RPC_SERVER_H__
#define __LWPB_RPC_SERVER_H__

#include <lwpb/lwpb.h>


/* Forward declaration */
struct lwpb_server;

/**
 * This handler is called when the server needs to process an RPC call.
 * @param server Server
 * @param method_desc Method descriptor
 * @param req_desc Request message descriptor
 * @param req_buf Request message buffer
 * @param req_len Length of request message buffer
 * @param res_desc Response message descriptor
 * @param res_buf Response message buffer
 * @param res_len Length of response message buffer
 * @param arg User argument
 * @return Return LWPB_ERR_OK when message was successfully encoded.
 */
typedef lwpb_err_t (*lwpb_server_call_handler_t)
    (struct lwpb_server *server, const struct lwpb_method_desc *method_desc,
     const struct lwpb_msg_desc *req_desc, void *req_buf, size_t req_len,
     const struct lwpb_msg_desc *res_desc, void *res_buf, size_t *res_len,
     void *arg);

/** Protocol buffer RPC server */
struct lwpb_server {
    const struct lwpb_service_desc **service_list;
    struct lwpb_transport *transport;
    void *arg;
    lwpb_server_call_handler_t call_handler;
};

void lwpb_server_init(struct lwpb_server *server,
                      const struct lwpb_service_desc **service_list,
                      struct lwpb_transport *transport);

void lwpb_server_arg(struct lwpb_server *server, void *arg);

void lwpb_server_handler(struct lwpb_server *server,
                         lwpb_server_call_handler_t call_handler);

#endif // __LWPB_RPC_SERVER_H__
