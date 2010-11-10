#ifndef __LWPB_PY_PRIMITIVES_H__
#define __LWPB_PY_PRIMITIVES_H__

#include "Python.h"
#include <lwpb/lwpb.h>

PyObject*
lwpb_to_py(union lwpb_value *p, unsigned int type);

int
py_to_lwpb(union lwpb_value* p, PyObject *val, unsigned int type);

#endif

