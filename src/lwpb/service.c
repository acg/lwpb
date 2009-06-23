/**
 * @file service.c
 * 
 * Implementation of the protocol buffers service.
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
#include <lwpb/service.h>

// Generic allocator implementation

static lwpb_err_t generic_alloc_buf(void **buf, size_t *len)
{
    *len = 1024;
    *buf = malloc(*len);
    
    return LWPB_ERR_OK;
}

static void generic_free_buf(void *buf)
{
    free(buf);
};

static const struct lwpb_allocator_funs generic_allocator_funs = {
    .alloc_buf = generic_alloc_buf,
    .free_buf = generic_free_buf,
};

// Generic client service implementation

static lwpb_err_t generic_client_call(struct lwpb_service *service,
                                      struct lwpb_client *client,
                                      const struct lwpb_method_desc *method_desc)
{
    lwpb_err_t ret;
    void *buf;
    size_t len;
    
    // Allocate a data buffer
    ret = lwpb_service_alloc_buf(service, &buf, &len);
    if (ret != LWPB_ERR_OK)
        return ret;

    // Encode the request message
    ret = client->request_handler(client, method_desc, method_desc->req_desc,
                                  buf, &len, client->arg);
    if (ret != LWPB_ERR_OK) {
        // Free the data buffer
        lwpb_service_free_buf(service, buf);
        return ret;
    }
    
    // Send the request message and free the data buffer
    ret = service->client_funs->send_request(service, client, method_desc, buf, len);
    lwpb_service_free_buf(service, buf);
    return ret;
}

static lwpb_err_t generic_client_send_request(struct lwpb_service *service,
                                              struct lwpb_client *client,
                                              const struct lwpb_method_desc *method_desc,
                                              void *buf, size_t len)
{
    lwpb_err_t ret;
    void *res_buf;
    size_t res_len;
    
    // Allocate response buffer
    ret = lwpb_service_alloc_buf(service, &res_buf, &res_len);
    if (ret != LWPB_ERR_OK)
        return ret;
    
    // Process the request on the server
    ret = service->server->call_handler(service->server, method_desc,
                                        method_desc->req_desc, buf, len,
                                        method_desc->res_desc, res_buf, &res_len,
                                        service->server->arg);
    if (ret != LWPB_ERR_OK) {
        lwpb_service_free_buf(service, res_buf);
        return ret;
    }
    
    // Handle the response in the client
    ret = client->response_handler(client, method_desc,
                                   method_desc->res_desc, res_buf, res_len,
                                   client->arg);
    lwpb_service_free_buf(service, res_buf);
    return ret;
}

static const struct lwpb_client_funs generic_client_funs = {
    .call = generic_client_call,
    .send_request = generic_client_send_request,
};

// Generic server service implementation

static const struct lwpb_server_funs generic_server_funs = {
        
};


void lwpb_service_init(struct lwpb_service *service,
                       const struct lwpb_allocator_funs *allocator_funs,
                       struct lwpb_client *client,
                       const struct lwpb_client_funs *client_funs,
                       struct lwpb_server *server,
                       const struct lwpb_server_funs *server_funs)
{
    service->allocator_funs = allocator_funs ? allocator_funs : &generic_allocator_funs;
    service->client = client;
    service->client_funs = client_funs ? client_funs : &generic_client_funs;
    service->server = server;
    service->server_funs = server_funs ? server_funs : &generic_server_funs;
}

lwpb_err_t lwpb_service_alloc_buf(struct lwpb_service *service,
                                  void **buf, size_t *len)
{
    return service->allocator_funs->alloc_buf(buf, len);
}

void lwpb_service_free_buf(struct lwpb_service *service, void *buf)
{
    service->allocator_funs->free_buf(buf);
}
