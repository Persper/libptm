CXX=g++-5
CFLAGS=-fgnu-tm -std=c++11 -Wall
SRCS=$(wildcard *.cc)
OBJS=$(SRCS:.cc=.o)

all:	$(OBJS)

.cc.o:
	$(CXX) $(CFLAGS) $< -o $@

clean:
	$(RM) *.o
