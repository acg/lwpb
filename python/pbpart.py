#!/usr/bin/env python

'''
  pbpart - partition a protobuf record stream into multiple files
'''

import sys
import getopt
import lwpb
import lwpb.stream
import lwpb.codec
import percent.stream


def shift(L): e = L[0] ; del L[0:1] ; return e

def main():

  key = None
  typename = ""
  begincode = None
  mapcode = None
  endcode = None
  codeglobals = {}
  pb2file = None
  pb2codec = None
  template = None
  fin = sys.stdin
  infile = '-'
  verbose = 0

  opts, args = getopt.getopt(sys.argv[1:], 'p:k:m:e:B:E:v')

  for o, a in opts:
    if o == '-p':
      pb2file = a
    elif o == '-k':
      key = a
    elif o == '-m':
      typename = a
    elif o == '-e':
      mapcode = compile(a,"-e '%s'" % a,'exec')
    elif o == '-B':
      begincode = compile(a,"-B '%s'" % a,'exec')
    elif o == '-E':
      endcode = compile(a,"-E '%s'" % a,'exec')
    elif o == '-t':
      template = a
    elif o == '-v':
      verbose += 1

  if len(args):
    infile = shift(args)
    fin = file(infile)

  if template == None:
    template = infile+".%s"

  if key == None:
    raise Exception("missing key parameter, specify with -k")

  pb2codec = lwpb.codec.MessageCodec(pb2file=pb2file, typename=typename)
  reader = lwpb.stream.StreamReader(fin, codec=pb2codec)
  fouts = {}
  writers = {}

  if begincode != None:
    exec begincode in codeglobals


  for record in reader:

    if mapcode != None:
      exec mapcode in codeglobals, record

    partition = str(record[key])
    fout = fouts.get(partition, None)
    writer = writers.get(partition, None)

    if not fout:
      outfile = template % partition
      if verbose: print >> sys.stderr, outfile
      fout = file(outfile, 'w')
      fouts[partition] = fout
      writer = lwpb.stream.StreamWriter(fout, codec=pb2codec)
      writers[partition] = writer

    writer.write_raw(reader.current_raw)


  for partition, fout in fouts.items():
    fout.close()

  if endcode != None:
    exec endcode in codeglobals

  return 0


if __name__ == '__main__':
  sys.exit(main())


