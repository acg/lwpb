#ifndef __LWPB_PY_DECODER_H__
#define __LWPB_PY_DECODER_H__

#include "Python.h"
#include <lwpb/lwpb.h>

/* Decoder object */

typedef struct {
  PyObject_HEAD
  struct lwpb_decoder decoder;
} Decoder;

extern PyTypeObject DecoderType;

#define Decoder_Check(v)  (Py_TYPE(v) == &DecoderType)

#endif

