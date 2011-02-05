#!/usr/bin/env python
# coding=utf-8

'''
  pbio - convert between protocol buffer and delimited text records
'''

# TODO bit of a mismash right now, move supporting code out of here

import sys
import getopt
import os
import string
import re
import lwpb
import lwpb.stream
import lwpb.codec


def unflatten(src, dst={}):

  for k, v in src.items():
    path = k.split(".")
    p = dst

    for i in range(0, len(path)):

      if i == len(path)-1:
        default = v
      elif path[i+1].isdigit():
        default = []
      else:
        default = {}

      part = path[i]

      if part.isdigit():
        part = int(part)
        if part >= len(p):
          p.extend([None] * (part-len(p)+1))
        p[part] = default
      elif not part in p:
        p[part] = default

      p = p[part]

  return dst


def flatten(src):

  flat = {}

  if isinstance(src, list):
    for i in range(0, len(src)):
      flat[str(i)] = flatten(src[i])
  elif isinstance(src, dict):
    for k, v in src.items():
      flat[k] = flatten(v)
  else:
    return src

  dst = {}

  for k1, v1 in flat.items():
    if isinstance(v1, dict):
      for k2, v2 in v1.items():
        dst["%s.%s" % (k1,k2)] = v2
    else:
      dst[k1] = v1

  return dst


def test_unflatten():

  src = {
    "a.b.c": 42,
    "a.b.d": 43,
    "d.e.g": 12,
    "d.e.f.0": 10,
    "d.e.f.1": 20,
    "d.e.f.2": 30,
  }

  dst = unflatten(src)
  print dst
  print flatten(dst)


class PercentCodec:

  import string
  import re

  URI_RESERVED = "!*'();:@&=+$,/?#[]"
  URI_UNRESERVED = string.uppercase + string.lowercase + "-_.~"

  def __init__(self, safe=URI_UNRESERVED, unsafe=URI_RESERVED):

    safeset = set([c for c in safe]) - set([c for c in unsafe])
    self.chrtohex = dict((chr(i), '%%%02x' % i) for i in range(256))
    for c in safeset: self.chrtohex[c] = c
    self.chrtohex['%'] = '%%'

    self.hextochr = dict(('%02x' % i, chr(i)) for i in range(256))
    self.hextochr['%'] = '%'

    self.regex = re.compile('%(%|[a-fA-F0-9]{2})')

  def encode(self, s):
    return ''.join([self.chrtohex[c] for c in s])

  def decode(self, s):
    return re.sub(self.regex, lambda m: self.hextochr[m.group(1)], s)


def test_percent_codec():
  codec = PercentCodec(safe=string.printable, unsafe=" \r\n")
  s = "%abc def\nghi§xyz"
  print s
  e = codec.encode(s)
  print e
  d = codec.decode(e)
  print d


class PercentCodecReader:

  def __init__(self, f, delim, fields):
    self.f = f
    self.delim = delim
    self.fields = fields
    self.escaper = PercentCodec(safe=string.printable, unsafe="\r\n\t\v\f"+delim)

  def __iter__(self):

    self.current_number = 0
    self.current_offset = 0

    for line in self.f:

      self.current_raw = line
      self.current_number += 1
      self.current_length = len(line)
      self.current_offset += self.current_length

      values = line.rstrip('\r\n').split(self.delim)
      flat = {}

      for i in range(0, len(values)):
        k = self.fields[i]
        if not k.startswith('$'):
          flat[k] = self.escaper.decode(values[i])

      record = unflatten(flat)
      yield record



class PercentCodecWriter:

  def __init__(self, f, delim, fields):
    self.f = f
    self.delim = delim
    self.fields = fields
    self.escaper = PercentCodec(safe=string.printable, unsafe="\r\n\t\v\f"+delim)

  def write(self, record):

    flat = flatten(record)
    values = []

    for k in self.fields:
      if k in flat:
        v = flat[k]
      else:
        v = ""
      v = self.escaper.encode(str(v))
      values.append(v)

    line = self.delim.join(values)
    print >> self.f, line
    return len(line)


def shift(L):
  e = L[0]
  del L[0:1]
  return e


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

  codec = lwpb.codec.MessageCodec(pb2file=pb2file, typename=typename )

  # reader

  if reader_format == 'pb':
    reader = lwpb.stream.StreamReader(fin, codec=codec)
  elif reader_format == 'txt':
    reader = PercentCodecReader(fin, '\t', fields)
  else:
    raise Exception("bad reader format")

  # writer

  if writer_format == 'pb':
    writer = lwpb.stream.StreamWriter(fout, codec=codec)
  elif writer_format == 'txt':
    writer = PercentCodecWriter(fout, '\t', fields)
  else:
    raise Exception("bad writer format")

  # TODO implement skip and count
  # TODO implement raw $RECORD passthrough when writing

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


