/** @file socket_server.h
 * 
 * Socket server RPC transport implementation.
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

#include <lwpb/lwpb.h>


#define LWPB_TRANSPORT_SOCKET_SERVER_CONNS 4

/** A single client connection in the socket server */
struct lwpb_socket_server_conn {
    int index;
    int socket;
    struct lwpb_client client;
    void *buf;
    void *pos;
    size_t len;
};

/** Socket server RPC transport implementation */
struct lwpb_transport_socket_server {
    struct lwpb_transport super;
    struct lwpb_server *server;
    int socket;
    int num_conns;
    struct lwpb_socket_server_conn conns[LWPB_TRANSPORT_SOCKET_SERVER_CONNS];
};

lwpb_transport_t lwpb_transport_socket_server_init(struct lwpb_transport_socket_server *socket_server);

lwpb_err_t lwpb_transport_socket_server_open(lwpb_transport_t transport,
                                             const char *host, u16_t port);

void lwpb_transport_socket_server_close(lwpb_transport_t transport);

lwpb_err_t lwpb_transport_socket_server_update(lwpb_transport_t transport);

#endif // __LWPB_RPC_SOCKET_SERVER_H__
