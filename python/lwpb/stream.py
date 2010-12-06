import os
import lwpb


class StreamReader:

  def __init__(self, stream, codec=None):
    self.stream = stream
    self.codec = codec
    self.eof = False
    self.seekable = True
    self.current_raw = None
    self.current_record = None
    self.current_length = None
    self.current_offset = None
    self.current_number = -1

  def __iter__(self):
    while not self.eof:
      record = self.read()
      if record != None:
        yield record

  def read(self):
    c = self.codec
    if self.read_raw() != None:
      self.current_record = c.decode(self.current_raw)
    return self.current_record

  def read_raw(self):
    c = self.codec
    f = self.stream

    if self.seekable:
      try:
        self.current_offset = f.tell()
      except:
        self.seekable = False

    self.current_length = None
    self.current_record = None
    self.current_raw = None

    data = f.read(4)
    (self.current_length, bytes) = c.decoder.decode_32bit(data)

    if self.current_length == None:
      self.eof = True
      return None

    self.current_raw = f.read(self.current_length)
    self.current_number += 1

    return self.current_raw



class StreamWriter:

  def __init__(self, stream, codec=None):
    self.stream = stream
    self.codec = codec

  def write(self, record):
    data = self.codec.encode(record)
    self.write_raw(data)

  def write_raw(self, raw):
    recordlen = len(raw)
    self.stream.write(self.codec.encoder.encode_32bit(recordlen))
    self.stream.write(raw)

