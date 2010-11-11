from lwpb.cext import *

# Defines the "meta schema" of a binary file created with:
#   $ protoc foo.proto -o foo.pb2 

PROTOFILE_DEFINITION = {
  "name": "descriptor.proto",
  "package": "google.protobuf",
  "dependency": [ ],
  "message_type": [
    {
      "name": "FileDescriptorSet",
      "field":
      [
        {
           "name": "file",
           "number": 1,
           "label": LABEL_REPEATED,
           "type": TYPE_MESSAGE,
           "type_name": ".google.protobuf.FileDescriptorProto",
        }
      ]
    },
    {
      "name": "FileDescriptorProto",
      "field":
      [
        {
           "name": "name",
           "number": 1,
           "label": LABEL_OPTIONAL,
           "type": TYPE_STRING,
        },
        {
           "name": "package",
           "number": 2,
           "label": LABEL_OPTIONAL,
           "type": TYPE_STRING,
        },
        {
           "name": "dependency",
           "number": 3,
           "label": LABEL_REPEATED,
           "type": TYPE_STRING,
        },
        {
           "name": "message_type",
           "number": 4,
           "label": LABEL_REPEATED,
           "type": TYPE_MESSAGE,
           "type_name": ".google.protobuf.DescriptorProto",
        }
      ]
    },
    {
      "name": "DescriptorProto",
      "field":
      [
        {
           "name": "name",
           "number": 1,
           "label": LABEL_OPTIONAL,
           "type": TYPE_STRING,
        },
        {
           "name": "field",
           "number": 2,
           "label": LABEL_REPEATED,
           "type": TYPE_MESSAGE,
           "type_name": ".google.protobuf.FieldDescriptorProto",
        },
      ]
    },
    {
      "name": "FieldDescriptorProto",
      "field":
      [
        {
           "name": "name",
           "number": 1,
           "label": LABEL_OPTIONAL,
           "type": TYPE_STRING,
        },
        {
           "name": "number",
           "number": 3,
           "label": LABEL_OPTIONAL,
           "type": TYPE_INT32,
        },
        {
           "name": "label",
           "number": 4,
           "label": LABEL_OPTIONAL,
           "type": TYPE_ENUM,
        },
        {
           "name": "type",
           "number": 5,
           "label": LABEL_OPTIONAL,
           "type": TYPE_ENUM,
        },
        {
           "name": "type_name",
           "number": 6,
           "label": LABEL_OPTIONAL,
           "type": TYPE_STRING,
        },
        {
           "name": "default_value",
           "number": 7,
           "label": LABEL_OPTIONAL,
           "type": TYPE_STRING,
        },
        {
           "name": "options",
           "number": 8,
           "label": LABEL_OPTIONAL,
           "type": TYPE_MESSAGE,
        },
      ]
    },
    {
      "name": "FieldOptions",
      "field":
      [
        {
           "name": "packed",
           "number": 2,
           "label": LABEL_OPTIONAL,
           "type": TYPE_BOOL,
        },
        {
           "name": "deprecated",
           "number": 3,
           "label": LABEL_OPTIONAL,
           "type": TYPE_BOOL,
           "default_value": False,
        },
      ]
    },
  ]
}

