/**
 * @file socket_server.h
 * 
 * Socket server RPC service implementation.
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

#ifndef __LWPB_RPC_SOCKET_SERVER_H__
#define __LWPB_RPC_SOCKET_SERVER_H__

#include <stdint.h>

#include <lwpb/lwpb.h>

#define LWPB_SERVICE_SOCKET_SERVER_CONNS 4

/** A single client connection in the socket server */
struct lwpb_service_socket_server_conn {
    int index;
    int socket;
    struct lwpb_client client;
    void *buf;
    void *pos;
    size_t len;
};

/** Socket server RPC service implementation */
struct lwpb_service_socket_server {
    struct lwpb_service super;
    struct lwpb_server *server;
    int socket;
    int num_conns;
    struct lwpb_service_socket_server_conn conns[LWPB_SERVICE_SOCKET_SERVER_CONNS];
};

lwpb_service_t lwpb_service_socket_server_init(struct lwpb_service_socket_server *socket_server);

lwpb_err_t lwpb_service_socket_server_open(lwpb_service_t service,
                                           const char *host, uint16_t port);

void lwpb_service_socket_server_close(lwpb_service_t service);

lwpb_err_t lwpb_service_socket_server_update(lwpb_service_t service);

#endif // __LWPB_RPC_SOCKET_SERVER_H__
