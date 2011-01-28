Testing lwpb
============

To run the test suite:

    python setup.py test

The test suite uses a truth database generated from the official C++ protobuf implementation from google. A prebuilt truth database is included so you don't have to install google protobuf just to test lwpb.

To add new tests, you'll need the protoc compiler, the development libraries, and the headers from protobuf-2.3.0 or later. Edit truthdb.cc, then rebuild the truth database with:

    make CFLAGS=-I$HOME/src/protobuf-2.3.0/src

