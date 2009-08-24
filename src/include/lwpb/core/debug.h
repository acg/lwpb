/**
 * @file debug.h
 * 
 * Debugging macros.
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

#ifndef __LWPB_CORE_DEBUG_H__
#define __LWPB_CORE_DEBUG_H__

#include <lwpb/lwpb.h>


/** Simple assert macro */
#define LWPB_ASSERT(_expr_, _msg_)                                          \
    do {                                                                    \
        if (!(_expr_)) {                                                    \
            LWPB_DIAG_PRINTF(_msg_ "\n");                                   \
            LWPB_ABORT();                                                   \
        }                                                                   \
    } while (0)

/** Simple failure macro */
#define LWPB_FAIL(_msg_)                                                    \
    do {                                                                    \
        LWPB_DIAG_PRINTF(_msg_ "\n");                                       \
        LWPB_ABORT();                                                       \
    } while(0)

/* Logging macros */
#define LWPB_DEBUG(_format_, _args_...) \
    LWPB_DIAG_PRINTF("DBG: " _format_ "\n", ##_args_)
#define LWPB_INFO(_format_, _args_...) \
    LWPB_DIAG_PRINTF("INF: " _format_ "\n", ##_args_)
#define LWPB_ERR(_format_, _args_...) \
    LWPB_DIAG_PRINTF("ERR: " _format_ "\n", ##_args_)

#endif // __LWPB_CORE_DEBUG_H__
