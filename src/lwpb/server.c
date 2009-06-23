/**
 * @file server.c
 * 
 * Implementation of the protocol buffers service server.
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
#include <lwpb/server.h>

#include "core/private.h"

/**
 * Initializes the server.
 * @param server Server
 * @param service Service implementation
 */
void lwpb_server_init(struct lwpb_server *server, struct lwpb_service *service)
{
    server->service = service;
    server->arg = NULL;
    server->request_handler = NULL;
}

/**
 * Sets the user argument to be passed back with the handlers.
 * @param server Server
 * @param arg User argument
 */
void lwpb_server_arg(struct lwpb_server *server, void *arg)
{
    server->arg = arg;
}

/**
 * Sets the request and response message handlers.
 * @param server Server
 * @param request_handler Request message handler
 */
void lwpb_server_handler(struct lwpb_server *server,
                         lwpb_server_request_handler_t request_handler)
{
    server->request_handler = request_handler;
}
