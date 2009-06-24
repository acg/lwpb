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

#define LWPB_SERVICE_SOCKET_SERVER_CONNECTIONS 4

/** Socket server RPC service implementation */
struct lwpb_service_socket_server {
    struct lwpb_service super;
    struct lwpb_server *server;
    int socket;
    int num_clients;
    struct lwpb_client clients[LWPB_SERVICE_SOCKET_SERVER_CONNECTIONS];
    int client_sockets[LWPB_SERVICE_SOCKET_SERVER_CONNECTIONS];
    int terminate;
};

lwpb_service_t lwpb_service_socket_server_init(
        struct lwpb_service_socket_server *service_socket_server);

lwpb_err_t lwpb_service_socket_server_bind(lwpb_service_t service,
                                           const char *host, uint16_t port);

lwpb_err_t lwpb_service_socket_server_run(lwpb_service_t service);

void lwpb_service_socket_server_terminate(lwpb_service_t service);


#endif // __LWPB_RPC_SOCKET_SERVER_H__
