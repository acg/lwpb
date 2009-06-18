/**
 * @file misc.c
 * 
 * Misc functions.
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

/**
 * Returns a textual description of an lwpb error code.
 * @param err Error code
 * @return Returns the textual description.
 */
const char *lwpb_err_text(lwpb_err_t err)
{
    switch (err) {
    case LWPB_ERR_OK: return "OK";
    case LWPB_ERR_UNKNOWN_FIELD: return "Unknown field";
    case LWPB_ERR_END_OF_BUF: return "End of buffer";
    default:
        return "Unknown errorcode";
    }
}
