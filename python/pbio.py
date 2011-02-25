#!/usr/bin/env python

'''
  pbio - convert between protocol buffer and delimited text records
'''

import sys
import getopt


def shift(L): e = L[0] ; del L[0:1] ; return e

def main():

  reader_format = 'pb'
  writer_format = 'txt'
  delim = '\t'
  infields = []
  outfields = []
  typename = ""
  skip = 0
  count = -1
  begincode = None
  mapcode = None
  endcode = None
  codeglobals = {}
  aggregate = None
  pb2file = None
  codec = None
  fin = sys.stdin
  fout = sys.stdout

  opts, args = getopt.getopt(sys.argv[1:], 'R:W:p:F:I:O:d:m:s:c:e:B:E:A:')

  for o, a in opts:
    if o == '-R':
      reader_format = a
    elif o == '-W':
      writer_format = a
    elif o == '-d':
      delim = a
    elif o == '-p':
      pb2file = a
    elif o == '-F':
      infields = outfields = a.split(',')
    elif o == '-I':
      infields = a.split(',')
    elif o == '-O':
      outfields = a.split(',')
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
    elif o == '-A':
      aggregate = a

  if len(args): fin = file(shift(args))
  if len(args): fout = file(shift(args), 'w')

  if reader_format == 'pb' or writer_format == 'pb':
    import lwpb.codec
    pb2codec = lwpb.codec.MessageCodec( pb2file=pb2file, typename=typename )

  if reader_format == 'txt':
    import percent.codec
    intxtcodec = percent.codec.PercentCodec( infields, delim )

  if writer_format == 'txt':
    import percent.codec
    outtxtcodec = percent.codec.PercentCodec( outfields, delim )

  # reader

  if reader_format == 'pb':
    import lwpb.stream
    reader = lwpb.stream.StreamReader( fin, codec=pb2codec )
  elif reader_format == 'txt':
    import percent.stream
    reader = percent.stream.PercentCodecReader( fin, intxtcodec )
  else:
    raise Exception("bad reader format")

  # writer

  if writer_format == 'pb':
    import lwpb.stream
    writer = lwpb.stream.StreamWriter( fout, codec=pb2codec )
  elif writer_format == 'txt':
    import percent.stream
    writer = percent.stream.PercentCodecWriter( fout, outtxtcodec )
  else:
    raise Exception("bad writer format")

  def emit(record):
    if writer_format == 'pb' and '_RECORD' in record:
      writer.write_raw( record['_RECORD'] )
    else:
      writer.write( record )

  codeglobals['emit'] = emit

  written = 0
  lastkey = None

  if begincode != None:
    exec begincode in codeglobals


  # record reading loop

  for record in reader:

    # skip to first record

    if reader.current_number < skip:
      continue

    # stop after <count> records

    if count >= 0 and written >= count:
      break

    # in aggregate mode (-A key), run -B and -E code at key boundaries

    if aggregate != None:
      key = record.get( aggregate, None )
      if lastkey != None and key != lastkey:
        if endcode != None: exec endcode in codeglobals
        if begincode != None: exec begincode in codeglobals
      lastkey = key

    if mapcode != None:
      exec mapcode in codeglobals, record

    if record.get('_SKIP', False):
      continue

    for k in infields:

      if k in record:
        continue

      v = None

      if k == '_RECORD':
        v = reader.current_raw
      elif k == '_NUMBER':
        v = reader.current_number
      elif k == '_OFFSET':
        v = reader.current_offset
      elif k == '_LENGTH':
        v = reader.current_length

      if v != None:
        record[k] = str(v)

    emit(record)

    written += 1


  if endcode != None:
    exec endcode in codeglobals

  return 0


if __name__ == '__main__':
  sys.exit(main())


