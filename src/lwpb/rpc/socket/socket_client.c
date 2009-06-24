/**
 * @file socket_client.c
 * 
 * Socket client RPC service implementation.
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
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#include <lwpb/lwpb.h>
#include <lwpb/rpc/socket/socket_client.h>

#include "protocol.h"


/**
 * Makes a socket non-blocking.
 * @param sock
 */
static void make_nonblock(int sock)
{
    int opts;

    opts = fcntl(sock, F_GETFL);
    if (opts < 0)
        LWPB_FAIL("fcntl(F_GETFL)");
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL,opts) < 0)
        LWPB_FAIL("fcntl(F_SETFL)");
}

/**
 * This method is called from the client when it is registered with the
 * service.
 * @param service Service implementation
 * @param client Client
 */
static void service_register_client(lwpb_service_t service,
                                    struct lwpb_client *client)
{
    struct lwpb_service_socket_client *socket_client =
        (struct lwpb_service_socket_client *) service;

    LWPB_ASSERT(!socket_client->client, "Only one client can be registered");
    
    socket_client->client = client;
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
    struct lwpb_service_socket_client *socket_client =
        (struct lwpb_service_socket_client *) service;
    lwpb_err_t ret = LWPB_ERR_OK;
    void *req_buf = NULL;
    size_t req_len;
    
    // Allocate a buffer for the request message
    ret = lwpb_service_alloc_buf(service, &req_buf, &req_len);
    if (ret != LWPB_ERR_OK)
        goto out;
    
    // Encode the request message
    ret = client->request_handler(client, method_desc, method_desc->req_desc,
                                  req_buf, &req_len, client->arg);
    if (ret != LWPB_ERR_OK)
        goto out;
    
    // Only continue if connected to server
    if (socket_client->socket == -1) {
        client->done_handler(client, method_desc,
                             LWPB_RPC_NOT_CONNECTED, client->arg);
        goto out;
    }
    
    // Send the request to the server
    // TODO
out:
    // Free allocated requiest message buffer
    if (req_buf)
        lwpb_service_free_buf(service, req_buf);
    
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
    LWPB_FAIL("No servers can be registered");
}

static const struct lwpb_service_funs service_funs = {
        .register_client = service_register_client,
        .call = service_call,
        .cancel = service_cancel,
        .register_server = service_register_server,
};





/**
 * Initializes the socket client service implementation.
 * @param socket_client Socket client service data
 * @return Returns the service handle.
 */
lwpb_service_t lwpb_service_socket_client_init(
        struct lwpb_service_socket_client *socket_client)
{
    int i;
    
    LWPB_DEBUG("Initializing socket client");
    
    lwpb_service_init(&socket_client->super, &service_funs);
    
    socket_client->client = NULL;
    socket_client->socket = -1;
    
    return &socket_client->super;
}

/**
 * Opens the socket client for communication.
 * @param service Service handle
 * @param host Hostname or IP address (using local address if NULL)
 * @param port Port number for listen port
 * @return Returns LWPB_ERR_OK if successful.
 */
lwpb_err_t lwpb_service_socket_client_open(lwpb_service_t service,
                                           const char *host, uint16_t port)
{
    struct lwpb_service_socket_client *socket_client =
        (struct lwpb_service_socket_client *) service;
    lwpb_err_t ret = LWPB_ERR_OK;
    int status;
    struct addrinfo hints;
    struct addrinfo *res;
    struct sockaddr_in *addr;
    char tmp[16];
    int yes = 1;
    
    if (socket_client->socket != -1) {
        LWPB_INFO("Socket client already opened");
        return LWPB_ERR_OK;
    }

    // Resolve hostname
    LWPB_DEBUG("Resolving hostname '%s'", host);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_STREAM;    // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;        // Fill in the IP
    snprintf(tmp, sizeof(tmp), "%d", port);
    if ((status = getaddrinfo(host, tmp, &hints, &res)) != 0) {
        LWPB_ERR("getaddrinfo error: %s\n", gai_strerror(status));
        ret = LWPB_ERR_NET_INIT;
        goto out;
    }
    addr = (struct sockaddr_in *) res->ai_addr;
    inet_ntop(res->ai_family, &addr->sin_addr, tmp, sizeof(tmp));
    
    // Create client socket
    LWPB_DEBUG("Creating client socket");
    socket_client->socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socket_client->socket == -1) {
        LWPB_ERR("Cannot create client socket");
        ret = LWPB_ERR_NET_INIT;
        goto out;
    }
    
    // Reuse address if necessary
    if (setsockopt(socket_client->socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        LWPB_ERR("Cannot set SO_REUSEADDR (error: %d)", errno);
        ret = LWPB_ERR_NET_INIT;
        goto out;
    }
    
    // Connect to server
    LWPB_DEBUG("Connecting to %s:%d", tmp, port);
    if (connect(socket_client->socket, res->ai_addr, res->ai_addrlen) == -1) {
        LWPB_ERR("Cannot open connection (errno: %d)", errno);
        ret = LWPB_ERR_NET_INIT;
        goto out;
    }
    
    // Make non-blocking
    make_nonblock(socket_client->socket);
    
out:
    freeaddrinfo(res);
    
    return ret;
}

/**
 * Closes the socket client.
 * @param service Service handle
 */
void lwpb_service_socket_client_close(lwpb_service_t service)
{
    struct lwpb_service_socket_client *socket_client =
        (struct lwpb_service_socket_client *) service;
    int i;
    
    if (socket_client->socket == -1)
        return;
    
    // Close socket
    close(socket_client->socket);
    socket_client->socket == -1;
}

/**
 * Updates the socket client. This method needs to be called periodically.
 * @param service Service handle
 */
lwpb_err_t lwpb_service_socket_client_update(lwpb_service_t service)
{
    struct lwpb_service_socket_client *socket_client =
        (struct lwpb_service_socket_client *) service;
    int i;
    int socket;
    struct timeval timeout;
    fd_set read_fds;
    int high;
    
    if (socket_client->socket == -1)
        return;
    
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    // Create set of active sockets
    high = socket_client->socket;
    FD_ZERO(&read_fds);
    FD_SET(socket_client->socket, &read_fds);
    
    // Wait for socket to get active
    i = select(high + 1, &read_fds, NULL, NULL, &timeout);
    if (i < 0)
        LWPB_FAIL("select() failed");
    if (i == 0)
        return LWPB_ERR_OK;
    
    // Handle data
    
    
    return LWPB_ERR_OK;
}
