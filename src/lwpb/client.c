/**
 * @file client.c
 * 
 * Implementation of the protocol buffers service client.
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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lwpb/lwpb.h>
#include <lwpb/client.h>

#include "private.h"

/**
 * Initializes the client.
 * @param client Client
 * @param service Service implementation
 */
void lwpb_client_init(struct lwpb_client *client, struct lwpb_service *service)
{
    client->service = service;
    client->arg = NULL;
    client->request_handler = NULL;
    client->response_handler = NULL;
    client->call_done_handler = NULL;
}

/**
 * Sets the user argument to be passed back with the handlers.
 * @param client Client
 * @param arg User argument
 */
void lwpb_client_arg(struct lwpb_client *client, void *arg)
{
    client->arg = arg;
}

/**
 * Sets the request and response message handlers.
 * @param client Client
 * @param request_handler Request message handler
 * @param response_handler Response message handler
 * @param call_done_handler_t Call done handler 
 */
void lwpb_client_handler(struct lwpb_client *client,
                         lwpb_client_request_handler_t request_handler,
                         lwpb_client_response_handler_t response_handler,
                         lwpb_client_call_done_handler_t call_done_handler)
{
    client->request_handler = request_handler;
    client->response_handler = response_handler;
    client->call_done_handler = call_done_handler;
}



lwpb_err_t lwpb_client_call(struct lwpb_client *client,
                            const struct lwpb_method_desc *method_desc)
{
    LWPB_ASSERT(client->service, "Service implementation missing");
    LWPB_ASSERT(client->request_handler, "Request handler missing");
    LWPB_ASSERT(client->response_handler, "Response handler missing");
    LWPB_ASSERT(client->call_done_handler, "Call done handler missing");
    
    // Dispatch the call to the service implementation
    return client->service->client_funs->call(client->service, client, method_desc);
}
