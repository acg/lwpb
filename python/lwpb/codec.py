import lwpb


class MessageCodec:

  def __init__(self, typename="", pb2=None, pb2file=None):

    if pb2file != "":
      pb2 = file(pb2file).read()

    self.encoder = lwpb.Encoder()
    self.decoder = lwpb.Decoder()

    pb2_definition = lwpb.PROTOFILE_DEFINITION
    pb2_descriptor = lwpb.Descriptor(pb2_definition)
    pb2_types = pb2_descriptor.message_types()
    pb2_type_fds = pb2_types['google.protobuf.FileDescriptorSet']

    self.definition = self.decoder.decode(pb2, pb2_descriptor, pb2_type_fds)
    self.descriptor = lwpb.Descriptor(self.definition['file'][0])
    self.types = self.descriptor.message_types()
    self.messagetype = self.types[typename]


  def encode(self, record):
    return self.encoder.encode(record, self.descriptor, self.messagetype)

  def decode(self, data):
    return self.decoder.decode(data, self.descriptor, self.messagetype)

