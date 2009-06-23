/**
 * @file service_impl.h
 * 
 * Lightweight protocol buffers service interface.
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

#ifndef __LWPB_SERVICE_IMPL_H__
#define __LWPB_SERVICE_IMPL_H__

#include <stdint.h>
#include <stdlib.h>

#include <lwpb/lwpb.h>
#include <lwpb/client.h>
#include <lwpb/server.h>

/* Forward declaration */
struct lwpb_client;
struct lwpb_server;
struct lwpb_service;

struct lwpb_allocator_funs {
    lwpb_err_t (*alloc_buf)(void **buf, size_t *len);
    void (*free_buf)(void *buf);
};

struct lwpb_client_funs {
    lwpb_err_t (*call)(struct lwpb_service *service,
                       struct lwpb_client *client,
                       const struct lwpb_method_desc *method_desc);
    lwpb_err_t (*send_request)(struct lwpb_service *service,
                               struct lwpb_client *client,
                               const struct lwpb_method_desc *method_desc,
                               void *buf, size_t len);
};

struct lwpb_server_funs {
    lwpb_err_t (*response)(struct lwpb_service *service,
                           const struct lwpb_method_desc *method_desc,
                           const struct lwpb_msg_desc *msg_desc,
                           void *buf, size_t len);
};

struct lwpb_service {
    const struct lwpb_allocator_funs *allocator_funs;
    struct lwpb_client *client;
    const struct lwpb_client_funs *client_funs;
    struct lwpb_server *server;
    const struct lwpb_server_funs *server_funs;
};

void lwpb_service_init(struct lwpb_service *service,
                       const struct lwpb_allocator_funs *allocator_funs,
                       struct lwpb_client *client,
                       const struct lwpb_client_funs *client_funs,
                       struct lwpb_server *server,
                       const struct lwpb_server_funs *server_funs);

lwpb_err_t lwpb_service_alloc_buf(struct lwpb_service *service,
                                  void **buf, size_t *len);

void lwpb_service_free_buf(struct lwpb_service *service, void *buf);

#endif // __LWPB_SERVICE_IMPL_H__
