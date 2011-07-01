#include "encoder.h"
#include "Python.h"
#include <lwpb/lwpb.h>
#include <lwpb/core/encoder2.h>
#include "descriptor.h"
#include "primitives.h"
#include "pythoncompat.h"


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
  u8_t* buf)
{
  size_t len = 0;
  size_t extralen = 0;
  unsigned int i;
  unsigned int j;
  union lwpb_value val;
  PyObject* pyval = NULL;
  PyObject* pylist = NULL;
  int error = 0;

  /* Python object must be a dict to be encoded as a message. */

  if (!PyDict_Check(obj)) {
    PyErr_SetString(PyExc_TypeError, "encode expects a dict");
    return -1;
  }

  /* Encode each field defined in the message descriptor. */

  for (i=0; i<msg_desc->num_fields; i++)
  {
    u8_t* fieldbuf = buf;
    size_t fieldlen = 0;
    const struct lwpb_field_desc *field_desc = &msg_desc->fields[i];

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

    /* If the field is packed repeated, enter packed encoder mode
       and reserve enough leading room for the eventual length prefix.  */

    if (LWPB_IS_PACKED_REPEATED(field_desc)) {
      lwpb_encoder2_packed_repeated_start(encoder, field_desc);
      if (fieldbuf) fieldbuf += MSG_RESERVE_BYTES;
      extralen += MSG_RESERVE_BYTES;
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
        nestedlen = pyobject_encode(pyval, encoder, field_desc->msg_desc, nestedbuf);

        if (nestedlen < 0) {
          error = 1;
          break;
        }

        val.message.data = nestedbuf;
        val.message.len = nestedlen;
      }
      else {
        if (py_to_lwpb(&val, pyval, field_desc->opts.typ) < 0) {
          error = 1;
          break;
        }
      }

      valuelen = lwpb_encoder2_add_field(encoder, field_desc, &val, valuebuf);
      if (valuebuf) valuebuf += valuelen;
      fieldlen += valuelen;
    }

    Py_XDECREF(pylist);
    if (error) return -1;

    /* If the field is packed repeated, leave packed encoder mode
       and re-encode with the length prefix. */

    if (LWPB_IS_PACKED_REPEATED(field_desc)) {
      lwpb_encoder2_packed_repeated_end(encoder);
      val.message.data = fieldbuf;
      val.message.len = fieldlen;
      fieldlen = lwpb_encoder2_add_field(encoder, field_desc, &val, buf);
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
  int pass;

  /*
     In pass #0, invoke the object encoder on a NULL buffer just to
     calculate the required size. Allocate a buffer of that size.

     In pass #1, actually encode the object into the allocated buffer.
     Copy this into a new Python string.
  */

  for (pass=0; pass<2; pass++)
  {
    lwpb_encoder2_start(&self->encoder, &descriptor->msg_desc[msgnum]);
    len = pyobject_encode(dict, &self->encoder, &descriptor->msg_desc[msgnum], buf);

    if (len < 0)
      break;

    if (pass == 0) {
      if (!(buf = calloc(1, len))) {
        PyErr_SetString(PyExc_MemoryError, "unable to allocate string");
        break;
      }
    } else if (pass == 1) {
      string = PyString_FromStringAndSize((char*)buf, len);
    }
  }

  if (buf) free(buf);
  return string;
}


static PyObject*
Encoder_encode_varint(Encoder *self, PyObject *args)
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
Encoder_encode_32bit(Encoder *self, PyObject *args)
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
Encoder_encode_64bit(Encoder *self, PyObject *args)
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


PyTypeObject EncoderType = {
  /* The ob_type field must be initialized in the module init function
   * to be portable to Windows without using C++. */
  PyObject_HEAD_INIT(NULL)
  0,                            /*ob_size*/
  "lwpb.cext.Encoder",          /*tp_name*/
  sizeof(Encoder),              /*tp_basicsize*/
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


