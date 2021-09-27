CXX=g++
CXXFLAGS=-g -std=c++17

SRC=$(wildcard *.cpp)
HPP_FILES=$(wildcard *.hpp)

HTTP_CPP=$(wildcard http/*.cpp)
HTTP_HPP=$(wildcard http/*.hpp)

server: $(SRC) $(HPP_FILES) $(HTTP_CPP) $(HTTP_HPP)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) $(HTTP_CPP) -lpthread


clean:
	rm -rf server
