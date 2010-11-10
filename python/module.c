#include "Python.h"
#include "descriptor.h"
#include "decoder.h"
#include "encoder.h"

static PyObject *ErrorObject;  // FIXME move into own header?


/* List of functions defined in the module */

static PyMethodDef lwpb_methods[] = {
  {NULL,    NULL}    /* sentinel */
};

PyDoc_STRVAR(module_doc, "Fast encoding and decoding of protocol buffers.");

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

