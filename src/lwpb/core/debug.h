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

#include <stdint.h>
#include <stdlib.h>

/* Simple assert macro */
#define LWPB_ASSERT(expr, msg)                                              \
    do {                                                                    \
        if (!(expr)) {                                                      \
            printf(msg "\n");                                               \
            exit(0);                                                        \
        }                                                                   \
    } while (0)

/* Simple failure macro */
#define LWPB_FAIL(msg)                                                      \
    do {                                                                    \
        printf(msg "\n");                                                   \
        exit(0);                                                            \
    } while(0)

/* Logging macros */
#define LWPB_DEBUG(format, args...) printf("DBG: " format "\n", ##args)
#define LWPB_INFO(format, args...) printf("INF: " format "\n", ##args)
#define LWPB_ERR(format, args...) printf("ERR: " format "\n", ##args)

#endif // __LWPB_CORE_DEBUG_H__
