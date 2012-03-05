SUBDIRS = .

lib_LTLIBRARIES = liblwpb.la

liblwpb_la_SOURCES = \
lwpb/core/buf.c \
lwpb/core/decoder.c \
lwpb/core/encoder.c \
lwpb/core/encoder2.c \
lwpb/core/misc.c \
lwpb/rpc/client.c \
lwpb/rpc/direct.c \
lwpb/rpc/server.c \
lwpb/rpc/socket_client.c \
lwpb/rpc/socket_helper.c \
lwpb/rpc/socket_protocol_pb2.c \
lwpb/rpc/socket_server.c \
lwpb/rpc/transport.c \
lwpb/utils/struct_decoder.c \
lwpb/utils/utils.c

liblwpb_la_CFLAGS = -fPIC -Wall -I./include -I./lwpb/core -I./lwpb/rpc

