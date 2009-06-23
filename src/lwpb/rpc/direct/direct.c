/**
 * @file direct.c
 * 
 * Direct RPC service implementation.
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
#include <stdio.h>

#include <lwpb/lwpb.h>
#include <lwpb/rpc/direct/direct.h>


/**
 * This method is called from the client when it is registered with the
 * service.
 * @param service Service implementation
 * @param client Client
 */
static void service_register_client(lwpb_service_t service,
                                    struct lwpb_client *client)
{
    struct lwpb_service_direct *direct = (struct lwpb_service_direct *) service;
    
    LWPB_ASSERT(!direct->client, "Only one client can be registered");
    
    direct->client = client;
}

/**
 * This method is called from the client to start an RPC call.
 * @param service Service implementation
 * @param client Client
 * @param method_desc Method descriptor
 * @return Returns LWPB_ERR_OK if successful.
 */
static lwpb_err_t service_call(lwpb_service_t service,
                               struct lwpb_client *client,
                               const struct lwpb_method_desc *method_desc)
{
    struct lwpb_service_direct *direct = (struct lwpb_service_direct *) service;
    lwpb_err_t ret = LWPB_ERR_OK;
    void *req_buf = NULL;
    size_t req_len;
    void *res_buf = NULL;
    size_t res_len;
    
    // Allocate a buffer for the request message
    ret = lwpb_service_alloc_buf(service, &req_buf, &req_len);
    if (ret != LWPB_ERR_OK)
        goto out;
    
    // Allocate a buffer for the response message
    ret = lwpb_service_alloc_buf(service, &res_buf, &res_len);
    if (ret != LWPB_ERR_OK)
        goto out;

    // Encode the request message
    ret = client->request_handler(client, method_desc, method_desc->req_desc,
                                  req_buf, &req_len, client->arg);
    if (ret != LWPB_ERR_OK)
        goto out;
    
    // We need a registered server to continue
    if (!direct->server) {
        client->done_handler(client, method_desc,
                             LWPB_RPC_NOT_CONNECTED, client->arg);
        goto out;
    }
    
    // Process the call on the server
    ret = direct->server->call_handler(direct->server, method_desc,
                                       method_desc->req_desc, req_buf, req_len,
                                       method_desc->res_desc, res_buf, &res_len,
                                       direct->server->arg);
    if (ret != LWPB_ERR_OK) {
        client->done_handler(client, method_desc,
                             LWPB_RPC_FAILED, client->arg);
        goto out;
    }
    
    // Process the response in the client
    ret = client->response_handler(client, method_desc,
                                   method_desc->res_desc, res_buf, res_len,
                                   client->arg);
    
    client->done_handler(client, method_desc, LWPB_RPC_OK, client->arg);
    
out:
    // Free allocated message buffers
    if (req_buf)
        lwpb_service_free_buf(service, req_buf);
    if (res_buf)
        lwpb_service_free_buf(service, res_buf);
    
    return ret;
}

/**
 * This method is called from the client when the current RPC call should
 * be cancelled.
 * @param service Service implementation
 * @param client Client
 */
static void service_cancel(lwpb_service_t service,
                           struct lwpb_client *client)
{
    // Cancel is not supported in this service implementation.
}

/**
 * This method is called from the server when it is registered with the
 * service.
 * @param service Service implementation
 * @param server Server
 */
static void service_register_server(lwpb_service_t service,
                                    struct lwpb_server *server)
{
    struct lwpb_service_direct *direct = (struct lwpb_service_direct *) service;
    
    LWPB_ASSERT(!direct->server, "Only one server can be registered");
    
    direct->server = server;
}

static const struct lwpb_service_funs service_funs = {
        .register_client = service_register_client,
        .call = service_call,
        .cancel = service_cancel,
        .register_server = service_register_server,
};

/**
 * Initializes the direct service implementation.
 * @param service_direct Direct service data
 * @return Returns the service implementation handle.
 */
lwpb_service_t lwpb_service_direct_init(struct lwpb_service_direct *service_direct)
{
    lwpb_service_init(&service_direct->super, &service_funs);
    
    service_direct->client = NULL;
    service_direct->server = NULL;
    
    return &service_direct->super;
}
