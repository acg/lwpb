/**
 * @file direct.c
 * 
 * Direct RPC transport implementation.
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
#include <lwpb/rpc/direct.h>


/**
 * This method is called from the client when it is registered with the
 * transport.
 * @param transport Transport implementation
 * @param client Client
 */
static void transport_register_client(lwpb_transport_t transport,
                                    struct lwpb_client *client)
{
    struct lwpb_transport_direct *direct = (struct lwpb_transport_direct *) transport;
    
    LWPB_ASSERT(!direct->client, "Only one client can be registered");
    
    direct->client = client;
}

/**
 * This method is called from the client to start an RPC call.
 * @param transport Transport implementation
 * @param client Client
 * @param method_desc Method descriptor
 * @return Returns LWPB_ERR_OK if successful.
 */
static lwpb_err_t transport_call(lwpb_transport_t transport,
                               struct lwpb_client *client,
                               const struct lwpb_method_desc *method_desc)
{
    struct lwpb_transport_direct *direct = (struct lwpb_transport_direct *) transport;
    lwpb_err_t ret = LWPB_ERR_OK;
    void *req_buf = NULL;
    size_t req_len;
    void *res_buf = NULL;
    size_t res_len;
    
    // Allocate a buffer for the request message
    ret = lwpb_transport_alloc_buf(transport, &req_buf, &req_len);
    if (ret != LWPB_ERR_OK)
        goto out;
    
    // Allocate a buffer for the response message
    ret = lwpb_transport_alloc_buf(transport, &res_buf, &res_len);
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
        lwpb_transport_free_buf(transport, req_buf);
    if (res_buf)
        lwpb_transport_free_buf(transport, res_buf);
    
    return ret;
}

/**
 * This method is called from the client when the current RPC call should
 * be cancelled.
 * @param transport Transport implementation
 * @param client Client
 */
static void transport_cancel(lwpb_transport_t transport,
                           struct lwpb_client *client)
{
    // Cancel is not supported in this transport implementation.
}

/**
 * This method is called from the server when it is registered with the
 * transport.
 * @param transport Transport implementation
 * @param server Server
 */
static void transport_register_server(lwpb_transport_t transport,
                                    struct lwpb_server *server)
{
    struct lwpb_transport_direct *direct = (struct lwpb_transport_direct *) transport;
    
    LWPB_ASSERT(!direct->server, "Only one server can be registered");
    
    direct->server = server;
}

static const struct lwpb_transport_funs transport_funs = {
        .register_client = transport_register_client,
        .call = transport_call,
        .cancel = transport_cancel,
        .register_server = transport_register_server,
};

/**
 * Initializes the direct transport implementation.
 * @param transport_direct Direct transport data
 * @return Returns the transport implementation handle.
 */
lwpb_transport_t lwpb_transport_direct_init(struct lwpb_transport_direct *transport_direct)
{
    lwpb_transport_init(&transport_direct->super, &transport_funs);
    
    transport_direct->client = NULL;
    transport_direct->server = NULL;
    
    return &transport_direct->super;
}
