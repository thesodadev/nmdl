BIN_NAME = nmdl
SRC_DIR = src
BUILD_DIR = build

SRC_FILES = main.cpp nmdl_writer.cpp obj_reader.cpp

SRC_OBJ_FILES = $(patsubst %,$(BUILD_DIR)/%,$(addsuffix .o,$(basename $(SRC_FILES))))
BIN_PATH = $(BUILD_DIR)/$(BIN_NAME)

CXXFLAGS = -std=c++2a -Wall -O2

$(BIN_PATH): $(SRC_OBJ_FILES)
	$(CXX) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

.PHONY: all clean build rebuild install

all: build
build: path_builder $(BIN_PATH)
rebuild: clean build
clean:
	rm -rf $(BUILD_DIR)
path_builder:
	mkdir -p $(BUILD_DIR)

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

install: $(BIN_PATH)
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $(BIN_PATH) $(DESTDIR)$(PREFIX)/bin/
