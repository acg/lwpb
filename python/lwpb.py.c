#include "Python.h"
#include <lwpb/lwpb.h>
#include <lwpb/core/encoder2.h>  // FIXME


/* ----------------------------------------- */

/* Utility functions: scalar type conversion */

static PyObject*
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

static int
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



/* ----------------------------------------- */

static PyObject *ErrorObject;

/* ----------------------------------------- */

/* Descriptor class */

typedef struct {
  PyObject_HEAD
  PyObject* strings;
  unsigned int num_msgs;
  struct lwpb_msg_desc *msg_desc;
} Descriptor;

static PyTypeObject DescriptorType;

#define Descriptor_Check(v)  (Py_TYPE(v) == &DescriptorType)


/* Descriptor methods */

static PyObject *
Descriptor_new(PyTypeObject *type, PyObject *arg, PyObject *kwds)
{
  Descriptor *self;
  
  if (!(self = (Descriptor*)type->tp_alloc(type, 0)))
    return NULL;

  if (!(self->strings = PyList_New(0)))
    return NULL;

  self->num_msgs = 0;
  self->msg_desc = NULL;

  return (PyObject *)self;
}


/* NB: This method is not exposed to Python. */
static void
Descriptor_clear(Descriptor *self)
{
  unsigned int i;

  /* Free all field descriptors. */

  for (i=0; i<self->num_msgs; i++)
  {
    struct lwpb_msg_desc* m = &self->msg_desc[i];

    if (m->fields){
      free((void*)m->fields);
      m->fields = NULL;
    }
  }
 
  /* Free all message descriptors. */

  if (self->msg_desc) {
    free((void*)self->msg_desc);
    self->msg_desc = NULL;
    self->num_msgs = 0;
  }

  /* Clear the string table. */

  if (self->strings)
    PyList_SetSlice(self->strings, 0, PyList_Size(self->strings), NULL);
}


static void
Descriptor_dealloc(Descriptor *self)
{
  Descriptor_clear(self);
  Py_DECREF(self->strings);
  self->ob_type->tp_free((PyObject*)self);
}


/* NB: This method is not exposed to Python. */
char*
Descriptor_store_string(Descriptor *self, PyObject* str)
{
  PyObject* copy = PyString_FromStringAndSize(PyString_AsString(str), PyString_Size(str));
  PyList_Append(self->strings, copy);
  return PyString_AsString(copy);
}


static int
Descriptor_init(Descriptor *self, PyObject *arg, PyObject *kwds)
{
  int error = 0;

  PyObject* dict;
  PyObject* prop;
  PyObject* msgtypes;
  PyObject* msgtype;
  PyObject* fields;
  PyObject* field;
  PyObject* options;
  Py_ssize_t msgtypes_len;
  Py_ssize_t fields_len;
  Py_ssize_t i;
  Py_ssize_t j;
  Py_ssize_t k;

  /* We are expecting a dictionary containing a key "message_type"
     with a list of message descriptor structures. */

  if (!PyArg_ParseTuple(arg, "O!:init", &PyDict_Type, &dict))
    return -1;

  if (!(msgtypes = PyDict_GetItemString(dict, "message_type")))
    return 0;

  if (!PyList_Check(msgtypes)) {
    PyErr_SetString(PyExc_RuntimeError, "message_type: list expected");
    return -1;
  }

  const char* package = "";

  if ((prop = PyDict_GetItemString(dict, "package")) && PyString_Check(prop))
    package = PyString_AsString(prop);

  if (!(msgtypes_len = PyList_Size(msgtypes)))
    goto init_cleanup;

  if (!(self->msg_desc = (struct lwpb_msg_desc*)calloc(msgtypes_len, sizeof(struct lwpb_msg_desc)))) {
    PyErr_SetString(PyExc_MemoryError, "unable to allocate message descriptors");
    error = -1;
    goto init_cleanup;
  }

  self->num_msgs = msgtypes_len;

  int pass;

  for (pass=0; pass<2; pass++)
  {
    for (i=0; i<msgtypes_len; i++)
    {
      struct lwpb_msg_desc* m = &self->msg_desc[i];

      if (!(msgtype = PyList_GetItem(msgtypes, i)) || !PyDict_Check(msgtype))
        continue;

      if (!(fields = PyDict_GetItemString(msgtype, "field")) || !PyList_Check(fields))
        continue;

      fields_len = PyList_Size(fields);

      if (pass == 0)
      {
        /* First pass: just allocate enough field descriptors, and fill in
           any message descriptor properties. */

        if (fields_len)
          if (!(m->fields = (struct lwpb_field_desc*)calloc(fields_len, sizeof(struct lwpb_field_desc)))) {
            PyErr_SetString(PyExc_MemoryError, "unable to allocate field descriptors");
            error = -1;
            goto init_cleanup;
          }

        m->num_fields = fields_len;

        if ((prop = PyDict_GetItemString(msgtype, "name")) && PyString_Check(prop))
          m->name = Descriptor_store_string(self, prop);
      }
      else if (pass == 1)
      {
        /* Second pass: fill in all field descriptors and their properties,
           Resolve TYPE_MESSAGE fields to their message descriptors. */

        for (j=0; j<m->num_fields; j++)
        {
          struct lwpb_field_desc* f = (struct lwpb_field_desc*)&m->fields[j];

          if (!(field = PyList_GetItem(fields, j)) || !PyDict_Check(field))
            continue;

          if ((prop = PyDict_GetItemString(field, "number")) && PyInt_Check(prop))
            f->number = PyInt_AsLong(prop);

          if ((prop = PyDict_GetItemString(field, "label")) && PyInt_Check(prop))
            f->opts.label = PyInt_AsLong(prop);

          if ((prop = PyDict_GetItemString(field, "type")) && PyInt_Check(prop))
            f->opts.typ = PyInt_AsLong(prop);

          if ((options = PyDict_GetItemString(field, "options")) && PyDict_Check(options))
          {
            if (PyDict_GetItemString(options, "packed") == Py_True)
              f->opts.flags |= LWPB_IS_PACKED;

            if (PyDict_GetItemString(options, "deprecated") == Py_True)
              f->opts.flags |= LWPB_IS_DEPRECATED;
          }

          if ((prop = PyDict_GetItemString(field, "name")) && PyString_Check(prop))
            f->name = Descriptor_store_string(self, prop);

          /* Convert the Python default value for a field to an lwpb value.
             If it's a string, make sure we create and use a private copy. */

          if ((prop = PyDict_GetItemString(field, "default_value"))) {
            if (!py_to_lwpb(&f->def, prop, f->opts.typ)) {
              f->opts.flags |= LWPB_HAS_DEFAULT;
              if (PyString_Check(prop))
                f->def.string.str = Descriptor_store_string(self, prop);
            }
          }

          /* If "type_name" is set for this field, it names a message
             descriptor fully qualified by package. Search through message
             descriptors for a match. */

          if ((prop = PyDict_GetItemString(field, "type_name")) && PyString_Check(prop)) {
            if (f->opts.typ == LWPB_MESSAGE)
            {
              for (k=0; k<msgtypes_len; k++)
              {
                PyObject* msgtype2;
                PyObject* prop2;
                PyObject* qualified;

                if (!(msgtype2 = PyList_GetItem(msgtypes, k)) || !PyDict_Check(msgtype2))
                  continue;

                if (!(prop2 = PyDict_GetItemString(msgtype2, "name")) || !PyString_Check(prop2))
                  continue;

                if (!(qualified = PyString_FromFormat(".%s.%s", package, PyString_AsString(prop2))))
                  continue;

                int cmp, ret;
                ret = PyObject_Cmp(prop, qualified, &cmp);
                Py_DECREF(qualified);
                
                if (ret >= 0 && cmp == 0) {
                  f->msg_desc = &self->msg_desc[k];
                  break;
                }
              }

              if (!f->msg_desc) {
                PyErr_Format(PyExc_RuntimeError, "could not resolve type_name: %s", PyString_AsString(prop));
                error = -1;
                goto init_cleanup;
              }
            }
          }

        } // for each field
      } // if second pass, set up fields
    } // for each msgtype 
  } // for each pass

init_cleanup:

  if (error) Descriptor_clear(self);
  return error;
}


static PyObject *
Descriptor_debug_print(Descriptor *self, PyObject *args)
{
  unsigned int i;
  unsigned int j;
  
  for (i=0; i<self->num_msgs; i++)
  {
    struct lwpb_msg_desc* m = &self->msg_desc[i];
    if (m->name)
      printf( "name = %s\n", m->name );
    printf( "num_fields = %d\n", m->num_fields );

    for (j=0; j<m->num_fields; j++)
    {
      struct lwpb_field_desc* f = (struct lwpb_field_desc*)&m->fields[j];
      printf( "  number = %d\n", f->number );
      if (f->name)
        printf( "  name = %s\n", f->name );
      printf( "  label = %d\n", f->opts.label );
      printf( "  type = %d\n", f->opts.typ );
      printf( "  flags = %d\n", f->opts.flags );

      if (f->msg_desc && f->msg_desc->name)
        printf( "  message = %s\n", m->name );
      printf( "\n" );
    }

    printf( "\n\n" );
  }
  
  Py_INCREF(Py_None);
  return Py_None;
}


static PyMethodDef Descriptor_methods[] = {
  {"debug_print",  (PyCFunction)Descriptor_debug_print,  METH_VARARGS,
    PyDoc_STR("debug_print() -> None")},
  {NULL,    NULL}    /* sentinel */
};


static PyTypeObject DescriptorType = {
  /* The ob_type field must be initialized in the module init function
   * to be portable to Windows without using C++. */
  PyVarObject_HEAD_INIT(NULL, 0)
  "lwpb.cext.Descriptor",       /*tp_name*/
  sizeof(Descriptor),           /*tp_basicsize*/
  0,                            /*tp_itemsize*/
  /* methods */
  (destructor)Descriptor_dealloc,  /*tp_dealloc*/
  0,                            /*tp_print*/
  0,                            /*tp_getattr*/
  0,                            /*tp_setattr*/
  0,                            /*tp_compare*/
  0,                            /*tp_repr*/
  0,                            /*tp_as_number*/
  0,                            /*tp_as_sequence*/
  0,                            /*tp_as_mapping*/
  0,                            /*tp_hash*/
  0,                            /*tp_call*/
  0,                            /*tp_str*/
  0,                            /*tp_getattro*/
  0,                            /*tp_setattro*/
  0,                            /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  0,                            /*tp_doc*/
  0,                            /*tp_traverse*/
  0,                            /*tp_clear*/
  0,                            /*tp_richcompare*/
  0,                            /*tp_weaklistoffset*/
  0,                            /*tp_iter*/
  0,                            /*tp_iternext*/
  Descriptor_methods,           /*tp_methods*/
  0,                            /*tp_members*/
  0,                            /*tp_getset*/
  0,                            /*tp_base*/
  0,                            /*tp_dict*/
  0,                            /*tp_descr_get*/
  0,                            /*tp_descr_set*/
  0,                            /*tp_dictoffset*/
  (initproc)Descriptor_init,    /*tp_init*/
  0,                            /*tp_alloc*/
  Descriptor_new,               /*tp_new*/
  0,                            /*tp_free*/
  0,                            /*tp_is_gc*/
};



/* --------------------------------------------------------------------- */

/* Decoder object */

typedef struct {
  PyObject_HEAD
  struct lwpb_decoder decoder;
} Decoder;

typedef struct {
  PyObject* stack;
} DecoderContext;

static PyTypeObject DecoderType;

#define Decoder_Check(v)  (Py_TYPE(v) == &DecoderType)


/* Decoder methods */

static PyObject *
Decoder_new(PyTypeObject *type, PyObject *arg, PyObject *kwds)
{
  Decoder *self;
  self = (Decoder *)type->tp_alloc(type, 0);
  if (self == NULL) return NULL;
  return (PyObject *)self;
}


static int
Decoder_init(Decoder *self, PyObject *arg, PyObject *kwds)
{
  return 0;
}


static void
Decoder_dealloc(Decoder *self)
{
  self->ob_type->tp_free((PyObject*)self);
}


/* Decoder handlers */

static void
msg_start_handler(
  struct lwpb_decoder *decoder,
  const struct lwpb_msg_desc *msg_desc,
  void *arg)
{
}

static void
msg_end_handler(
  struct lwpb_decoder *decoder,
  const struct lwpb_msg_desc *msg_desc,
  void *arg)
{
  if (!arg) return;

  DecoderContext* context = (DecoderContext*)arg;

  /* Leaving a nested message, pop the top off the context stack. */

  Py_ssize_t stacklen = PyList_Size(context->stack);
  PyList_SetSlice(context->stack, stacklen-1, stacklen, NULL);
}

void
field_handler(
  struct lwpb_decoder *decoder,
  const struct lwpb_msg_desc *msg_desc,
  const struct lwpb_field_desc *field_desc,
  union lwpb_value *value,
  void *arg)
{
  if (!arg) return;

  DecoderContext* context = (DecoderContext*)arg;

  /* Get the top of the decoder context stack, the current target dict. */

  Py_ssize_t stacklen = PyList_Size(context->stack);
  PyObject* top = PyList_GetItem(context->stack, stacklen-1);
  PyObject* pyval;

  /* If the lwpb value is not NULL, convert it to a pyval. */
  /* Otherwise, if lwpb value is NULL, this signals a nested message. */
  /* Create or acquire a dict to hold the nested message. */

  if (value)
    pyval = lwpb_to_py(value, field_desc->opts.typ);
  else {
    if (field_desc->opts.label == LWPB_REPEATED)
      pyval = PyDict_New();
    else {
      PyObject* curval;
      if ((curval = PyDict_GetItemString(top, field_desc->name))) {
        pyval = curval;
        Py_INCREF(pyval);
      }
      else
        pyval = PyDict_New();
    }
  }

  if (pyval == NULL) return;

  /* If this is a REPEATED field, always append to a list under the key. */
  /* If this is not REPEATED, store under key, overwriting any old value. */

  if (field_desc->opts.label == LWPB_REPEATED) {
    PyObject* list;
    
    if (!(list = PyDict_GetItemString(top, field_desc->name))) {
      list = PyList_New(0);
      PyDict_SetItemString(top, field_desc->name, list);
      Py_DECREF(list);
    }

    PyList_Append(list, pyval);
  }
  else {
    PyDict_SetItemString(top, field_desc->name, pyval);
  }

  /* When the lwpb value is NULL, pyval is a dict, and we are starting a
     new nested message. Push the new target dict onto the stack. */

  if (!value)
    PyList_Append(context->stack, pyval);

  Py_DECREF(pyval);
}

static PyObject *
Decoder_decode(Decoder *self, PyObject *args)
{
  char *buf;
  int len;
  unsigned int msgnum;
  lwpb_err_t ret;
  Descriptor* descriptor;

  if (!PyArg_ParseTuple(args, "s#O!i:decode", &buf, &len, &DescriptorType, &descriptor, &msgnum))
    return NULL;

  if (msgnum >= descriptor->num_msgs) {
    PyErr_SetString(PyExc_IndexError, "message type index out of range");
    return NULL;
  }

  /* Create a new decoder context with an empty stack. */
  /* Push a new target dict onto the stack for the decoding process. */

  DecoderContext context;
  context.stack = PyList_New(0);
  PyObject* dst = PyDict_New();
  PyList_Append(context.stack, dst);

  /* Set up the decoder event handlers. */

  lwpb_decoder_arg(&self->decoder, &context);
  lwpb_decoder_msg_handler(&self->decoder, msg_start_handler, msg_end_handler);
  lwpb_decoder_field_handler(&self->decoder, field_handler);

  /* Perform decoding of the specified message type. */
  
  ret = lwpb_decoder_decode(&self->decoder, &descriptor->msg_desc[msgnum], buf, len, NULL);

  /* Done with the decoder context now. */

  Py_DECREF(context.stack);

  /* On success, return the target dict object.
     On partial message error, return None.
     On any other error, throw an exception with the error code. */

  if (ret == LWPB_ERR_OK)
    return dst;

  Py_DECREF(dst);

  if (ret == LWPB_ERR_END_OF_BUF) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  else {
    // TODO define specific exception classes
    PyErr_Format(PyExc_RuntimeError, "decode error: %d", ret);
    return NULL;
  }
}


static PyObject*
Decoder_decode_varint(Decoder *self, PyObject *args)
{
  char* buf;
  int len;

  if (!PyArg_ParseTuple(args, "s#:decode_varint", &buf, &len))
    return NULL;

  struct lwpb_buf lbuf;
  lwpb_err_t ret;
  u64_t value;
 
  lbuf.base = (u8_t*)buf;
  lbuf.pos  = (u8_t*)buf;
  lbuf.end  = (u8_t*)buf + len;
 
  ret = lwpb_decode_varint(&lbuf, &value);

  PyObject* tuple = PyTuple_New(2);

  switch (ret) {

    case LWPB_ERR_OK:
      PyTuple_SetItem(tuple, 0, PyLong_FromLong(value));
      PyTuple_SetItem(tuple, 1, PyLong_FromLong(lbuf.pos-lbuf.base));
      break;

    case LWPB_ERR_END_OF_BUF:
      PyTuple_SetItem(tuple, 0, Py_None);
      PyTuple_SetItem(tuple, 1, PyLong_FromLong(0));
      break;

    default:
      // TODO define specific exception classes
      Py_DECREF(tuple);
      PyErr_Format(PyExc_RuntimeError, "decode_varint error: %d", ret);
      return NULL;
  }

  return tuple;
}


static PyObject*
Decoder_decode_32bit(Decoder *self, PyObject *args)
{
  char* buf;
  int len;

  if (!PyArg_ParseTuple(args, "s#:decode_32bit", &buf, &len))
    return NULL;

  struct lwpb_buf lbuf;
  lwpb_err_t ret;
  u32_t value;
 
  lbuf.base = (u8_t*)buf;
  lbuf.pos  = (u8_t*)buf;
  lbuf.end  = (u8_t*)buf + len;
 
  ret = lwpb_decode_32bit(&lbuf, &value);

  PyObject* tuple = PyTuple_New(2);

  switch (ret) {

    case LWPB_ERR_OK:
      PyTuple_SetItem(tuple, 0, PyInt_FromLong(value));
      PyTuple_SetItem(tuple, 1, PyLong_FromLong(lbuf.pos-lbuf.base));
      break;

    case LWPB_ERR_END_OF_BUF:
      PyTuple_SetItem(tuple, 0, Py_None);
      PyTuple_SetItem(tuple, 1, PyLong_FromLong(0));
      break;

    default:
      // TODO define specific exception classes
      Py_DECREF(tuple);
      PyErr_Format(PyExc_RuntimeError, "decode_varint error: %d", ret);
      return NULL;
  }

  return tuple;
}


static PyObject*
Decoder_decode_64bit(Decoder *self, PyObject *args)
{
  char* buf;
  int len;

  if (!PyArg_ParseTuple(args, "s#:decode_64bit", &buf, &len))
    return NULL;

  struct lwpb_buf lbuf;
  lwpb_err_t ret;
  u64_t value;
 
  lbuf.base = (u8_t*)buf;
  lbuf.pos  = (u8_t*)buf;
  lbuf.end  = (u8_t*)buf + len;
 
  ret = lwpb_decode_64bit(&lbuf, &value);

  PyObject* tuple = PyTuple_New(2);

  switch (ret) {

    case LWPB_ERR_OK:
      PyTuple_SetItem(tuple, 0, PyLong_FromLong(value));
      PyTuple_SetItem(tuple, 1, PyLong_FromLong(lbuf.pos-lbuf.base));
      break;

    case LWPB_ERR_END_OF_BUF:
      PyTuple_SetItem(tuple, 0, Py_None);
      PyTuple_SetItem(tuple, 1, PyLong_FromLong(0));
      break;

    default:
      // TODO define specific exception classes
      Py_DECREF(tuple);
      PyErr_Format(PyExc_RuntimeError, "decode_64bit error: %d", ret);
      return NULL;
  }

  return tuple;
}


static PyMethodDef Decoder_methods[] = {
  {"decode",  (PyCFunction)Decoder_decode,  METH_VARARGS,
    PyDoc_STR("decode(data,descriptor,msgnum) -> Dict")},
  {"decode_varint",  (PyCFunction)Decoder_decode_varint,  METH_VARARGS,
    PyDoc_STR("decode_varint(data) -> (Number,Bytes)")},
  {"decode_32bit",  (PyCFunction)Decoder_decode_32bit,  METH_VARARGS,
    PyDoc_STR("decode_32bit(data) -> (Number,Bytes)")},
  {"decode_64bit",  (PyCFunction)Decoder_decode_64bit,  METH_VARARGS,
    PyDoc_STR("decode_64bit(data) -> (Number,Bytes)")},
  {NULL,    NULL}    /* sentinel */
};


static PyTypeObject DecoderType = {
  /* The ob_type field must be initialized in the module init function
   * to be portable to Windows without using C++. */
  PyVarObject_HEAD_INIT(NULL, 0)
  "lwpb.cext.Decoder",           /*tp_name*/
  sizeof(Decoder),        /*tp_basicsize*/
  0,                            /*tp_itemsize*/
  /* methods */
  (destructor)Decoder_dealloc,  /*tp_dealloc*/
  0,                            /*tp_print*/
  0,                            /*tp_getattr*/
  0,                            /*tp_setattr*/
  0,                            /*tp_compare*/
  0,                            /*tp_repr*/
  0,                            /*tp_as_number*/
  0,                            /*tp_as_sequence*/
  0,                            /*tp_as_mapping*/
  0,                            /*tp_hash*/
  0,                            /*tp_call*/
  0,                            /*tp_str*/
  0,                            /*tp_getattro*/
  0,                            /*tp_setattro*/
  0,                            /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  0,                            /*tp_doc*/
  0,                            /*tp_traverse*/
  0,                            /*tp_clear*/
  0,                            /*tp_richcompare*/
  0,                            /*tp_weaklistoffset*/
  0,                            /*tp_iter*/
  0,                            /*tp_iternext*/
  Decoder_methods,              /*tp_methods*/
  0,                            /*tp_members*/
  0,                            /*tp_getset*/
  0,                            /*tp_base*/
  0,                            /*tp_dict*/
  0,                            /*tp_descr_get*/
  0,                            /*tp_descr_set*/
  0,                            /*tp_dictoffset*/
  (initproc)Decoder_init,       /*tp_init*/
  0,                            /*tp_alloc*/
  Decoder_new,                  /*tp_new*/
  0,                            /*tp_free*/
  0,                            /*tp_is_gc*/
};

/* --------------------------------------------------------------------- */

/* Encoder object */

typedef struct {
  PyObject_HEAD
  struct lwpb_encoder2 encoder;
} Encoder;

static PyTypeObject EncoderType;

#define Encoder_Check(v)  (Py_TYPE(v) == &EncoderType)


/* Encoder methods */

static PyObject *
Encoder_new(PyTypeObject *type, PyObject *arg, PyObject *kwds)
{
  Encoder *self;
  self = (Encoder *)type->tp_alloc(type, 0);
  if (self == NULL) return NULL;
  return (PyObject *)self;
}


static int
Encoder_init(Encoder *self, PyObject *arg, PyObject *kwds)
{
  lwpb_encoder2_init(&self->encoder);
  return 0;
}


static void
Encoder_dealloc(Encoder *self)
{
  self->ob_type->tp_free((PyObject*)self);
}


#define MSG_RESERVE_BYTES 10

size_t
pyobject_encode(
  PyObject* obj,
  struct lwpb_encoder2 *encoder,
  const struct lwpb_msg_desc *msg_desc,
  u8_t* buf,
  int* error)
{
  size_t len = 0;
  size_t extralen = 0;
  unsigned int i;
  unsigned int j;
  union lwpb_value val;
  PyObject* pyval = NULL;
  PyObject* pylist = NULL;

  /* Python object must be a dict to be encoded as a message. */

  if (!PyDict_Check(obj)) {
    *error = 1; // FIXME more specific error code
    return -1;
  }

  /* Encode each field defined in the message descriptor. */

  for (i=0; i<msg_desc->num_fields; i++)
  {
    u8_t* fieldbuf = buf;
    size_t fieldlen = 0;
    const struct lwpb_field_desc *field_desc = &msg_desc->fields[i];

    /* If the field is packed repeated, enter packed encoder mode
       and reserve enough leading room for the eventual length prefix.  */

    if (LWPB_IS_PACKED_REPEATED(field_desc)) {
      lwpb_encoder2_packed_repeated_start(encoder, field_desc);
      if (fieldbuf) fieldbuf += MSG_RESERVE_BYTES;
      extralen += MSG_RESERVE_BYTES;
    }

    /* Get the python field value.
       If it isn't a list, turn it into a list with one item. */

    pyval = PyDict_GetItemString(obj, field_desc->name);

    if (!pyval) {
      continue;
    }
    else if (PyList_Check(pyval)) {
      pylist = pyval;
      Py_INCREF(pylist);
    }
    else {
      pylist = PyList_New(1);
      Py_INCREF(pyval);
      PyList_SetItem(pylist, 0, pyval);
    }

    /* Encode each python value under this field. */

    u8_t* valuebuf = fieldbuf;
    size_t valuelen = 0;

    Py_ssize_t num_values = PyList_Size(pylist);

    for (j=0; j<num_values; j++)
    {
      pyval = PyList_GetItem(pylist, j);

      /* Recurse when encoding a nested message,
         reserving enough leading room for the length prefix. */

      if (field_desc->opts.typ == LWPB_MESSAGE)
      {
        u8_t* nestedbuf;
        size_t nestedlen;

        extralen += MSG_RESERVE_BYTES;
        nestedbuf = valuebuf ? valuebuf + MSG_RESERVE_BYTES : 0;
        nestedlen = pyobject_encode(pyval, encoder, field_desc->msg_desc, nestedbuf, error);
        if (*error) break;

        val.message.data = nestedbuf;
        val.message.len = nestedlen;
      }
      else {
        if (py_to_lwpb(&val, pyval, field_desc->opts.typ) < 0) {
          *error = 1; // FIXME more specific error code
          break;
        }
      }

      valuelen = lwpb_encoder2_add_field(encoder, field_desc, &val, valuebuf);
      if (valuebuf) valuebuf += valuelen;
      fieldlen += valuelen;
    }

    Py_XDECREF(pylist);
    if (*error) return -1;

    /* If the field is packed repeated, leave packed encoder mode
       and re-encode with the length prefix. */

    if (LWPB_IS_PACKED_REPEATED(field_desc)) {
      lwpb_encoder2_packed_repeated_end(encoder);
      val.message.data = fieldbuf;
      val.message.len = fieldlen;
      fieldlen = lwpb_encoder2_add_field(encoder, field_desc, &val, buf);
      if (*error) return -1;
    }

    if (buf) buf += fieldlen;
    len += fieldlen;
  }

  return buf ? len : (len + extralen);
}

static PyObject *
Encoder_encode(Encoder *self, PyObject *args)
{
  PyObject* dict;
  Descriptor* descriptor;
  unsigned int msgnum;

  if (!PyArg_ParseTuple(args, "O!O!i:encode", &PyDict_Type, &dict, &DescriptorType, &descriptor, &msgnum))
    return NULL;

  if (msgnum >= descriptor->num_msgs) {
    PyErr_SetString(PyExc_IndexError, "message type index out of range");
    return NULL;
  }

  PyObject* string = NULL;
  size_t len;
  u8_t *buf = 0;
  int error = 0;
  int pass;

  for (pass=0; pass<2; pass++)
  {
    lwpb_encoder2_start(&self->encoder, &descriptor->msg_desc[msgnum]);
    len = pyobject_encode(dict, &self->encoder, &descriptor->msg_desc[msgnum], buf, &error);

    if (error) {
      PyErr_Format(PyExc_RuntimeError, "encode error: %d", error); // FIXME exception class
      break;
    }

    if (pass == 0) {
      if (!(buf = calloc(1, len))) {
        PyErr_SetString(PyExc_MemoryError, "unable to allocate string");
        break;
      }
    } else if (pass == 1) {
      if (!(string = PyString_FromStringAndSize((char*)buf, len)))
        break;
    }
  }

  if (buf) free(buf);
  return string;
}


static PyObject*
Encoder_encode_varint(Decoder *self, PyObject *args)
{
  u64_t value;
  u8_t buf[64];
  size_t len;

  if (!PyArg_ParseTuple(args, "K:encode_varint", &value))
    return NULL;

  len = lwpb_encode_varint(buf, value);
  return PyString_FromStringAndSize((char*)buf, len);
}


static PyObject*
Encoder_encode_32bit(Decoder *self, PyObject *args)
{
  u32_t value;
  u8_t buf[64];
  size_t len;

  if (!PyArg_ParseTuple(args, "I:encode_32bit", &value))
    return NULL;

  len = lwpb_encode_32bit(buf, value);
  return PyString_FromStringAndSize((char*)buf, len);
}


static PyObject*
Encoder_encode_64bit(Decoder *self, PyObject *args)
{
  u64_t value;
  u8_t buf[64];
  size_t len;

  if (!PyArg_ParseTuple(args, "K:encode_64bit", &value))
    return NULL;

  len = lwpb_encode_64bit(buf, value);
  return PyString_FromStringAndSize((char*)buf, len);
}


static PyMethodDef Encoder_methods[] = {
  {"encode",  (PyCFunction)Encoder_encode,  METH_VARARGS,
    PyDoc_STR("encode(dict,descriptor,msgnum) -> String")},
  {"encode_varint",  (PyCFunction)Encoder_encode_varint,  METH_VARARGS,
    PyDoc_STR("encode_varint(Number) -> String")},
  {"encode_32bit",  (PyCFunction)Encoder_encode_32bit,  METH_VARARGS,
    PyDoc_STR("encode_32bit(Number) -> String")},
  {"encode_64bit",  (PyCFunction)Encoder_encode_64bit,  METH_VARARGS,
    PyDoc_STR("encode_64bit(Number) -> String")},
  {NULL,    NULL}    /* sentinel */
};


static PyTypeObject EncoderType = {
  /* The ob_type field must be initialized in the module init function
   * to be portable to Windows without using C++. */
  PyVarObject_HEAD_INIT(NULL, 0)
  "lwpb.cext.Encoder",           /*tp_name*/
  sizeof(Encoder),        /*tp_basicsize*/
  0,                            /*tp_itemsize*/
  /* methods */
  (destructor)Encoder_dealloc,  /*tp_dealloc*/
  0,                            /*tp_print*/
  0,                            /*tp_getattr*/
  0,                            /*tp_setattr*/
  0,                            /*tp_compare*/
  0,                            /*tp_repr*/
  0,                            /*tp_as_number*/
  0,                            /*tp_as_sequence*/
  0,                            /*tp_as_mapping*/
  0,                            /*tp_hash*/
  0,                            /*tp_call*/
  0,                            /*tp_str*/
  0,                            /*tp_getattro*/
  0,                            /*tp_setattro*/
  0,                            /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  0,                            /*tp_doc*/
  0,                            /*tp_traverse*/
  0,                            /*tp_clear*/
  0,                            /*tp_richcompare*/
  0,                            /*tp_weaklistoffset*/
  0,                            /*tp_iter*/
  0,                            /*tp_iternext*/
  Encoder_methods,              /*tp_methods*/
  0,                            /*tp_members*/
  0,                            /*tp_getset*/
  0,                            /*tp_base*/
  0,                            /*tp_dict*/
  0,                            /*tp_descr_get*/
  0,                            /*tp_descr_set*/
  0,                            /*tp_dictoffset*/
  (initproc)Encoder_init,       /*tp_init*/
  0,                            /*tp_alloc*/
  Encoder_new,                  /*tp_new*/
  0,                            /*tp_free*/
  0,                            /*tp_is_gc*/
};


/* --------------------------------------------------------------------- */


/* List of functions defined in the module */

static PyMethodDef lwpb_methods[] = {
  {NULL,    NULL}    /* sentinel */
};

PyDoc_STRVAR(module_doc,
"This is a template module just for instruction.");

/* Initialization function for the module (*must* be called initcext) */

PyMODINIT_FUNC
initcext(void)
{
  /* Make sure exposed types are subclassable. */

  DescriptorType.tp_base = &PyBaseObject_Type;
  DecoderType.tp_base = &PyBaseObject_Type;
  EncoderType.tp_base = &PyBaseObject_Type;

  /* Finalize the type object including setting type of the new type
   * object; doing it here is required for portability, too. */

  if (PyType_Ready(&DescriptorType) < 0)
    return;
  if (PyType_Ready(&DecoderType) < 0)
    return;
  if (PyType_Ready(&EncoderType) < 0)
    return;

  /* Create the module and add the functions */

  PyObject* mod = Py_InitModule3("lwpb.cext", lwpb_methods, module_doc);
  if (mod == NULL)
    return;

  /* Add some symbolic constants to the module */

  if (ErrorObject == NULL) {
    ErrorObject = PyErr_NewException("lwpb.cext.Error", NULL, NULL);
    if (ErrorObject == NULL)
      return;
  }

  Py_INCREF(ErrorObject);
  PyModule_AddObject(mod, "Error", ErrorObject);

  Py_INCREF(&DescriptorType);
  PyModule_AddObject(mod, "Descriptor", (PyObject*)&DescriptorType);

  Py_INCREF(&DecoderType);
  PyModule_AddObject(mod, "Decoder", (PyObject*)&DecoderType);

  Py_INCREF(&EncoderType);
  PyModule_AddObject(mod, "Encoder", (PyObject*)&EncoderType);

  PyModule_AddIntConstant(mod, "LABEL_REQUIRED", LWPB_REQUIRED);
  PyModule_AddIntConstant(mod, "LABEL_OPTIONAL", LWPB_OPTIONAL);
  PyModule_AddIntConstant(mod, "LABEL_REPEATED", LWPB_REPEATED);

  PyModule_AddIntConstant(mod, "TYPE_DOUBLE", LWPB_DOUBLE);
  PyModule_AddIntConstant(mod, "TYPE_FLOAT", LWPB_FLOAT);
  PyModule_AddIntConstant(mod, "TYPE_INT32", LWPB_INT32);
  PyModule_AddIntConstant(mod, "TYPE_INT64", LWPB_INT64);
  PyModule_AddIntConstant(mod, "TYPE_UINT32", LWPB_UINT32);
  PyModule_AddIntConstant(mod, "TYPE_UINT64", LWPB_UINT64);
  PyModule_AddIntConstant(mod, "TYPE_SINT32", LWPB_SINT32);
  PyModule_AddIntConstant(mod, "TYPE_SINT64", LWPB_SINT64);
  PyModule_AddIntConstant(mod, "TYPE_FIXED32", LWPB_FIXED32);
  PyModule_AddIntConstant(mod, "TYPE_FIXED64", LWPB_FIXED64);
  PyModule_AddIntConstant(mod, "TYPE_SFIXED32", LWPB_SFIXED32);
  PyModule_AddIntConstant(mod, "TYPE_SFIXED64", LWPB_SFIXED64);
  PyModule_AddIntConstant(mod, "TYPE_BOOL", LWPB_BOOL);
  PyModule_AddIntConstant(mod, "TYPE_ENUM", LWPB_ENUM);
  PyModule_AddIntConstant(mod, "TYPE_STRING", LWPB_STRING);
  PyModule_AddIntConstant(mod, "TYPE_BYTES", LWPB_BYTES);
  PyModule_AddIntConstant(mod, "TYPE_MESSAGE", LWPB_MESSAGE);

  PyModule_AddIntConstant(mod, "WIRETYPE_VARINT", 0);
  PyModule_AddIntConstant(mod, "WIRETYPE_FIXED64", 1);
  PyModule_AddIntConstant(mod, "WIRETYPE_LENGTH_DELIMITED", 2);
  PyModule_AddIntConstant(mod, "WIRETYPE_START_GROUP", 3);
  PyModule_AddIntConstant(mod, "WIRETYPE_END_GROUP", 4);
  PyModule_AddIntConstant(mod, "WIRETYPE_FIXED32", 5);
}

