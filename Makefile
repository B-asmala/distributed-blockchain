# Compiler
CC = gcc
CFLAGS = -Iinclude -Wall -g -O2

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Executables
NODE_TARGET = $(BIN_DIR)/node
ORCH_TARGET = $(BIN_DIR)/orch

# Source files
NODE_SOURCES = $(filter-out $(SRC_DIR)/orch.c, $(wildcard $(SRC_DIR)/*.c))
ORCH_SOURCES = $(filter-out $(SRC_DIR)/node.c, $(wildcard $(SRC_DIR)/*.c))

# Object files
NODE_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(NODE_SOURCES))
ORCH_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(ORCH_SOURCES))

# Default target
all: $(NODE_TARGET) $(ORCH_TARGET)

# Link each executable
$(NODE_TARGET): $(NODE_OBJECTS) | $(BIN_DIR)
	$(CC) $(NODE_OBJECTS) -o $@ -lpthread -lcrypto

$(ORCH_TARGET): $(ORCH_OBJECTS) | $(BIN_DIR)
	$(CC) $(ORCH_OBJECTS) -o $@ -lpthread -lcrypto

# Compile .c → .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure dirs exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean
