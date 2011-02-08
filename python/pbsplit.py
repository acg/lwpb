#!/usr/bin/env python

'''
  pbsplit - split a protobuf stream into multiple files
'''

import sys
import getopt
import lwpb
import lwpb.stream
import lwpb.codec


def shift(L): e = L[0] ; del L[0:1] ; return e

def main():

  typename = ""
  skip = 0
  count = -1
  splitsize = 1000  # in number of records
  pb2file = None
  infile = "-"
  fin = sys.stdin
  template = None

  opts, args = getopt.getopt(sys.argv[1:], 'p:m:s:c:t:z:')

  for o, a in opts:
    if o == '-p':
      pb2file = a
    elif o == '-m':
      typename = a
    elif o == '-s':
      skip = int(a)
    elif o == '-c':
      count = int(a)
    elif o == '-t':
      template = a
    elif o == '-z':
      splitsize = int(a)

  if len(args):
    infile = shift(args)
    fin = file(infile)

  if template == None:
    template = infile+".%05u"

  codec = lwpb.codec.MessageCodec(pb2file=pb2file, typename=typename)
  reader = lwpb.stream.StreamReader(fin, codec=codec)
  writer = None
  fout = None
  outfile = None
  splitnum = 0
  splitwritten = 0
  written = 0

  for record in reader:

    if reader.current_number < skip:
      continue

    if count >= 0 and written >= count:
      break

    if fout == None:
      outfile = template % splitnum
      fout = file(outfile, 'w')
      writer = lwpb.stream.StreamWriter(fout, codec=codec)
      splitwritten = 0

    writer.write_raw( reader.current_raw )
    written += 1
    splitwritten += 1

    if splitwritten >= splitsize:
      fout.close()
      fout = None
      splitnum += 1

  if fout:
    fout.close()

  return 0


if __name__ == '__main__':
  sys.exit(main())


