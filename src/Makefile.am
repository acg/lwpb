SUBDIRS = .

lib_LTLIBRARIES = liblwpb.la

liblwpb_la_SOURCES = \
lwpb/buf.c \
lwpb/client.c \
lwpb/decoder.c \
lwpb/encoder.c \
lwpb/misc.c \
lwpb/server.c \
lwpb/service.c

bin_PROGRAMS = test1 test2 test2_generate test3

test1_SOURCES = \
test/test1.c \
test/generated/test1_pb2.c

test1_CFLAGS = -I${top_dir}/test/generated
test1_LDADD = liblwpb.la


test2_SOURCES = \
test/test2.c \
test/generated/test2_pb2.c

test2_LDADD = liblwpb.la


test2_generate_SOURCES = \
test/test2_generate.cc \
test/generated/test2.pb.cc

test2_generate_LDADD = /usr/lib/libprotobuf.la -lpthread


test3_SOURCES = \
test/test3.c \
test/generated/test1_pb2.c

test3_CFLAGS = -I${top_dir}/test/generated
test3_LDADD = liblwpb.la
