import sys
import lwpb

arg_protofile = sys.argv[1]
arg_infile = sys.argv[2]
arg_messagename = sys.argv[3]

d = lwpb.Decoder()
protofile_def = lwpb.PROTOFILE_DEFINITION
protofile_desc = lwpb.Descriptor(protofile_def)
protofile_msg = protofile_desc.message_types()
protofile_bin = file(arg_protofile).read()

m = protofile_msg['google.protobuf.FileDescriptorSet']
schema_def = d.decode(protofile_bin, protofile_desc, m)
schema_desc = lwpb.Descriptor(schema_def['file'][0])
schema_msg = schema_desc.message_types()

m = schema_msg[arg_messagename]
f = file(arg_infile)
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
    record = d.decode(buf[pos:pos+recordlen],schema_desc,m)
    pos += recordlen
    print record

  if pos > 0:
    buf = buf[pos:]
  elif eof and len(buf):
    raise EOFError("partial record at EOF %d %s" % (pos, buf))

