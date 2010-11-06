
import sys
import pprint
import lwpb
#import json

protodef = {
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
           "label": lwpb.LABEL_REPEATED,
           "type": lwpb.TYPE_MESSAGE,
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
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_STRING,
        },
        {
           "name": "package",
           "number": 2,
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_STRING,
        },
        {
           "name": "dependency",
           "number": 3,
           "label": lwpb.LABEL_REPEATED,
           "type": lwpb.TYPE_STRING,
        },
        {
           "name": "message_type",
           "number": 4,
           "label": lwpb.LABEL_REPEATED,
           "type": lwpb.TYPE_MESSAGE,
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
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_STRING,
        },
        {
           "name": "field",
           "number": 2,
           "label": lwpb.LABEL_REPEATED,
           "type": lwpb.TYPE_MESSAGE,
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
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_STRING,
        },
        {
           "name": "number",
           "number": 3,
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_INT32,
        },
        {
           "name": "label",
           "number": 4,
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_ENUM,
        },
        {
           "name": "type",
           "number": 5,
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_ENUM,
        },
        {
           "name": "type_name",
           "number": 6,
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_STRING,
        },
        {
           "name": "default_value",
           "number": 7,
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_STRING,
        },
        {
           "name": "options",
           "number": 8,
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_MESSAGE,
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
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_BOOL,
        },
        {
           "name": "deprecated",
           "number": 3,
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_BOOL,
           "default_value": False,
        },
      ]
    },
  ]
}

protodesc = lwpb.Descriptor(protodef)
# protodesc.debug_print()
d = lwpb.Decoder()
pb1 = file(sys.argv[1]).read()
schema = d.decode(pb1, protodesc, 0)
schemadesc = lwpb.Descriptor(schema['file'][0])
#schemadesc.debug_print()

#pp = pprint.PrettyPrinter(indent=2)
#pp.pprint(record)

f = file(sys.argv[2])
blocksize = 4096
buf = ""
eof = False

while not eof or len(buf) > 0:

  if not eof:
    block = f.read(blocksize)
    if len(block) < blocksize: eof = True
    buf += block

  pos = 0

  while pos < len(buf):
    (recordlen, bytes) = d.decode_varint(buf[pos:])
    if not bytes: break
    if pos + bytes + recordlen > len(buf): break
    pos += bytes
    record = d.decode(buf[pos:pos+recordlen],schemadesc,1)
    pos += recordlen
    #print record

  if pos > 0:
    buf = buf[pos:]
  elif eof and len(buf):
    raise EOFError("partial record at EOF %d %s" % (pos, buf))

