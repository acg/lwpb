CFLAGS = -g -I/home/agrow/src/protobuf-2.3.0/src
LDFLAGS = -lprotobuf -lpthread


all : truthdb.txt

truthdb.txt : truthdb
	./truthdb > $@

truthdb : truthdb.o generated/test.pb.o python_format.o
	$(CC) -o $@ $^ $(LDFLAGS)

truthdb.o : truthdb.cc generated/test.pb.h
	$(CC) $(CFLAGS) -o $@ -c $<

python_format.o : python_format.cc
	$(CC) $(CFLAGS) -o $@ -c $<

generated/test.pb.o : generated/test.pb.cc generated/test.pb.h
	$(CC) $(CFLAGS) -o $@ -c $<

generated/test.pb.cc generated/test.pb.h : test.proto
	protoc --cpp_out=generated $^

clean :
	rm -f truthdb *.o generated/*

