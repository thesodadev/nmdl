BIN_NAME = nobj

SRC_FILES = nobj.cpp
OBJ_FILES = $(addsuffix .o,$(basename $(SRC_FILES)))

INCLUDE_DIRS = -I$(STEROIDS_DIR)/include

CXXFLAGS = -std=c++2a -Wall -O2

$(BIN_NAME): $(OBJ_FILES)
	$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $^ -o $@

.PHONY: all clean build rebuild

all: build
build: $(BIN_NAME)
rebuild: clean build
clean:
	rm -rf *.o $(BIN_NAME)