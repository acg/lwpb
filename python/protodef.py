
import sys
import pprint
import lwpb
#import json

protodesc = lwpb.Descriptor(lwpb.protodef)
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
    print record

  if pos > 0:
    buf = buf[pos:]
  elif eof and len(buf):
    raise EOFError("partial record at EOF %d %s" % (pos, buf))

