CXX=g++
CXXFLAGS=-g -std=c++17

SRC=$(wildcard *.cpp)
HPP_FILES=$(wildcard *.hpp)

HTTP_CPP=$(wildcard http/*.cpp)
HTTP_CPP+=$(wildcard judge/*.cpp)
HTTP_HPP=$(wildcard http/*.hpp)
HTTP_HPP+=$(wildcard judge/*.hpp)

TARGET=$(patsubst %.cpp,%.o,$(SRC))
TARGET+=$(patsubst %.cpp,%.o,$(HTTP_CPP))

server: $(TARGET) $(HPP_FILES) $(HTTP_HPP)
	$(CXX) $(CXXFLAGS) -o $@ $(TARGET) -lpthread

%.o: %.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $< -lpthread


clean::
	rm -rf server $(TARGET)

