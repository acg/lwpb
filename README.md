lwpb
====

The "Lightweight Protocol Buffer Library" provides fast encoding and decoding of Google Protocol Buffers in C and Python.

Quick Python synopsis:

  from lwpb.codec import MessageCodec

  codec = MessageCodec( pb2file='person.pb2', typename='example.Person' )
  serialized = codec.encode( { 'name': 'John Doe', 'id': 1234  } )
  deserialized = codec.decode( serialized )

  print deserialized

This expects a compiled .proto file you can generate with:

  protoc person.proto -o person.pb2

Some distinctive things about lwpb:

  * lwpb does not require a codegen step

  * lwpb does not force OO on the programmer, encoding and decoding works with dicts

  * lwpb is fast and small

  * lwpb does not wrap Google's C++ API

The C library was originally released by Simon Kallweit: http://code.google.com/p/lwpb/. More documentation on the C API is there.

