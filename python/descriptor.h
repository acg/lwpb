#ifndef __LWPB_PY_DESCRIPTOR_H__
#define __LWPB_PY_DESCRIPTOR_H__

#include "Python.h"
#include <lwpb/lwpb.h>

/* Descriptor class */

typedef struct {
  PyObject_HEAD
  PyObject* strings;
  PyObject* message_types;
  unsigned int num_msgs;
  struct lwpb_msg_desc *msg_desc;
} Descriptor;

extern PyTypeObject DescriptorType;

#define Descriptor_Check(v)  (Py_TYPE(v) == &DescriptorType)

#endif
