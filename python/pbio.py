#!/usr/bin/env python

'''
  pbio - convert between protocol buffer and delimited text records
'''

import sys
import getopt
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
  begincode = None
  mapcode = None
  endcode = None
  codeglobals = {}
  pb2file = None
  pb2codec = None
  fin = sys.stdin
  fout = sys.stdout

  opts, args = getopt.getopt(sys.argv[1:], 'R:W:p:F:d:m:s:c:e:B:E:')

  for o, a in opts:
    if o == '-R':
      reader_format = a
    elif o == '-W':
      writer_format = a
    elif o == '-p':
      pb2file = a
    elif o == '-F':
      fields = a.split(',')
    elif o == '-m':
      typename = a
    elif o == '-s':
      skip = int(a)
    elif o == '-c':
      count = int(a)
    elif o == '-e':
      mapcode = compile(a,"-e '%s'" % a,'exec')
    elif o == '-B':
      begincode = compile(a,"-B '%s'" % a,'exec')
    elif o == '-E':
      endcode = compile(a,"-E '%s'" % a,'exec')

  if len(args): fin = file(shift(args))
  if len(args): fout = file(shift(args), 'w')

  if pb2file:
    pb2codec = lwpb.codec.MessageCodec(pb2file=pb2file, typename=typename)

  # reader

  if reader_format == 'pb':
    reader = lwpb.stream.StreamReader(fin, codec=pb2codec)
  elif reader_format == 'txt':
    reader = percent.stream.PercentCodecReader(fin, '\t', fields)
  else:
    raise Exception("bad reader format")

  # writer

  if writer_format == 'pb':
    writer = lwpb.stream.StreamWriter(fout, codec=pb2codec)
  elif writer_format == 'txt':
    writer = percent.stream.PercentCodecWriter(fout, '\t', fields)
  else:
    raise Exception("bad writer format")

  written = 0

  if begincode != None:
    exec begincode in codeglobals


  # record reading loop

  for record in reader:

    if reader.current_number < skip:
      continue

    if count >= 0 and written >= count:
      break

    if mapcode != None:
      exec mapcode in codeglobals, record

    for k in fields:

      if k in record:
        continue

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

    if writer_format == 'pb' and '$RECORD' in record:
      writer.write_raw( record['$RECORD'] )
    else:
      writer.write( record )

    written += 1


  if endcode != None:
    exec endcode in codeglobals

  return 0


if __name__ == '__main__':
  sys.exit(main())


