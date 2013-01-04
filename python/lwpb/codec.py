import lwpb


class MessageCodec:

  def __init__(self, typename="", pb2=None, pb2file=None, filenum=0):

    if pb2file != "":
      pb2 = file(pb2file).read()

    self.encoder = lwpb.Encoder()
    self.decoder = lwpb.Decoder()

    pb2_definition = lwpb.PROTOFILE_DEFINITION
    pb2_descriptor = lwpb.Descriptor(pb2_definition)
    pb2_types = pb2_descriptor.message_types()
    pb2_type_fds = pb2_types['google.protobuf.FileDescriptorSet']

    self.definition = self.decoder.decode(pb2, pb2_descriptor, pb2_type_fds)
    self.descriptor = lwpb.Descriptor(self.definition['file'][filenum])
    self.types = self.descriptor.message_types()
    self.messagetype = self.types[typename]

    # Expose enumeration values as self.enums.<Name>.<Member>

    self.enums = ProtoEnums()
    for e in self.definition['file'][filenum].get('enum_type',[]):
      enum = ProtoEnum()
      for v in e['value']:
        setattr(enum,v['name'],v['number'])
      setattr(self.enums,e['name'],enum)


  def encode(self, record):
    return self.encoder.encode(record, self.descriptor, self.messagetype)

  def decode(self, data):
    return self.decoder.decode(data, self.descriptor, self.messagetype)


# These are used to expose enumeration symbols and values.

class ProtoEnums(object): pass
class ProtoEnum(object): pass


