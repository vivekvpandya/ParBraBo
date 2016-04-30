IDIR = ./include
CC = gcc
CXX = g++
MPICXX = mpic++
CPP_FILES = $(wildcard src/*.cpp)
OBJ_FILES = $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CC_FLAGS = -I $(IDIR) -std=c++11 -fopenmp -Wno-unused-result
LD_FLAGS = -fopenmp

out : $(OBJ_FILES) 
	$(MPICXX) $(LD_FLAGS) -o $@ $^

obj/%.o: src/%.cpp
	$(MPICXX) $(CC_FLAGS) -c -o $@ $<
