/**
 * @file client.h
 * 
 * Lightweight protocol buffers service client interface.
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

#ifndef __LWPB_CLIENT_H__
#define __LWPB_CLIENT_H__

#include <stdint.h>
#include <stdlib.h>

#include <lwpb/lwpb.h>
#include <lwpb/service.h>

/** RPC call results. */
typedef enum {
    LWPB_CALL_OK,
    LWPB_CALL_FAILED,
} lwpb_call_result_t;

/* Forward declaration */
struct lwpb_client;

/**
 * This handler is called when the client needs to encode the request message
 * of an RPC call.
 * @param client Client
 * @param method_desc Method descriptor
 * @param msg_desc Request message descriptor
 * @param buf Message buffer
 * @param len Length of message buffer
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
 * @param buf Message buffer
 * @param len Length of message buffer
 * @param arg User argument
 * @return Return LWPB_ERR_OK when message was successfully decoded.
 */
typedef lwpb_err_t (*lwpb_client_response_handler_t)
    (struct lwpb_client *client, const struct lwpb_method_desc *method_desc,
     const struct lwpb_msg_desc *msg_desc, void *buf, size_t len, void *arg);


typedef void (*lwpb_client_call_done_handler_t)
    (struct lwpb_client *client, const struct lwpb_method_desc *method_desc,
     lwpb_call_result_t result, void *arg);


/** Protocol buffer service client */
struct lwpb_client {
    struct lwpb_service *service;
    void *arg;
    lwpb_client_request_handler_t request_handler;
    lwpb_client_response_handler_t response_handler;
    lwpb_client_call_done_handler_t call_done_handler;
};

void lwpb_client_init(struct lwpb_client *client, struct lwpb_service *service);

void lwpb_client_arg(struct lwpb_client *client, void *arg);

void lwpb_client_handler(struct lwpb_client *client,
                         lwpb_client_request_handler_t request_handler,
                         lwpb_client_response_handler_t response_handler,
                         lwpb_client_call_done_handler_t call_done_handler);

lwpb_err_t lwpb_client_call(struct lwpb_client *client,
                            const struct lwpb_method_desc *method_desc);

#endif // __LWPB_CLIENT_H__
