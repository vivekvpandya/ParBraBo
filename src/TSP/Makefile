IDIR = ../../include
CC = gcc
CXX = g++
MPICXX = mpic++
CPP_FILES = $(join $(wildcard *.cpp), $(../framework.cpp))
OBJ_FILES = $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CC_FLAGS = -I $(IDIR) -std=c++11 -fopenmp -Wno-unused-result
LD_FLAGS = -fopenmp

out : $(OBJ_FILES) 
	$(MPICXX) $(LD_FLAGS) -o $@ $^

obj/%.o: %.cpp
	$(MPICXX) $(CC_FLAGS) -c -o $@ $<