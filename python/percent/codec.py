#!/usr/bin/env python
# coding=utf-8

import string
import re


class PercentCodec:

  # TODO this could be SOOO much faster in C, rewrite

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


