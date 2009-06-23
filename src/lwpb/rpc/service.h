/**
 * @file service.h
 * 
 * Lightweight protocol buffers RPC service interface.
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

#ifndef __LWPB_RPC_SERVICE_H__
#define __LWPB_RPC_SERVICE_H__

#include <stdint.h>
#include <stdlib.h>

#include <lwpb/lwpb.h>

/* Forward declaration */
struct lwpb_client;
struct lwpb_server;
struct lwpb_service;

/** RPC service implementation handle */
typedef struct lwpb_service *lwpb_service_t;

/** RPC service allocator functions */
struct lwpb_allocator_funs {
    /**
     * This method is called to allocate a new buffer.
     * @param service Service implementation
     * @param buf Pointer to buffer
     * @param buf Pointer to length of buffer
     * @return Returns LWPB_ERR_OK when successful or LWPB_ERR_MEM if memory
     * could not be allocated.
     */
    lwpb_err_t (*alloc_buf)(lwpb_service_t service, void **buf, size_t *len);
    
    /**
     * This method is called to free a buffer.
     * @param service Service implementation
     * @param buf Buffer
     */
    void (*free_buf)(lwpb_service_t service, void *buf);
};

/** RPC service functions */
struct lwpb_service_funs {
    /**
     * This method is called from the client when it is registered with the
     * service.
     * @param service Service implementation
     * @param client Client
     */
    void (*register_client)(lwpb_service_t service, struct lwpb_client *client);
    /**
     * This method is called from the client to start an RPC call.
     * @param service Service implementation
     * @param client Client
     * @param method_desc Method descriptor
     * @return Returns LWPB_ERR_OK if successful.
     */
    lwpb_err_t (*call)(lwpb_service_t service,
                       struct lwpb_client *client,
                       const struct lwpb_method_desc *method_desc);
    /**
     * This method is called from the client when the current RPC call should
     * be cancelled.
     * @param service Service implementation
     * @param client Client
     */
    void (*cancel)(lwpb_service_t service,
                   struct lwpb_client *client);
    /**
     * This method is called from the server when it is registered with the
     * service.
     * @param service Service implementation
     * @param server Server
     */
    void (*register_server)(lwpb_service_t service, struct lwpb_server *server);
};

/** RPC service base structure */
struct lwpb_service {
    const struct lwpb_allocator_funs *allocator_funs;
    const struct lwpb_service_funs *service_funs;
};

void lwpb_service_init(lwpb_service_t service,
                       const struct lwpb_service_funs *service_funs);

void lwpb_service_set_allocator(lwpb_service_t service,
                                const struct lwpb_allocator_funs *allocator_funs);

lwpb_err_t lwpb_service_alloc_buf(lwpb_service_t service,
                                  void **buf, size_t *len);

void lwpb_service_free_buf(lwpb_service_t service, void *buf);

#endif // __LWPB_RPC_SERVICE_H__
