/**
 * @file utils.c
 * 
 * Implementation of some utility functions.
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

#include <lwpb/lwpb.h>

void *__lwpb_malloc(size_t size)
{
    
}

void __lwpb_free(void *ptr)
{
    
}

void *__lwpb_memcpy(void *dest, const void *src, size_t n)
{
    u8_t *d = dest;
    const u8_t *s = src;
    
    while (n--)
        *d++ = *s++;
    
    return dest;
}

void *__lwpb_memmove(void *dest, const void *src, size_t n)
{
    u8_t *d = dest;
    const u8_t *s = src;
    size_t i;

    if (d == s)
        return dest;
    
    if (d < s) {
        for (i = 0; i < n; i++)
            d[i] = s[i];
    } else {
        for (i = n-1; i >= 0; i--)
            d[i] = s[i];
    }
    
    return dest;
}

size_t __lwpb_strlen(const char *s)
{
    size_t len = 0;
    
    while (*s++ != '\0')
        len++;
    
    return len;
}
