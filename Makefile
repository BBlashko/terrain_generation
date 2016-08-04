PROGS = main
OBJS = main.o
OPENGLLIBRARIES = -lglfw -lGLEW -lSOIL
GXX = g++
GXXFLAGS = -g -O -lGL
CXXWARNS = -Wall -Werror

all: main

main.o : main.cpp
	$(GXX) $(GXXFLAGS) $(GXXWARNS) -c $^

main : $(OBJS)
	$(GXX) $(OPENGLLIBRARIES) $(GXXFLAGS) $(GXXWARNS) -o $@ $?
	rm -f $(OBJS)

clean:
	rm -f $(OBJS) $(PROGS)
