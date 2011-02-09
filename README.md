lwpb
====

The "Lightweight Protocol Buffer Library" provides fast encoding and decoding of Google Protocol Buffers in C and Python.

Some distinctive things about lwpb:

  * lwpb does not require a codegen step.

  * lwpb does not force OO on the programmer. Encoding and decoding works with dicts in Python, and plain old structures in C.

  * lwpb is fast and small. The C library is 31kb stripped. The Python module is mostly written in C and [performs well](#performance) in benchmarks.

  * lwpb does not depend on Google's C++ API or anything outside the Python standard libraries.

  * lwpb supports [most features](#completeness) of the protocol buffer serialization format -- probably, all the ones you care about. :)

Python library
--------------

Quick Python synopsis:

    from lwpb.codec import MessageCodec

    codec = MessageCodec( pb2file='person.pb2', typename='example.Person' )
    serialized = codec.encode( { 'name': 'John Doe', 'id': 1234  } )
    deserialized = codec.decode( serialized )

    print deserialized

This expects a compiled .proto file you can generate with:

    protoc person.proto -o person.pb2

To build the Python module, first compile the C library (see below), then:

    cd python
    ./build_inplace
    python setup.py test
    python setup.py build
    python setup.py install

For Debian and Ubuntu users:

    cd python
    debian/rules binary
    dpkg -i ../*.deb

C library
---------

The C library was originally released by Simon Kallweit. More documentation on the C API can be found here: [ http://code.google.com/p/lwpb/ ](http://code.google.com/p/lwpb/)

To compile:

    ./autogen.sh && ./configure && make

Currently, the test build step requires Google's protobuf 2.3.0+. If you're just interested in building the Python library, you can ignore build failures in the top level test/ directory.

<span id="performance"></span>

Performance
-----------

From a [recent benchmark][fastpb-benchmark] which included the lwpb Python module (lower times are better):

    JSON
    3.56521892548

    SimpleJSON 
    0.727998971939

    Protocol Buffer (fast)
    0.38397192955

    Protocol Buffer (standard)
    4.86640501022

    Protocol Buffer (lwpb)
    0.323328971863

    cPickle
    0.811990976334

[fastpb-benchmark]: https://github.com/Greplin/fast-python-pb/tree/master/benchmark

<span id="completeness"></span>

Completeness
------------

Supported types:

  * INT32
  * UINT32
  * INT64
  * UINT64
  * FIXED32
  * FIXED64
  * SFIXED32
  * SFIXED64
  * DOUBLE
  * FLOAT
  * BOOL
  * STRING
  * MESSAGE
  * BYTES
  * ENUM

That is to say, everything but GROUP, which Google has deprecated.

Supported options:

  * packed repeated fields

Supported by the C library, but not by the Python library:

  * services
  * RPC

Not supported:

  * extensions


