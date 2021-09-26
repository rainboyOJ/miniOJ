CXX=g++
CXXFLAGS=-g -std=c++17

SRC=$(wildcard *.cpp)
HPP_FILES=$(wildcard *.hpp)

server: $(SRC) $(HPP_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $(SRC) -lpthread


clean:
	rm -rf server
