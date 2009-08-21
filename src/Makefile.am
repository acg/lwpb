SUBDIRS = .

lib_LTLIBRARIES = liblwpb.la

liblwpb_ladir = lwpb

liblwpb_la_HEADERS = \
include/lwpb/lwpb.h \
include/lwpb/core/debug.h \
include/lwpb/core/decoder.h \
include/lwpb/core/encoder.h \
include/lwpb/core/misc.h \
include/lwpb/core/types.h \
include/lwpb/rpc/client.h \
include/lwpb/rpc/direct.h \
include/lwpb/rpc/server.h \
include/lwpb/rpc/socket_client.h \
include/lwpb/rpc/socket_server.h \
include/lwpb/rpc/transport.h \
include/lwpb/utils/struct_decoder.h \
include/lwpb/utils/struct_map.h

liblwpb_la_SOURCES = \
lwpb/core/buf.c \
lwpb/core/decoder.c \
lwpb/core/encoder.c \
lwpb/core/misc.c \
lwpb/rpc/client.c \
lwpb/rpc/direct.c \
lwpb/rpc/server.c \
lwpb/rpc/socket_client.c \
lwpb/rpc/socket_helper.c \
lwpb/rpc/socket_protocol_pb2.c \
lwpb/rpc/socket_server.c \
lwpb/rpc/transport.c \
lwpb/utils/struct_decoder.c

liblwpb_la_CFLAGS = -I./include -I./lwpb/core -I./lwpb/rpc

bin_PROGRAMS = \
test_simple \
test_full \
test_full_generate \
test_rpc_direct \
test_rpc_socket_client \
test_rpc_socket_server \
test_struct_map

test_simple_SOURCES = \
test/test_simple.c \
test/generated/test_simple_pb2.c

test_simple_LDADD = liblwpb.la
test_simple_CFLAGS = -I./include

test_full_SOURCES = \
test/test_full.c \
test/generated/test_full_pb2.c

test_full_LDADD = liblwpb.la
test_full_CFLAGS = -I./include

test_full_generate_SOURCES = \
test/test_full_generate.cc \
test/generated/test_full.pb.cc

test_full_generate_LDADD = /usr/local/lib/libprotobuf.la -lpthread
test_full_generate_CFLAGS = -I./include

test_rpc_direct_SOURCES = \
test/test_rpc_direct.c \
test/generated/test_rpc_pb2.c

test_rpc_direct_LDADD = liblwpb.la
test_rpc_direct_CFLAGS = -I./include

test_rpc_socket_client_SOURCES = \
test/test_rpc_socket_client.c \
test/generated/test_rpc_pb2.c

test_rpc_socket_client_LDADD = liblwpb.la
test_rpc_socket_client_CFLAGS = -I./include

test_rpc_socket_server_SOURCES = \
test/test_rpc_socket_server.c \
test/generated/test_rpc_pb2.c

test_rpc_socket_server_LDADD = liblwpb.la
test_rpc_socket_server_CFLAGS = -I./include

test_struct_map_SOURCES = \
test/test_struct_map.c \
test/generated/test_struct_map_pb2.c

test_struct_map_LDADD = liblwpb.la
test_struct_map_CFLAGS = -I./include

