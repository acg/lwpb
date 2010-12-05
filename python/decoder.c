#include "decoder.h"
#include "Python.h"
#include <lwpb/lwpb.h>
#include "descriptor.h"
#include "primitives.h"


typedef struct {
  PyObject* stack;
} DecoderContext;


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

  if (!decoder->packed) {
    Py_ssize_t stacklen = PyList_Size(context->stack);
    PyList_SetSlice(context->stack, stacklen-1, stacklen, NULL);
  }
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


PyTypeObject DecoderType = {
  /* The ob_type field must be initialized in the module init function
   * to be portable to Windows without using C++. */
  PyObject_HEAD_INIT(NULL)
  0,                            /*ob_size*/
  "lwpb.cext.Decoder",           /*tp_name*/
  sizeof(Decoder),              /*tp_basicsize*/
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


