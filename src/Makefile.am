SUBDIRS = .

lib_LTLIBRARIES = liblwpb.la

liblwpb_la_SOURCES = \
lwpb/decoder.c \
lwpb/encoder.c

bin_PROGRAMS = test1

test1_SOURCES = \
test/test1.c \
test/test1_pb2.c

test1_LDADD = liblwpb.la

