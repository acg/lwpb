#ifndef __LWPB_PY_ENCODER_H__
#define __LWPB_PY_ENCODER_H__

#include "Python.h"
#include <lwpb/lwpb.h>
#include <lwpb/core/encoder2.h>

/* Encoder object */

typedef struct {
  PyObject_HEAD
  struct lwpb_encoder2 encoder;
} Encoder;

extern PyTypeObject EncoderType;

#define Encoder_Check(v)  (Py_TYPE(v) == &EncoderType)

#endif

