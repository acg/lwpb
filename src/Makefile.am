SUBDIRS = .

lib_LTLIBRARIES = liblwpb.la

liblwpb_la_SOURCES = \
lwpb/core/buf.c \
lwpb/core/decoder.c \
lwpb/core/encoder.c \
lwpb/core/misc.c \
lwpb/rpc/client.c \
lwpb/rpc/server.c \
lwpb/rpc/service.c \
lwpb/rpc/direct/direct.c \
lwpb/rpc/socket/socket_server.c

liblwpb_la_CFLAGS = -I./lwpb/core -I./lwpb/rpc

bin_PROGRAMS = test_simple test_full test_full_generate test_rpc_direct

test_simple_SOURCES = \
test/test_simple.c \
test/generated/test_simple_pb2.c

test_simple_LDADD = liblwpb.la

test_full_SOURCES = \
test/test_full.c \
test/generated/test_full_pb2.c

test_full_LDADD = liblwpb.la

test_full_generate_SOURCES = \
test/test_full_generate.cc \
test/generated/test_full.pb.cc

test_full_generate_LDADD = /usr/lib/libprotobuf.la -lpthread

test_rpc_direct_SOURCES = \
test/test_rpc_direct.c \
test/generated/test_simple_pb2.c

test_rpc_direct_LDADD = liblwpb.la
