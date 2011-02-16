#!/usr/bin/env python

import string
from percent.codec import PercentCodec


class PercentCodecReader:

  def __init__(self, f, codec):
    self.f = f
    self.codec = codec
    self.current_number = 0
    self.current_offset = 0

  def __iter__(self):

    eof = False

    while not eof:
      record = self.read()
      if record == None:
        eof = True
      else:
        yield record

  def read(self):

    line = self.f.readline()
    if line == "":
      return None

    self.current_raw = line
    self.current_number += 1
    self.current_length = len(line)
    self.current_offset += self.current_length
    self.current_record = self.codec.decode( line )

    return self.current_record


class PercentCodecWriter:

  def __init__(self, f, codec):
    self.f = f
    self.codec = codec

  def write(self, record):
    line = self.codec.encode( record )
    print >> self.f, line
    return len(line)


