CC = clang
CXX = clang++
CXXFLAGS += -std=c++11 -stdlib=libc++

OBJ = naming.o

all: cmain ccmain

cmain: main.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
ccmain: main.cc $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@
naming.o: naming.cc naming.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

test: testc testcc
testc: cmain
	./cmain
	./cmain
testcc: ccmain
	./ccmain
	./ccmain

clean:
	$(RM) cmain ccmain $(OBJ)
