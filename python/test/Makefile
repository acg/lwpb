TARGET_FILES = truthdb.txt test.proto.pb
TARGET = truthdb

SOURCES_CXX = \
	truthdb.cc \
	python_format.cc \
	generated/test.pb.cc

OBJECTS = $(SOURCES_CXX:.cc=.o) 
DEPS = $(SOURCES_CXX:.cc=.d) 

PROTOC = protoc
CFLAGS += -g
CXXFLAGS += $(CFLAGS) 
LIBS = -lprotobuf -lpthread


all : $(DEPS) $(TARGET_FILES)

test.proto.pb : test.proto
	$(PROTOC) $< -o $@

truthdb.txt : truthdb
	./truthdb > $@

$(TARGET) : $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o : %.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<

# Trick for setting up header file dependencies

%.d : %.cc
	set -e; $(CXX) $(CXXFLAGS) -MM $(LDFLAGS) $< | \
	sed -e "s@^\(.*\)\.o:@\1.d \1.o:@" > $@

generated/test.pb.cc generated/test.pb.h : test.proto
	$(PROTOC) --cpp_out=generated $^

clean :
	rm -f $(TARGET) $(TARGET_FILES) *.o generated/*

