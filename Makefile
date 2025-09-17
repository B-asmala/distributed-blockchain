# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Iinclude -Wall -g -O2

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Target executable name
TARGET = $(BIN_DIR)/blockchain

# Find all .c files in the src directory
SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Generate corresponding .o file names in the build directory
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ -lcrypto

# Compile .c files to .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean up build and bin directories
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Phony targets
.PHONY: all clean