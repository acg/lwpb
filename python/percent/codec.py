#!/usr/bin/env python
# coding=utf-8

import string
import re
from flat import flatten, unflatten


class PercentCodec:

  # TODO this could be SOOO much faster in C, rewrite

  URI_RESERVED = "!*'();:@&=+$,/?#[]"
  URI_UNRESERVED = string.uppercase + string.lowercase + "-_.~"
  PRINTABLE = string.printable
  WHITESPACE = string.whitespace


  def __init__(self, fields, delim, safe=PRINTABLE, unsafe=WHITESPACE):

    self.fields = fields
    self.delim = delim

    safeset = set([c for c in safe]) - set([c for c in unsafe+delim])
    self.chrtohex = dict((chr(i), '%%%02x' % i) for i in range(256))
    for c in safeset: self.chrtohex[c] = c
    self.chrtohex['%'] = '%%'

    h1 = dict( ('%02x' % i, chr(i)) for i in range(256) )
    h2 = dict( ('%02X' % i, chr(i)) for i in range(256) )
    self.hextochr = dict( h1, **h2 )
    self.hextochr['%'] = '%'

    self.regex = re.compile('%(%|[a-fA-F0-9]{2})')


  def encode(self, record):

    flat = flatten(record)
    values = []

    for k in self.fields:
      if k in flat:
        v = flat[k]
      else:
        v = ""
      v = self.escape(str(v))
      values.append(v)

    data = self.delim.join(values)
    return data


  def decode(self, data):

    maxsplit = len(self.fields)-1
    if maxsplit < 0: maxsplit = 0
    values = data.rstrip('\r\n').split(self.delim, maxsplit)
    flat = {}

    for i in range(0, len(values)):
      k = self.fields[i]
      flat[k] = self.unescape(values[i])

    record = unflatten(flat)
    return record


  def escape(self, s):
    return ''.join([self.chrtohex[c] for c in s])

  def unescape(self, s):
    return re.sub(self.regex, lambda m: self.hextochr[m.group(1)], s)


def test_percent_codec():
  # FIXME make this a real unit test
  codec = PercentCodec([], '\t', safe=string.printable, unsafe=" \r\n")
  s = "%abc def\nghi§xyz"
  print s
  e = codec.encode_string(s)
  print e
  d = codec.decode_string(e)
  print d


