CXX		  := g++
CXX_FLAGS := -Wall -Wextra -std=c++17 -ggdb
LDFLAGS := -lgtest

BIN		:= out
SRC		:= src
INCLUDE	:= include -Iinclude/MemoryManagement
LIB		:= lib

LIBRARIES	:=
EXECUTABLE	:= main


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES) $(LDFLAGS)

clean:
	-rm $(BIN)/*.o
