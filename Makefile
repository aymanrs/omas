CXX = g++
EXEC = plotgen.out
CXXFLAGS = -O2 -Iinclude --std=c++23 -Wall
LDFLAGS = -O2
SRC = $(wildcard src/*.cpp)
OBJ = $(patsubst src/%.cpp,obj/%.o,$(SRC))
INCLUDES = $(wildcard include/*.hpp)

all: bin/$(EXEC)

obj/%.o: src/%.cpp $(INCLUDES)
	$(CXX) -o $@ -c $< $(CXXFLAGS)

bin/$(EXEC): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

%.plot: data/%.plot plot.py
	.venv/bin/python3 plot.py $<

clean:
	rm obj/*

mrproper: clean
	rm bin/*
	rm data/*