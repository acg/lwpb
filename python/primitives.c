#include "primitives.h"
#include "Python.h"
#include <lwpb/lwpb.h>


/* Utility functions: scalar type conversion */

PyObject*
lwpb_to_py(union lwpb_value *p, unsigned int type)
{
  switch(type) {
    default:
      PyErr_SetString(PyExc_RuntimeError, "internal: unexpected type");
      return NULL;
    case LWPB_DOUBLE:
      return PyFloat_FromDouble(p->double_);
    case LWPB_FLOAT:
      return PyFloat_FromDouble(p->float_);
    case LWPB_INT64:
    case LWPB_SINT64:
    case LWPB_SFIXED64:
      return PyLong_FromLongLong(p->int64);
    case LWPB_UINT64:
    case LWPB_FIXED64:
      return PyLong_FromUnsignedLongLong(p->uint64);
    case LWPB_SFIXED32:
    case LWPB_SINT32:
    case LWPB_INT32:
    case LWPB_ENUM:
#if PY_MAJOR_VERSION >= 3
      return PyLong_FromLong(p->int32);
#else
      return PyInt_FromLong(p->int32);
#endif
    case LWPB_FIXED32:
    case LWPB_UINT32:
      return PyLong_FromLong(p->uint32);
    case LWPB_BOOL:
      return PyBool_FromLong(p->bool);
    case LWPB_STRING:
    case LWPB_BYTES:
      /* Py3k will distinguish between these two. */
      return PyString_FromStringAndSize(p->string.str, p->string.len);
    case LWPB_MESSAGE: {
      PyErr_SetString(PyExc_NotImplementedError, "only converts primitives");
      return NULL;
    }
  }
}

static long
convert_to_long(PyObject *val, long lobound, long hibound, int *ok)
{
  PyObject *o = PyNumber_Int(val);
  if(!o) {
    PyErr_SetString(PyExc_OverflowError, "could not convert to long");
    *ok = 0;
    return -1;
  }
  long longval = PyInt_AS_LONG(o);
  if(longval > hibound || longval < lobound) {
    PyErr_SetString(PyExc_OverflowError, "value outside type bounds");
    *ok = 0;
    return -1;
  }
  *ok = 1;
  return longval;
}

int
py_to_lwpb(union lwpb_value* p, PyObject *val, unsigned int type)
{
  switch(type) {
    default:
      PyErr_SetString(PyExc_RuntimeError, "internal error");
      return -1;
    case LWPB_DOUBLE: {
      PyObject *o = PyNumber_Float(val);
      if(!o) {
        PyErr_SetString(PyExc_ValueError, "could not convert to double");
        return -1;
      }
      p->double_ = PyFloat_AS_DOUBLE(o);
      return 0;
    }
    case LWPB_FLOAT: {
      PyObject *o = PyNumber_Float(val);
      if(!o) {
        PyErr_SetString(PyExc_ValueError, "could not convert to float");
        return -1;
      }
      p->float_ = PyFloat_AS_DOUBLE(o);
      return 0;
    }
    case LWPB_INT64:
    case LWPB_SINT64:
    case LWPB_SFIXED64: {
#if LONG_MAX >= INT64_MAX
      int ok;
      long longval = convert_to_long(val, INT64_MIN, INT64_MAX, &ok);
      if (!ok) return -1;
      p->int32 = longval;
      return 0;
#else
      PyObject *o = PyNumber_Long(val);
      if(!o) {
        PyErr_SetString(PyExc_ValueError, "could not convert to int64");
        return -1;
      }
      p->int64 = PyLong_AsLongLong(o);
      return 0;
#endif
    }
    case LWPB_UINT64:
    case LWPB_FIXED64: {
      PyObject *o = PyNumber_Long(val);
      if(!o) {
        PyErr_SetString(PyExc_ValueError, "could not convert to uint64");
        return -1;
      }
      p->uint64 = PyLong_AsUnsignedLongLong(o);
      return 0;
    }
    case LWPB_SFIXED32:
    case LWPB_SINT32:
    case LWPB_INT32:
    case LWPB_ENUM: {
      int ok;
      long longval = convert_to_long(val, INT32_MIN, INT32_MAX, &ok);
      if (!ok) return -1;
      p->int32 = longval;
      return 0;
    }

    case LWPB_FIXED32:
    case LWPB_UINT32: {
#if LONG_MAX >= UINT32_MAX
      int ok;
      long longval = convert_to_long(val, 0, UINT32_MAX, &ok);
      if (!ok) return -1;
      p->int32 = longval;
      return 0;
#else
      PyObject *o = PyNumber_Long(val);
      if(!o) {
        PyErr_SetString(PyExc_ValueError, "could not convert to uint32");
        return -1;
      }
      p->uint32 = PyLong_AsUnsignedLong(o);
      return 0;
#endif
    }

    case LWPB_BOOL:
      if(!PyBool_Check(val)) {
        PyErr_SetString(PyExc_ValueError, "should be true or false");
        return -1;
      }
      if(val == Py_True) p->bool = 1;
      else if(val == Py_False) p->bool = 0;
      else {
        PyErr_SetString(PyExc_RuntimeError, "not true or false?");
        return -1;
      }
      return 0;

    case LWPB_STRING:
    case LWPB_BYTES: {
      char *str;
      Py_ssize_t len;
      if (PyString_AsStringAndSize(val, &str, &len) < 0)
        return -1;
      p->string.str = str;
      p->string.len = len;
      return 0;
    }
  }
}

