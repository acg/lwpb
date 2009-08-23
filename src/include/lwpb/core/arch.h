/**
 * @file arch.h
 * 
 * Architecture specifics.
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

#ifndef __LWPB_CORE_ARCH_H__
#define __LWPB_CORE_ARCH_H__

#include <lwpb/arch/cc.h>
#include <lwpb/arch/sys.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

// Provide default implementations if none are given

#ifndef LWPB_MALLOC
#define LWPB_MALLOC(size) __lwpb_malloc(size)
#endif

#ifndef LWPB_FREE
#define LWPB_FREE(ptr) __lwpb_free(ptr)
#endif

#ifndef LWPB_MEMCPY
#define LWPB_MEMCPY(dest, src, n) __lwpb_memcpy(dest, src, n)
#endif

#ifndef LWPB_MEMMOVE
#define LWPB_MEMMOVE(dest, src, n) __lwpb_memmove(dest, src, n)
#endif

#ifndef LWPB_STRLEN
#define LWPB_STRLEN(s) __lwpb_strlen(s)
#endif

#ifndef LWPB_DIAG_PRINTF
#define LWPB_DIAG_PRINTF(fmt, args...)
#endif

#ifndef LWPB_EXIT
#define LWPB_EXIT() LWPB_DIAG_PRINTF("!!! A fatail failure was hit, system is unusable !!!\n")
#endif

#endif // __LWPB_CORE_ARCH_H__
