SUBDIRS = .

lib_LTLIBRARIES = liblwpb.la

liblwpb_la_SOURCES = \
lwpb/core/buf.c \
lwpb/core/decoder.c \
lwpb/core/encoder.c \
lwpb/core/misc.c \
lwpb/client.c \
lwpb/server.c \
lwpb/service.c \
lwpb/service/tcp_server.c

liblwpb_la_CFLAGS = -I${top_dir}/lwpb/core

bin_PROGRAMS = test_simple test_full test_full_generate test3

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

test3_SOURCES = \
test/test3.c \
test/generated/test_simple_pb2.c

test3_LDADD = liblwpb.la
