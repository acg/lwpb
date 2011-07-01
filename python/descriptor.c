#include "descriptor.h"
#include "Python.h"
#include <lwpb/lwpb.h>
#include <stdio.h>  // for Descriptor_debug_print()
#include "primitives.h"
#include "pythoncompat.h"

/* Descriptor methods */

static PyObject *
Descriptor_new(PyTypeObject *type, PyObject *arg, PyObject *kwds)
{
  Descriptor *self;
  
  if (!(self = (Descriptor*)type->tp_alloc(type, 0)))
    return NULL;

  if (!(self->strings = PyList_New(0)))
    return NULL;

  if (!(self->message_types = PyDict_New()))
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

  /* Clear the string table and the messages index. */

  if (self->strings)
    PyList_SetSlice(self->strings, 0, PyList_Size(self->strings), NULL);

  if (self->message_types)
    PyDict_Clear(self->message_types);
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

  /* Top level message stuff. */

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

  /* Pass #0: fill in message descriptors.
     Pass #1: fill in field descriptors, resolving nested messages. */

  /* FIXME require that messages have a name */
  /* FIXME require that fields have a name and field number */

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
        {
          m->name = Descriptor_store_string(self, prop);

          /* Map the package-qualified message name to the index number,
             which Python clients can look up and pass to Decoder.decode()
             or Encoder.encode(). */

          PyObject* qualified;

          if ((qualified = PyString_FromFormat("%s.%s", package, m->name)))
            PyDict_SetItem(self->message_types, qualified, PyInt_FromLong(i));
        }
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

          /* TODO just lookup qualified type name in self->message_types */
          /* TODO support name resolution for nested types */

          const char* typename = "\"\"";

          if (f->opts.typ == LWPB_MESSAGE)
          {
            if ((prop = PyDict_GetItemString(field, "type_name")) && PyString_Check(prop))
            {
              typename = PyString_AsString(prop);

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
            }

            if (!f->msg_desc) {
              PyErr_Format(
                PyExc_RuntimeError,
                "could not resolve type_name for message field %s.%s.%s: %s",
                package, m->name, f->name, typename
              );
              error = -1;
              goto init_cleanup;
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

      if (f->opts.typ == LWPB_MESSAGE)
        printf( "  message = %s\n", m->name );
      printf( "\n" );
    }

    printf( "\n\n" );
  }
  
  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *
Descriptor_message_types(Descriptor *self, PyObject *arg)
{
  return PyDictProxy_New(self->message_types);
}


static PyMethodDef Descriptor_methods[] = {
  {"message_types",  (PyCFunction)Descriptor_message_types,  METH_VARARGS,
    PyDoc_STR("message_types() -> dict")},
  {"debug_print",  (PyCFunction)Descriptor_debug_print,  METH_VARARGS,
    PyDoc_STR("debug_print() -> None")},
  {NULL,    NULL}    /* sentinel */
};


PyTypeObject DescriptorType = {
  /* The ob_type field must be initialized in the module init function
   * to be portable to Windows without using C++. */
  PyObject_HEAD_INIT(NULL)
  0,                            /*ob_size*/
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
  0,                            /*tp_getattro */
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

