CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

all: kv_store tests

kv_store: main.cpp kv_store.cpp
	$(CXX) $(CXXFLAGS) -o kv_store main.cpp kv_store.cpp

tests: tests.cpp kv_store.cpp
	$(CXX) $(CXXFLAGS) -o tests tests.cpp kv_store.cpp

clean:
	rm -f kv_store tests
