
/* Use this file as a template to start implementing a module that
   also declares object types. All occurrences of 'decoder' should be changed
   to something reasonable for your objects. After that, all other
   occurrences of 'xx' should be changed to something reasonable for your
   module. If your module is named foo your sourcefile should be named
   foomodule.c.

   You will probably want to delete all references to 'x_attr' and add
   your own types of attributes instead.  Maybe you want to name your
   local variables other than 'self'.  If your object type is needed in
   other files, you'll have to create a file "foobarobject.h"; see
   intobject.h for an example. */

/* decoder objects */

#include "Python.h"
#include <lwpb/lwpb.h>


/* ----------------------------------------- */

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


static PyObject *ErrorObject;

/* ----------------------------------------- */

/* Descriptor class */

typedef struct {
  PyObject_HEAD
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
  self = (Descriptor *)type->tp_alloc(type, 0);
  if (self == NULL) return NULL;

  self->num_msgs = 0;
  self->msg_desc = NULL;

  return (PyObject *)self;
}

static int
Descriptor_init(Descriptor *self, PyObject *arg, PyObject *kwds)
{
  PyObject* dict;
  PyObject* msgtypes;

  if (!PyArg_ParseTuple(arg, "O!:init", &PyDict_Type, &dict))
    return -1;

  msgtypes = PyDict_GetItemString(dict, "message_type");
  if (msgtypes == NULL) return 0;

  if (!PyList_Check(msgtypes)) {
    PyErr_SetString(PyExc_RuntimeError, "message_type: list expected");
    return -1;
  }

  PyObject* msgtype;
  PyObject* fields;
  PyObject* field;
  PyObject* options;
  PyObject* prop;
  Py_ssize_t msgtypes_len;
  Py_ssize_t fields_len;
  Py_ssize_t i;
  Py_ssize_t j;
  Py_ssize_t k;

  msgtypes_len = PyList_Size(msgtypes);
  self->msg_desc = (struct lwpb_msg_desc*)calloc(msgtypes_len, sizeof(struct lwpb_msg_desc)); // FIXME check for out of memory error
  self->num_msgs = msgtypes_len;

  int pass;

  // FIXME for all string types, store a private PyString in the object, refer to that

  for (pass=0; pass<2; pass++)
  {
    for (i=0; i<msgtypes_len; i++)
    {
      struct lwpb_msg_desc* m = &self->msg_desc[i];

      msgtype = PyList_GetItem(msgtypes, i);
      if (msgtype == NULL) continue;
      if (!PyDict_Check(msgtype)) continue;

      fields = PyDict_GetItemString(msgtype, "field");
      if (fields == NULL) continue;
      if (!PyList_Check(fields)) continue;
      fields_len = PyList_Size(fields);

      if (pass == 0)
      {
        m->num_fields = fields_len;
        m->fields = (struct lwpb_field_desc*)calloc(fields_len, sizeof(struct lwpb_field_desc)); // FIXME check for out of memory error

        prop = PyDict_GetItemString(msgtype, "name");
        if (prop != NULL && PyString_Check(prop)) {
          m->name = PyString_AsString(prop);
        }
      }
      else if (pass == 1)
      {
        for (j=0; j<m->num_fields; j++)
        {
          struct lwpb_field_desc* f = (struct lwpb_field_desc*)&m->fields[j];

          field = PyList_GetItem(fields, j);
          if (field == NULL) continue;
          if (!PyDict_Check(field)) continue;

          prop = PyDict_GetItemString(field, "number");
          if (prop != NULL && PyInt_Check(prop)) {
            f->number = PyInt_AsLong(prop);
          }

          prop = PyDict_GetItemString(field, "label");
          if (prop != NULL && PyInt_Check(prop)) {
            f->opts.label = PyInt_AsLong(prop);
          }

          prop = PyDict_GetItemString(field, "type");
          if (prop != NULL && PyInt_Check(prop)) {
            f->opts.typ = PyInt_AsLong(prop);
          }

          options = PyDict_GetItemString(field, "options");

          if (options != NULL && PyDict_Check(options))
          {
            prop = PyDict_GetItemString(options, "packed");
            if (prop == Py_True) {
              f->opts.flags |= LWPB_IS_PACKED;
            }

            prop = PyDict_GetItemString(options, "deprecated");
            if (prop == Py_True) {
              f->opts.flags |= LWPB_IS_DEPRECATED;
            }
          }

          prop = PyDict_GetItemString(field, "name");
          if (prop != NULL && PyString_Check(prop)) {
            f->name = PyString_AsString(prop);
          }

          prop = PyDict_GetItemString(field, "default_value");
          if (prop != NULL) {
            if (!py_to_lwpb(&f->def, prop, f->opts.typ))
              f->opts.flags |= LWPB_HAS_DEFAULT;
          }

          prop = PyDict_GetItemString(field, "type_name");
          if (prop != NULL && PyString_Check(prop)) {
            if (f->opts.typ == LWPB_MESSAGE) {

              for (k=0; k<msgtypes_len; k++) {

                PyObject* msgtype2 = PyList_GetItem(msgtypes, k);
                if (msgtype2 == NULL) continue;
                if (!PyDict_Check(msgtype2)) continue;

                PyObject* prop2 = PyDict_GetItemString(msgtype2, "name");
                if (prop2 == NULL) continue;
                if (!PyString_Check(prop2)) continue;

                int cmp;
                if (PyObject_Cmp(prop, prop2, &cmp) >= 0 && cmp == 0) {
                  f->msg_desc = &self->msg_desc[k];
                  break;
                }
              }
            }
          }

        } // for each field
      } // if second pass, set up fields
    } // for each msgtype 
  } // for each pass

  return 0;
}

static void
Descriptor_dealloc(Descriptor *self)
{
  unsigned int i;

  for (i=0; i<self->num_msgs; i++)
  {
    struct lwpb_msg_desc* m = &self->msg_desc[i];
    if (m->fields)
      free((void*)m->fields);
  }
  
  if (self->msg_desc)
    free((void*)self->msg_desc);

  self->ob_type->tp_free((PyObject*)self);
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
  "lwpb.Descriptor",            /*tp_name*/
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
  Descriptor* descriptor;
} Decoder;

static PyTypeObject DecoderType;

#define Decoder_Check(v)  (Py_TYPE(v) == &DecoderType)


/* Decoder methods */

static PyObject *
Decoder_new(PyTypeObject *type, PyObject *arg, PyObject *kwds)
{
  Decoder *self;
  self = (Decoder *)type->tp_alloc(type, 0);
  if (self == NULL) return NULL;
  self->descriptor = NULL;
  return (PyObject *)self;
}

static int
Decoder_init(Decoder *self, PyObject *arg, PyObject *kwds)
{
  if (!PyArg_ParseTuple(arg, "O!:init", &DescriptorType, &self->descriptor))
    return -1;
  Py_INCREF(self->descriptor);
  return 0;
}

static void
Decoder_dealloc(Decoder *self)
{
  Py_XDECREF(self->descriptor);
  self->ob_type->tp_free((PyObject*)self);
}


/* Decoder handlers */

static void
msg_start_handler(
  struct lwpb_decoder *decoder,
  const struct lwpb_msg_desc *msg_desc,
  void *arg)
{
  if (!arg) return;

  PyObject* message_name;

  if (msg_desc && msg_desc->name != NULL)
    message_name = PyString_FromString(msg_desc->name);
  else {
    message_name = Py_None;
    Py_INCREF(Py_None);
  }

  PyObject* method = PyString_FromString("msg_start_handler");
  PyObject* object = (PyObject*)arg;
  PyObject* retval = PyObject_CallMethodObjArgs(object, method, message_name, NULL);

  /*
  TODO handle NULL return value (ie exception).
    need to modify lwpb to abort decoding.
  */

  Py_XDECREF(retval);
  Py_DECREF(method);
}

static void
msg_end_handler(
  struct lwpb_decoder *decoder,
  const struct lwpb_msg_desc *msg_desc,
  void *arg)
{
  if (!arg) return;

  PyObject* message_name;

  if (msg_desc && msg_desc->name != NULL)
    message_name = PyString_FromString(msg_desc->name);
  else {
    message_name = Py_None;
    Py_INCREF(Py_None);
  }

  PyObject* method = PyString_FromString("msg_end_handler");
  PyObject* object = (PyObject*)arg;
  PyObject* retval = PyObject_CallMethodObjArgs(object, method, message_name, NULL);

  /*
  TODO handle NULL return value (ie exception).
    need to modify lwpb to abort decoding.
  */

  Py_XDECREF(retval);
  Py_DECREF(method);
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

  PyObject* val;

  if (value)
    val = lwpb_to_py(value, field_desc->opts.typ);
  else {
    val = Py_None;
    Py_INCREF(Py_None);
  }

  if (val == NULL) return;

  PyObject* message_name;
  PyObject* field_name;

  if (msg_desc && msg_desc->name != NULL)
    message_name = PyString_FromString(msg_desc->name);
  else {
    message_name = Py_None;
    Py_INCREF(Py_None);
  }

  if (field_desc && field_desc->name != NULL)
    field_name = PyString_FromString(field_desc->name);
  else {
    field_name = Py_None;
    Py_INCREF(Py_None);
  }

  PyObject* method = PyString_FromString("field_handler");
  PyObject* object = (PyObject*)arg;
  PyObject* retval = PyObject_CallMethodObjArgs(object, method, message_name, field_name, val, NULL);

  /*
  TODO handle NULL return value (ie exception).
    need to modify lwpb to abort decoding in this case.
  */

  Py_XDECREF(retval);
  Py_DECREF(method);
  Py_DECREF(field_name);
  Py_DECREF(message_name);
  Py_DECREF(val);
}

static PyObject *
Decoder_decode(Decoder *self, PyObject *args)
{
  char *buf;
  int len;
  unsigned int msgnum;
  lwpb_err_t ret;

  if (!PyArg_ParseTuple(args, "s#i:decode", &buf, &len, &msgnum))
    return NULL;

  if (msgnum >= self->descriptor->num_msgs) {
    PyErr_SetString(PyExc_IndexError, "message type index out of range");
    return NULL;
  }

  //lwpb_decoder_use_debug_handlers(&self->decoder);
  lwpb_decoder_arg(&self->decoder, (PyObject*)self);
  lwpb_decoder_msg_handler(&self->decoder, msg_start_handler, msg_end_handler);
  lwpb_decoder_field_handler(&self->decoder, field_handler);
  ret = lwpb_decoder_decode(&self->decoder, &self->descriptor->msg_desc[msgnum], buf, len, NULL);

  return PyInt_FromLong(ret);
}

static PyObject *
Decoder_field_handler(Decoder *self, PyObject *args)
{
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
Decoder_msg_start_handler(Decoder *self, PyObject *args)
{
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
Decoder_msg_end_handler(Decoder *self, PyObject *args)
{
  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef Decoder_methods[] = {
  {"decode",  (PyCFunction)Decoder_decode,  METH_VARARGS,
    PyDoc_STR("decode(data,msgnum) -> Integer")},
  {"field_handler",  (PyCFunction)Decoder_field_handler,  METH_VARARGS,
    PyDoc_STR("field_handler(msgname,fieldname,value) -> None")},
  {"msg_start_handler",  (PyCFunction)Decoder_msg_start_handler,  METH_VARARGS,
    PyDoc_STR("msg_start_handler(msgname) -> None")},
  {"msg_end_handler",  (PyCFunction)Decoder_msg_end_handler,  METH_VARARGS,
    PyDoc_STR("msg_end_handler(msgname) -> None")},
  {NULL,    NULL}    /* sentinel */
};

static PyTypeObject DecoderType = {
  /* The ob_type field must be initialized in the module init function
   * to be portable to Windows without using C++. */
  PyVarObject_HEAD_INIT(NULL, 0)
  "lwpb.Decoder",               /*tp_name*/
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

/* List of functions defined in the module */

static PyMethodDef lwpb_methods[] = {
  {NULL,    NULL}    /* sentinel */
};

PyDoc_STRVAR(module_doc,
"This is a template module just for instruction.");

/* Initialization function for the module (*must* be called initlwpb) */

PyMODINIT_FUNC
initlwpb(void)
{
  /* Make sure exposed types are subclassable. */

  DecoderType.tp_base = &PyBaseObject_Type;
  DescriptorType.tp_base = &PyBaseObject_Type;

  /* Finalize the type object including setting type of the new type
   * object; doing it here is required for portability, too. */

  if (PyType_Ready(&DecoderType) < 0)
    return;
  if (PyType_Ready(&DescriptorType) < 0)
    return;

  /* Create the module and add the functions */

  PyObject* mod = Py_InitModule3("lwpb", lwpb_methods, module_doc);
  if (mod == NULL)
    return;

  /* Add some symbolic constants to the module */

  if (ErrorObject == NULL) {
    ErrorObject = PyErr_NewException("lwpb.Error", NULL, NULL);
    if (ErrorObject == NULL)
      return;
  }

  Py_INCREF(ErrorObject);
  PyModule_AddObject(mod, "Error", ErrorObject);

  Py_INCREF(&DecoderType);
  PyModule_AddObject(mod, "Decoder", (PyObject*)&DecoderType);

  Py_INCREF(&DescriptorType);
  PyModule_AddObject(mod, "Descriptor", (PyObject*)&DescriptorType);

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

