SUBDIRS = .

bin_PROGRAMS = \
test_simple \
test_full \
test_full_generate \
test_rpc_direct \
test_rpc_socket_client \
test_rpc_socket_server \
test_struct_map

test_simple_SOURCES = \
test_simple.c \
generated/test_simple_pb2.c

test_simple_LDADD = -L../src/.libs -llwpb
test_simple_CFLAGS = -I../src/include

test_full_SOURCES = \
test_full.c \
generated/test_full_pb2.c

test_full_LDADD = -L../src/.libs -llwpb
test_full_CFLAGS = -I../src/include

test_full_generate_SOURCES = \
test_full_generate.cc \
generated/test_full.pb.cc

test_full_generate_LDADD = -lprotobuf -lpthread
test_full_generate_CXXFLAGS = -I../src/include

test_rpc_direct_SOURCES = \
test_rpc_direct.c \
generated/test_rpc_pb2.c

test_rpc_direct_LDADD = -L../src/.libs -llwpb
test_rpc_direct_CFLAGS = -I../src/include

test_rpc_socket_client_SOURCES = \
test_rpc_socket_client.c \
generated/test_rpc_pb2.c

test_rpc_socket_client_LDADD = -L../src/.libs -llwpb
test_rpc_socket_client_CFLAGS = -I../src/include

test_rpc_socket_server_SOURCES = \
test_rpc_socket_server.c \
generated/test_rpc_pb2.c

test_rpc_socket_server_LDADD = -L../src/.libs -llwpb
test_rpc_socket_server_CFLAGS = -I../src/include

test_struct_map_SOURCES = \
test_struct_map.c \
generated/test_struct_map_pb2.c

test_struct_map_LDADD = -L../src/.libs -llwpb
test_struct_map_CFLAGS = -I../src/include

