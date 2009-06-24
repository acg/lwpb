/**
 * @file socket_server.c
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#include <lwpb/lwpb.h>
#include <lwpb/rpc/socket/socket_server.h>

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
 * Accepts new connections on the listen socket.
 * @param socket_server Socket server
 */
static void handle_new_connection(struct lwpb_service_socket_server *socket_server)
{
    int socket;
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    char tmp[16];
    struct sockaddr_in *addr_in;
    int i;
    const char msg[] = "No more connections allowed\n";
    
    socket = accept(socket_server->socket, (struct sockaddr *) &addr, &len);
    if (socket < 0) {
        LWPB_ERR("Accepting new socket failed (errno: %d)", errno);
        return;
    }
    
    make_nonblock(socket);
    
    for (i = 0; i < LWPB_SERVICE_SOCKET_SERVER_CONNECTIONS; i++)
        if (socket_server->client_sockets[i] == -1) {
            addr_in = (struct sockaddr_in *) &addr;
            inet_ntop(addr.ss_family, &addr_in->sin_addr, tmp, sizeof(tmp));
            LWPB_DEBUG("Client(%d) accepted conncetion from %s", i, tmp);
            socket_server->client_sockets[i] = socket;
            socket_server->num_clients++;
            return;
        }
    
    // No more connections allowed
    LWPB_DEBUG("No more connections allowed");
    send(socket, msg, sizeof(msg), 0);
    close(socket);
}

/**
 * Closes a client connection.
 * @param socket_server Socket server
 * @param index Client index
 */
static void close_connection(struct lwpb_service_socket_server *socket_server,
                             int index)
{
    int socket = socket_server->client_sockets[index];

    LWPB_DEBUG("Client(%d) disconnected", index);
    close(socket);
    socket_server->client_sockets[index] = -1;
    socket_server->num_clients--;
}

/**
 * Handles incoming data on a client connection.
 * @param socket_server Socket server
 * @param index Client index
 */
static void handle_connection(struct lwpb_service_socket_server *socket_server,
                              int index)
{
    int socket = socket_server->client_sockets[index];
    uint8_t buf[1024];
    ssize_t len;
    
    len = recv(socket, buf, sizeof(buf), 0);
    if (len <= 0) {
        close_connection(socket_server, index);
        return;
    }
    
    LWPB_DEBUG("Client(%d) received %d bytes", index, len);
    
    send(socket, buf, len, 0);
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
    LWPB_FAIL("No clients can be registered");
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
    struct lwpb_service_socket_server *socket_server =
        (struct lwpb_service_socket_server *) service;
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
    if (!socket_server->server) {
        client->done_handler(client, method_desc,
                             LWPB_RPC_NOT_CONNECTED, client->arg);
        goto out;
    }
    
    // Process the call on the server
    ret = socket_server->server->call_handler(socket_server->server, method_desc,
                                              method_desc->req_desc, req_buf, req_len,
                                              method_desc->res_desc, res_buf, &res_len,
                                              socket_server->server->arg);
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
    struct lwpb_service_socket_server *socket_server =
        (struct lwpb_service_socket_server *) service;
    
    LWPB_ASSERT(!socket_server->server, "Only one server can be registered");
    
    socket_server->server = server;
}

static const struct lwpb_service_funs service_funs = {
        .register_client = service_register_client,
        .call = service_call,
        .cancel = service_cancel,
        .register_server = service_register_server,
};

/**
 * Initializes the socket server service implementation.
 * @param socket_server Socket server service data
 * @return Returns the service handle.
 */
lwpb_service_t lwpb_service_socket_server_init(
        struct lwpb_service_socket_server *socket_server)
{
    int i;
    
    LWPB_DEBUG("Initializing socket server");
    
    lwpb_service_init(&socket_server->super, &service_funs);
    
    socket_server->server = NULL;
    socket_server->socket = -1;
    socket_server->num_clients = 0;
    for (i = 0; i < LWPB_SERVICE_SOCKET_SERVER_CONNECTIONS; i++)
        socket_server->client_sockets[i] = -1;
    
    return &socket_server->super;
}

/**
 * Opens the socket server for communication.
 * @param service Service handle
 * @param host Hostname or IP address (using local address if NULL)
 * @param port Port number for listen port
 * @return Returns LWPB_ERR_OK if successful.
 */
lwpb_err_t lwpb_service_socket_server_open(lwpb_service_t service,
                                           const char *host, uint16_t port)
{
    struct lwpb_service_socket_server *socket_server =
        (struct lwpb_service_socket_server *) service;
    lwpb_err_t ret = LWPB_ERR_OK;
    int status;
    struct addrinfo hints;
    struct addrinfo *res;
    struct sockaddr_in *addr;
    char tmp[16];
    int yes = 1;
    
    if (socket_server->socket != -1) {
        LWPB_INFO("Socket server already opened");
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
    
    // Create server socket
    LWPB_DEBUG("Creating server socket");
    socket_server->socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socket_server->socket == -1) {
        LWPB_ERR("Cannot create server socket");
        ret = LWPB_ERR_NET_INIT;
        goto out;
    }
    
    // Reuse address if necessary
    if (setsockopt(socket_server->socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        LWPB_ERR("Cannot set SO_REUSEADDR (error: %d)", errno);
        ret = LWPB_ERR_NET_INIT;
        goto out;
    }
    
    // Make non-blocking
    make_nonblock(socket_server->socket);
    
    // Bind listen socket
    LWPB_DEBUG("Binding server socket to %s:%d", tmp, port);
    if (bind(socket_server->socket, res->ai_addr, res->ai_addrlen) == -1) {
        LWPB_ERR("Cannot bind server socket (errno: %d)", errno);
        ret = LWPB_ERR_NET_INIT;
        goto out;
    }

    // Start listening
    LWPB_DEBUG("Start listening on server socket");
    if (listen(socket_server->socket, LWPB_SERVICE_SOCKET_SERVER_CONNECTIONS) == -1) {
        LWPB_ERR("Cannot listen on server socket (errno: %d)", errno);
        close(socket_server->socket);
        ret = LWPB_ERR_NET_INIT;
        goto out;
    }
    
out:
    freeaddrinfo(res);
    
    return ret;
}

/**
 * Closes the socket server.
 * @param service Service handle
 */
void lwpb_service_socket_server_close(lwpb_service_t service)
{
    struct lwpb_service_socket_server *socket_server =
        (struct lwpb_service_socket_server *) service;
    int i;
    
    if (socket_server->socket == -1)
        return;
    
    // Close active connections
    for (i = 0; i < LWPB_SERVICE_SOCKET_SERVER_CONNECTIONS; i++)
        if (socket_server->client_sockets[i] != -1)
            close_connection(socket_server, i);
        
    // Close listen socket
    close(socket_server->socket);
    socket_server->socket == -1;
}

/**
 * Updates the socket server. This method needs to be called periodically.
 * @param service Service handle
 */
lwpb_err_t lwpb_service_socket_server_update(lwpb_service_t service)
{
    struct lwpb_service_socket_server *socket_server =
        (struct lwpb_service_socket_server *) service;
    int i;
    int socket;
    struct timeval timeout;
    fd_set read_fds;
    int high;
    
    if (socket_server->socket == -1)
        return;
    
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    // Create set of active sockets
    high = socket_server->socket;
    FD_ZERO(&read_fds);
    FD_SET(socket_server->socket, &read_fds);
    for (i = 0; i < LWPB_SERVICE_SOCKET_SERVER_CONNECTIONS; i++)
        if (socket_server->client_sockets[i] != -1) {
            FD_SET(socket_server->client_sockets[i], &read_fds);
            high = socket_server->client_sockets[i];
        }
    
    // Wait for a socket to get active
    i = select(high + 1, &read_fds, NULL, NULL, &timeout);
    if (i < 0)
        LWPB_FAIL("select() failed");
    if (i == 0)
        return LWPB_ERR_OK;
    
    // Accept new connections
    if (FD_ISSET(socket_server->socket, &read_fds))
        handle_new_connection(socket_server);
    
    // Handle active connections
    for (i = 0; i < LWPB_SERVICE_SOCKET_SERVER_CONNECTIONS; i++) {
        if (socket_server->client_sockets[i] != -1 &&
            FD_ISSET(socket_server->client_sockets[i], &read_fds))
            handle_connection(socket_server, i);
    }
    
    return LWPB_ERR_OK;
}
