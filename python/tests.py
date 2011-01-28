import unittest
import lwpb


class DecoderTestCase(unittest.TestCase):

  def __init__(self, **keywords):
    unittest.TestCase.__init__(self)

    for k, v in keywords.items():
      setattr(self, k, v)

  def runTest(self):

    self.assertEqual(
      self.decoder.decode(self.indata, self.descriptor, self.msgnum),
      self.outdata)

  def shortDescription(self):
    return self.name


class EncoderTestCase(unittest.TestCase):

  def __init__(self, **keywords):
    unittest.TestCase.__init__(self)

    for k, v in keywords.items():
      setattr(self, k, v)

  def runTest(self):

    self.assertEqual(
      self.encoder.encode(self.indata, self.descriptor, self.msgnum),
      self.outdata)

  def shortDescription(self):
    return self.name


def run(pbfile, truthdbfile):

  suite = unittest.TestSuite()

  decoder = lwpb.Decoder()
  encoder = lwpb.Encoder()

  protofile_descriptor = lwpb.Descriptor(lwpb.PROTOFILE_DEFINITION)
  protofile_messages = protofile_descriptor.message_types()
  protofile_bin = file(pbfile).read()

  msgnum = protofile_messages['google.protobuf.FileDescriptorSet']
  schema_def = decoder.decode(protofile_bin, protofile_descriptor, msgnum)
  schema_descriptor = lwpb.Descriptor(schema_def['file'][0])
  schema_messages = schema_descriptor.message_types()

  block = []

  f = open(truthdbfile)

  for line in f:

    line = line.rstrip('\r\n')

    if line != "":

      block.append(line)

    elif len(block) >= 4:

      (name, msgname, pystr, pb2str) = block
      msgnum = schema_messages[msgname]
      pydata = eval(pystr)
      pbdata = pb2str.strip('"').decode('string_escape')

      suite.addTest(DecoderTestCase(
        name="Decode %s" % name,
        decoder=decoder,
        descriptor=schema_descriptor,
        msgnum=msgnum,
        indata=pbdata,
        outdata=pydata,
      ))

      suite.addTest(EncoderTestCase(
        name="Encode %s" % name,
        encoder=encoder,
        descriptor=schema_descriptor,
        msgnum=msgnum,
        indata=pydata,
        outdata=pbdata,
      ))

      block = []

  runner = unittest.TextTestRunner(verbosity=2)
  runner.run(suite)


if __name__ == '__main__':

  import sys

  if len(sys.argv) != 3:
    print >> sys.stderr, "usage: %s pbfile truthfile" % sys.argv[0]
    sys.exit(1)

  run( sys.argv[1], sys.argv[2] )


