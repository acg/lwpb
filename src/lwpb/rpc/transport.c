/**
 * @file transport.c
 * 
 * Implementation of the protocol buffers RPC transport.
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

// Default allocator implementation

static lwpb_err_t default_alloc_buf(lwpb_transport_t transport,
                                    void **buf, size_t *len)
{
    *len = 1024;
    *buf = malloc(*len);
    
    return LWPB_ERR_OK;
}

static void default_free_buf(lwpb_transport_t transport, void *buf)
{
    free(buf);
};

static const struct lwpb_allocator_funs default_allocator_funs = {
    .alloc_buf = default_alloc_buf,
    .free_buf = default_free_buf,
};



void lwpb_transport_init(lwpb_transport_t transport,
                         const struct lwpb_transport_funs *transport_funs)
{
    transport->allocator_funs = &default_allocator_funs;
    transport->transport_funs = transport_funs;
}

void lwpb_transport_set_allocator(lwpb_transport_t transport,
                                  const struct lwpb_allocator_funs *allocator_funs)
{
    transport->allocator_funs = allocator_funs;
}

lwpb_err_t lwpb_transport_alloc_buf(lwpb_transport_t transport,
                                    void **buf, size_t *len)
{
    return transport->allocator_funs->alloc_buf(transport, buf, len);
}

void lwpb_transport_free_buf(lwpb_transport_t transport, void *buf)
{
    transport->allocator_funs->free_buf(transport, buf);
}
