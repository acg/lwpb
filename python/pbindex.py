#!/usr/bin/env python

'''
  pbindex - create an index on field in a protobuf stream
    currently only supports one index type: -i cdb
'''

import sys
import getopt
import lwpb
import lwpb.stream
import lwpb.codec


def shift(L): e = L[0] ; del L[0:1] ; return e

def main():

  key = None
  typename = ""
  pb2file = None
  pb2codec = None
  indextype = None
  indexer = None
  outfile = None
  tempfile = None
  fin = sys.stdin
  infile = '-'
  verbose = 0

  opts, args = getopt.getopt(sys.argv[1:], 'p:k:i:o:t:m:v')

  for o, a in opts:
    if o == '-p':
      pb2file = a
    elif o == '-k':
      key = a
    elif o == '-m':
      typename = a
    elif o == '-o':
      outfile = a
    elif o == '-t':
      tempfile = a
    elif o == '-i':
      indextype = a
    elif o == '-v':
      verbose += 1

  if len(args):
    infile = shift(args)
    fin = file(infile)

  if key == None:
    raise Exception("missing key parameter, specify with -k")

  # create the indexer object

  if indextype == 'cdb':

    import cdb
    outfile = "%s.%s.%s.idx" % (infile, key, indextype)
    tempfile = "%s.%s.%s.tmp" % (infile, key, indextype)
    indexer = cdb.cdbmake( outfile, tempfile )

  elif indextype == None:

    raise Exception("missing index type parameter, specify with -i")

  # create the stream reader

  pb2codec = lwpb.codec.MessageCodec(pb2file=pb2file, typename=typename)
  reader = lwpb.stream.StreamReader(fin, codec=pb2codec)

  # index all the records

  for record in reader:
    indexkey = str(record[key])
    indexval = str(reader.current_offset)
    if verbose: print >> sys.stderr, indexkey
    indexer.add( indexkey, indexval )

  indexer.finish()

  return 0


if __name__ == '__main__':
  sys.exit(main())


