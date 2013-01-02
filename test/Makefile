PROGRAMS = \
test_simple \
test_full \
test_full_generate \
test_rpc_direct \
test_struct_map \

# test_rpc_socket_client \
# test_rpc_socket_server \

LDFLAGS += -L../src -llwpb -lprotobuf -lpthread
CFLAGS += -I../src/include

PROTOC ?= protoc
PROTOC_FLAGS ?= -I. -I../src/include


all : $(PROGRAMS)

check : $(PROGRAMS)
	for f in $(PROGRAMS); do ./$$f; done


test_simple : test_simple.o generated/test_simple_pb2.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) 

test_full : test_full.o generated/test_full_pb2.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) 

test_full_generate : test_full_generate.o generated/test_full.pb.o
	$(CXX) -o $@ $(CFLAGS) $^ $(LDFLAGS) 

test_rpc_direct : test_rpc_direct.o generated/test_rpc_pb2.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) 

test_rpc_socket_client : test_rpc_socket_client.o generated/test_rpc_pb2.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) 

test_rpc_socket_server : test_rpc_socket_server.o generated/test_rpc_pb2.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) 

test_struct_map : test_struct_map.o generated/test_struct_map_pb2.o
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS) 


test_full_generate.o : generated/test_full.pb.h

generated/test_full.pb.o : generated/test_full.pb.cc generated/test_full.pb.h

generated/test_full.pb.h : test_full.proto.done

generated/test_full.pb.cc : test_full.proto.done

test_full.proto.done : test_full.proto
	$(PROTOC) $(PROTOC_FLAGS) --cpp_out=generated $< > $@


clean :
	rm -f *.o $(PROGRAMS) generated/*.o

