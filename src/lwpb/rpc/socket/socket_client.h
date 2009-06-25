/**
 * @file socket_client.h
 * 
 * Socket client RPC transport implementation.
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

#ifndef __LWPB_RPC_SOCKET_CLIENT_H__
#define __LWPB_RPC_SOCKET_CLIENT_H__

#include <stdint.h>

#include <lwpb/lwpb.h>

/** Socket client RPC transport implementation */
struct lwpb_transport_socket_client {
    struct lwpb_transport super;
    struct lwpb_client *client;
    int socket;
    void *buf;
    void *pos;
    size_t len;
    const struct lwpb_method_desc *last_method;
};

lwpb_transport_t lwpb_transport_socket_client_init(struct lwpb_transport_socket_client *socket_client);

lwpb_err_t lwpb_transport_socket_client_open(lwpb_transport_t transport,
                                             const char *host, uint16_t port);

void lwpb_transport_socket_client_close(lwpb_transport_t transport);

lwpb_err_t lwpb_transport_socket_client_update(lwpb_transport_t transport);

#endif // __LWPB_RPC_SOCKET_CLIENT_H__
