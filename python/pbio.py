#!/usr/bin/env python

'''
  pbio - convert between protocol buffer and delimited text records
'''

import sys
import getopt
import os
import string
import re
import lwpb
import lwpb.stream
import lwpb.codec
import percent.stream


def shift(L): e = L[0] ; del L[0:1] ; return e

def main():

  reader_format = 'pb'
  writer_format = 'txt'
  delim = '\t'
  fields = []
  typename = ""
  skip = 0
  count = -1
  pb2file = None
  fin = sys.stdin
  fout = sys.stdout

  opts, args = getopt.getopt(sys.argv[1:], 'R:W:F:d:m:s:c:')

  for o, a in opts:
    if o == '-R':
      reader_format = a
    elif o == '-W':
      writer_format = a
    elif o == '-F':
      fields = a.split(',')
    elif o == '-m':
      typename = a
    elif o == '-s':
      skip = int(a)
    elif o == '-c':
      count = int(c)

  pb2file = shift(args)
  if len(args): fin = file(shift(args))
  if len(args): fout = file(shift(args), 'w')

  codec = lwpb.codec.MessageCodec(pb2file=pb2file, typename=typename)

  # reader

  if reader_format == 'pb':
    reader = lwpb.stream.StreamReader(fin, codec=codec)
  elif reader_format == 'txt':
    reader = percent.stream.PercentCodecReader(fin, '\t', fields)
  else:
    raise Exception("bad reader format")

  # writer

  if writer_format == 'pb':
    writer = lwpb.stream.StreamWriter(fout, codec=codec)
  elif writer_format == 'txt':
    writer = percent.stream.PercentCodecWriter(fout, '\t', fields)
  else:
    raise Exception("bad writer format")

  # TODO implement skip and count
  # TODO implement raw $RECORD passthrough when writing, see below

  for record in reader:

    for k in fields:
      v = None
      if k == '$RECORD':
        v = reader.current_raw
      elif k == '$NUMBER':
        v = reader.current_number
      elif k == '$OFFSET':
        v = reader.current_offset
      elif k == '$LENGTH':
        v = reader.current_length
      if v != None:
        record[k] = str(v)

    writer.write( record )

  return 0


'''
  elif mode == 'output':

    writer = lwpb.stream.StreamWriter(fout, codec=codec)

    ### If the original encoded record is included via the special
    ### $RECORD field, there is no need to re-encode all the fields.

    try:
      rawindex = fields.index('$RECORD')
    except ValueError, e:
      rawindex = -1

    for line in fin:
      values = line.rstrip('\r\n').split(delim)

      if rawindex >= 0:
        raw = escaper.decode(values[rawindex])
        writer.write_raw(record)
      else:
        flat = {}

        for i in range(0, len(values)):
          k = fields[i]
          if not k.startswith('$'):
            flat[k] = escaper.decode(values[i])

        record = unflatten(flat)
        writer.write(record)
'''


if __name__ == '__main__':
  sys.exit(main())
  #test_unflatten()
  #test_percent_codec()


