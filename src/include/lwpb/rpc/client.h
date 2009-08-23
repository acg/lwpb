/**
 * @file client.h
 * 
 * Lightweight protocol buffers RPC client interface.
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

#ifndef __LWPB_RPC_CLIENT_H__
#define __LWPB_RPC_CLIENT_H__

#include <lwpb/lwpb.h>


/* Forward declaration */
struct lwpb_client;

/**
 * This handler is called when the client needs to encode the request message
 * of an RPC call.
 * @param client Client
 * @param method_desc Method descriptor
 * @param msg_desc Request message descriptor
 * @param buf Request message buffer
 * @param len Length of request message buffer
 * @param arg User argument
 * @return Return LWPB_ERR_OK when message was successfully encoded.
 */
typedef lwpb_err_t (*lwpb_client_request_handler_t)
    (struct lwpb_client *client, const struct lwpb_method_desc *method_desc,
     const struct lwpb_msg_desc *msg_desc, void *buf, size_t *len, void *arg);

/**
 * This handler is called when the client needs to decode the response message
 * of an RPC call.
 * @param client Client
 * @param method_desc Method descriptor
 * @param msg_desc Response message descriptor
 * @param buf Response message buffer
 * @param len Length of response message buffer
 * @param arg User argument
 * @return Return LWPB_ERR_OK when message was successfully decoded.
 */
typedef lwpb_err_t (*lwpb_client_response_handler_t)
    (struct lwpb_client *client, const struct lwpb_method_desc *method_desc,
     const struct lwpb_msg_desc *msg_desc, void *buf, size_t len, void *arg);

/**
 * This handler is called when the execution of an RPC call is done.
 * @param client Client
 * @param method_desc Method descriptor
 * @param result Result code
 * @param arg User argument
 */
typedef void (*lwpb_client_done_handler_t)
    (struct lwpb_client *client, const struct lwpb_method_desc *method_desc,
     lwpb_rpc_result_t result, void *arg);


/** Protocol buffer RPC client */
struct lwpb_client {
    lwpb_transport_t transport;
    void *arg;
    lwpb_client_request_handler_t request_handler;
    lwpb_client_response_handler_t response_handler;
    lwpb_client_done_handler_t done_handler;
};

void lwpb_client_init(struct lwpb_client *client, lwpb_transport_t transport);

void lwpb_client_arg(struct lwpb_client *client, void *arg);

void lwpb_client_handler(struct lwpb_client *client,
                         lwpb_client_request_handler_t request_handler,
                         lwpb_client_response_handler_t response_handler,
                         lwpb_client_done_handler_t done_handler);

lwpb_err_t lwpb_client_call(struct lwpb_client *client,
                            const struct lwpb_method_desc *method_desc);

void lwpb_client_cancel(struct lwpb_client *client);

#endif // __LWPB_RPC_CLIENT_H__
