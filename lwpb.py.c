
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

// 'PhoneNumber.PhoneType' enumeration values
#define TEST_PHONENUMBER_MOBILE 0
#define TEST_PHONENUMBER_HOME 1
#define TEST_PHONENUMBER_WORK 2

extern const struct lwpb_msg_desc lwpb_messages_test[];

// Message descriptor pointers
#define test_PhoneNumber (&lwpb_messages_test[0])
#define test_Person (&lwpb_messages_test[1])

extern const struct lwpb_field_desc lwpb_fields_test_phonenumber[];

// 'PhoneNumber' field descriptor pointers
#define test_PhoneNumber_number (&lwpb_fields_test_phonenumber[0])
#define test_PhoneNumber_type (&lwpb_fields_test_phonenumber[1])

extern const struct lwpb_field_desc lwpb_fields_test_person[];

// 'Person' field descriptor pointers
#define test_Person_name (&lwpb_fields_test_person[0])
#define test_Person_id (&lwpb_fields_test_person[1])
#define test_Person_email (&lwpb_fields_test_person[2])
#define test_Person_phone (&lwpb_fields_test_person[3])

// 'PhoneNumber' field descriptors
const struct lwpb_field_desc lwpb_fields_test_phonenumber[] = {
    {
        .number = 1,
        .opts.label = LWPB_REQUIRED,
        .opts.typ = LWPB_STRING,
        .opts.flags = 0,
        .msg_desc = 0,
#if LWPB_FIELD_NAMES
        .name = "number",
#endif
#if LWPB_FIELD_DEFAULTS
        .def.string.str = "",
#endif
    },
    {
        .number = 2,
        .opts.label = LWPB_OPTIONAL,
        .opts.typ = LWPB_ENUM,
        .opts.flags = 0 | LWPB_HAS_DEFAULT,
        .msg_desc = 0,
#if LWPB_FIELD_NAMES
        .name = "type",
#endif
#if LWPB_FIELD_DEFAULTS
        .def.int32 = 1,
#endif
    },
};

// 'Person' field descriptors
const struct lwpb_field_desc lwpb_fields_test_person[] = {
    {
        .number = 1,
        .opts.label = LWPB_REQUIRED,
        .opts.typ = LWPB_STRING,
        .opts.flags = 0,
        .msg_desc = 0,
#if LWPB_FIELD_NAMES
        .name = "name",
#endif
#if LWPB_FIELD_DEFAULTS
        .def.string.str = "",
#endif
    },
    {
        .number = 2,
        .opts.label = LWPB_REQUIRED,
        .opts.typ = LWPB_INT32,
        .opts.flags = 0,
        .msg_desc = 0,
#if LWPB_FIELD_NAMES
        .name = "id",
#endif
#if LWPB_FIELD_DEFAULTS
        .def.int32 = 0,
#endif
    },
    {
        .number = 3,
        .opts.label = LWPB_OPTIONAL,
        .opts.typ = LWPB_STRING,
        .opts.flags = 0 | LWPB_IS_DEPRECATED,
        .msg_desc = 0,
#if LWPB_FIELD_NAMES
        .name = "email",
#endif
#if LWPB_FIELD_DEFAULTS
        .def.string.str = "",
#endif
    },
    {
        .number = 4,
        .opts.label = LWPB_REPEATED,
        .opts.typ = LWPB_MESSAGE,
        .opts.flags = 0,
        .msg_desc = test_PhoneNumber,
#if LWPB_FIELD_NAMES
        .name = "phone",
#endif
#if LWPB_FIELD_DEFAULTS
        .def.null = 0,
#endif
    },
};

// Message descriptors
const struct lwpb_msg_desc lwpb_messages_test[] = {
    {
        .num_fields = 2,
        .fields = lwpb_fields_test_phonenumber,
#if LWPB_MESSAGE_NAMES
        .name = "PhoneNumber",
#endif
    },
    {
        .num_fields = 4,
        .fields = lwpb_fields_test_person,
#if LWPB_MESSAGE_NAMES
        .name = "Person",
#endif
    },
};


/* ----------------------------------------- */

static PyObject* lwpb_to_py(union lwpb_value *p, unsigned int type)
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

/* ----------------------------------------- */


/* Decoder object */

static PyObject *ErrorObject;

typedef struct {
  PyObject_HEAD
  struct lwpb_decoder decoder;
  PyObject *x_attr;  /* Attributes dictionary */
} Decoder;

static PyTypeObject DecoderType;

#define Decoder_Check(v)  (Py_TYPE(v) == &DecoderType)


/* Decoder methods */

static PyObject *
Decoder_new(PyTypeObject *type, PyObject *arg, PyObject *kwds)
{
  Decoder *self;
  self = (Decoder *)type->tp_alloc(type, 0);
  if (self == NULL)
    return NULL;
  self->x_attr = NULL;
  return (PyObject *)self;
}

static void
Decoder_dealloc(Decoder *self)
{
  Py_XDECREF(self->x_attr);
  self->ob_type->tp_free((PyObject*)self);
}


/*
  TODO
  For all handlers, *arg will be a lwpb.Decoder PyObject.
  Look up msg_desc->name in the Descriptor pydict.
  Look up field_desc->name under the msg pydict.
  Convert the lwpb_value to a python value. See upb py code.
    - Convert NULL to PyNone
  Check object methods. Is there a python handler?
    - If yes, call it with arguments.
    - Maybe do something with return value, particularly if it is NULL
      (exception)

 */

static void
msg_start_handler(
  struct lwpb_decoder *decoder,
  const struct lwpb_msg_desc *msg_desc,
  void *arg)
{
   // TODO call python method
}

static void
msg_end_handler(
  struct lwpb_decoder *decoder,
  const struct lwpb_msg_desc *msg_desc,
  void *arg)
{
   // TODO call python method
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

  PyObject* method = PyString_FromString("field_handler");
  PyObject* object = (PyObject*)arg;
  PyObject* retval = PyObject_CallMethodObjArgs(object, method, val, NULL);

  Py_DECREF(retval);
  Py_DECREF(method);
  Py_DECREF(val);
}

static PyObject *
Decoder_decode(Decoder *self, PyObject *args)
{
  char *buf;
  int len;
  lwpb_err_t ret;

  if (!PyArg_ParseTuple(args, "s#:decode", &buf, &len))
    return NULL;

  //lwpb_decoder_use_debug_handlers(&self->decoder);
  lwpb_decoder_arg(&self->decoder, (PyObject*)self);
  lwpb_decoder_msg_handler(&self->decoder, msg_start_handler, msg_end_handler);
  lwpb_decoder_field_handler(&self->decoder, field_handler);
  ret = lwpb_decoder_decode(&self->decoder, test_Person, buf, len, NULL);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
Decoder_field_handler(Decoder *self, PyObject *args)
{
  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef Decoder_methods[] = {
  {"decode",  (PyCFunction)Decoder_decode,  METH_VARARGS,
    PyDoc_STR("decode() -> None")},
  {"field_handler",  (PyCFunction)Decoder_field_handler,  METH_VARARGS,
    PyDoc_STR("field_handler() -> None")},
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
  0,                            /*tp_init*/
  0,                            /*tp_alloc*/
  Decoder_new,                  /*tp_new*/
  0,                            /*tp_free*/
  0,                            /*tp_is_gc*/
};

/* --------------------------------------------------------------------- */

/*

 TODO lwpb.Descriptor(dict)

 Where dict has the form

  {
    "name": filename,
    "package": package,
    "dependency": [ "filename1", "filename2", ... ],
    "message_type": [
      {
        "name": message_name,
        "field":
        [
          {
             "name": field_name,
             "number": field_number,
             "label": label,     # integer
             "type": type,       # integer
             "type_name": type_name,
             "default_value": default_value,  # any python scalar type
             "options": {
               "packed": packed,  # boolean
               "deprecated": deprecated,  # boolean
             }
          },
        ]
      }
    ]
  }

  When a new one is created, dynamically create an array of lwpb_msg_desc
  with associated lwpb_field_desc entries, and populate them from the dict.
  Determine length of message_type array and field arrays for malloc().
  In pass #1 over pydict, fill in everything except field_desc->msg_desc.
  In pass #2 over lwpb_msg_desc fields, find type_name in pydict and
  set up pointer to the right lwpb_msg_desc slot in case of TYPE_MESSAGE.

 */


/* --------------------------------------------------------------------- */

/* Function of two integers returning integer */

PyDoc_STRVAR(lwpb_foo_doc,
"foo(i,j)\n\
\n\
Return the sum of i and j.");

static PyObject *
lwpb_foo(PyObject *self, PyObject *args)
{
  long i, j;
  long res;
  if (!PyArg_ParseTuple(args, "ll:foo", &i, &j))
    return NULL;
  res = i+j; /* XXX Do something here */
  return PyInt_FromLong(res);
}


/* List of functions defined in the module */

static PyMethodDef lwpb_methods[] = {
  {"foo",    lwpb_foo,    METH_VARARGS,
     lwpb_foo_doc},
  {NULL,    NULL}    /* sentinel */
};

PyDoc_STRVAR(module_doc,
"This is a template module just for instruction.");

/* Initialization function for the module (*must* be called initlwpb) */

PyMODINIT_FUNC
initlwpb(void)
{
  /* Make sure lwpb.Decoder is subclassable. */
  DecoderType.tp_base = &PyBaseObject_Type;

  /* Finalize the type object including setting type of the new type
   * object; doing it here is required for portability, too. */
  if (PyType_Ready(&DecoderType) < 0)
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
}

