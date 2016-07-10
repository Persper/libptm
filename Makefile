CC = clang
CXX = clang++
CXXFLAGS += -std=c++11

OBJ = naming.o MurmurHash3.o

all: cmain ccmain

cmain: main.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
ccmain: main.cc $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@
naming.o: naming.cc
	$(CXX) -c $(CXXFLAGS) $< -o $@
MurmurHash3.o: utils/MurmurHash3.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

test: testc testcc
testc: cmain
	./cmain
testcc: ccmain
	./ccmain

clean:
	$(RM) cmain ccmain $(OBJ)
