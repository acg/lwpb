TARGET = src/liblwpb.a

SOURCES = \
src/lwpb/core/buf.c \
src/lwpb/core/decoder.c \
src/lwpb/core/encoder.c \
src/lwpb/core/encoder2.c \
src/lwpb/core/misc.c \
src/lwpb/rpc/client.c \
src/lwpb/rpc/direct.c \
src/lwpb/rpc/server.c \
src/lwpb/rpc/socket_client.c \
src/lwpb/rpc/socket_helper.c \
src/lwpb/rpc/socket_protocol_pb2.c \
src/lwpb/rpc/socket_server.c \
src/lwpb/rpc/transport.c \
src/lwpb/utils/struct_decoder.c \
src/lwpb/utils/utils.c

OBJECTS = $(SOURCES:%.c=%.o)

CFLAGS = -fPIC -Wall -I./src/include -I./src/lwpb/core -I./src/lwpb/rpc


all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(AR) -cr $@ $^

check :
	$(MAKE) -C ./test check

clean :
	rm -f $(TARGET) $(OBJECTS)

