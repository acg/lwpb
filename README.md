lwpb
====

The "Lightweight Protocol Buffer Library" provides fast encoding and decoding of Google Protocol Buffers in C and Python.

Some distinctive things about lwpb:

  * lwpb does not require a codegen step.

  * lwpb does not force OO on the programmer. Encoding and decoding works with dicts in Python, and plain old structures in C.

  * lwpb is fast and small. The C library is 31kb stripped. The Python module is mostly written in C and performs well in benchmarks.

  * lwpb does not depend on Google's C++ API or anything outside the Python standard libraries.

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

Currently, the test step requires google's protobuf 2.3.0+.

