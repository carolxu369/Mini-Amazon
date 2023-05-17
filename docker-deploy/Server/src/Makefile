CXX=g++
CXXFLAGS=-std=c++11
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, %.o, $(SRCS))

all: main

main: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) protobuf/*.pb.cc -lpthread -lboost_thread -lprotobuf -lpqxx -lpq

clean:
	rm -f *~ *.o *.so
