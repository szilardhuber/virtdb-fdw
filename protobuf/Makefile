
all: data_interface

data_interface: data.pb.cc data.pb.h

data.pb.cc data.pb.h: data.proto
	protoc --cpp_out=. data.proto

clean:
	rm -f data.pb.cc data.pb.h
