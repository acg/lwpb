#!/usr/bin/env python

import string
from percent.codec import PercentCodec
from flat import flatten, unflatten


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
        flat[k] = self.escaper.decode(values[i])

      self.current_record = unflatten(flat)
      yield self.current_record



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


