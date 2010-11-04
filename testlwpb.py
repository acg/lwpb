
import lwpb

descdef = {
  "name": "test.proto",
  "package": "lwpbtest",
  "dependency": [ ],
  "message_type": [
    {
      "name": "PhoneNumber",
      "field":
      [
        {
           "name": "number",
           "number": 1,
           "label": lwpb.LABEL_REQUIRED,
           "type": lwpb.TYPE_STRING,
           "default_value": ""
        },
        {
           "name": "type",
           "number": 2,
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_ENUM,
           "default_value": 1
        }
      ]
    },
    {
      "name": "Person",
      "field":
      [
        {
           "name": "name",
           "number": 1,
           "label": lwpb.LABEL_REQUIRED,
           "type": lwpb.TYPE_STRING,
           "default_value": ""
        },
        {
           "name": "id",
           "number": 2,
           "label": lwpb.LABEL_REQUIRED,
           "type": lwpb.TYPE_INT32,
           "default_value": 0
        },
        {
           "name": "email",
           "number": 3,
           "label": lwpb.LABEL_OPTIONAL,
           "type": lwpb.TYPE_STRING,
           "default_value": "",
           "options": {
             "deprecated": True
           }
        },
        {
           "name": "phone",
           "number": 4,
           "label": lwpb.LABEL_REPEATED,
           "type": lwpb.TYPE_MESSAGE,
           "type_name": "PhoneNumber"
        }
      ]
    }
  ]
}

desc = lwpb.Descriptor(descdef)
desc.debug_print()

class Builder(lwpb.Decoder):

  def __init__(self, descriptor):
    lwpb.Decoder.__init__(self, descriptor)
    self.indent = 0

  def field_handler(self, message_name, field_name, value):
    if value == None:
      print "%s%s.%s" % ("  " * (self.indent-1), message_name, field_name)
    else:
      print "%s%s.%s = %s (%s)" % ("  " * (self.indent-1), message_name, field_name, value, type(value))

  def msg_start_handler(self, message_name):
    self.indent += 1

  def msg_end_handler(self, message_name):
    self.indent -= 1


d = Builder(desc)
data = file("person.pb").read()
status = d.decode(data, 1)
print "status = %d" % status

