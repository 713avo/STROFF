# STROFF - Document Formatting Language
# Makefile for building and managing the project

# Variables
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SRCDIR = src
BINDIR = bin
TARGET = $(BINDIR)/stroff
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BINDIR)/%.o)

# Default target
all: $(TARGET)

# Create directories if they don't exist
$(BINDIR):
	mkdir -p $(BINDIR)

# Compile object files
$(BINDIR)/%.o: $(SRCDIR)/%.c | $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link the final executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

# Build documentation
docs: $(TARGET) MANUAL.STR MANUAL_ENG.STR
	./$(TARGET) MANUAL.STR MANUAL.TXT
	./$(TARGET) MANUAL_ENG.STR MANUAL_ENG.TXT
	@echo "Documentation generated: MANUAL.TXT, MANUAL_ENG.TXT"

# Clean build artifacts
clean:
	rm -f $(BINDIR)/*.o $(TARGET)
	rm -f *.tmp

# Clean everything including generated docs
distclean: clean
	rm -f MANUAL.TXT MANUAL_ENG.TXT

# Install (copy to /usr/local/bin)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/stroff
	@echo "STROFF installed to /usr/local/bin/stroff"

# Uninstall
uninstall:
	rm -f /usr/local/bin/stroff
	@echo "STROFF uninstalled"

# Test with example document
test: $(TARGET)
	@echo "Running basic functionality test..."
	@echo '.TITLE "Test Document"' > test.trf
	@echo '.PAGEWIDTH 60' >> test.trf
	@echo '.DOCUMENT' >> test.trf
	@echo '.P' >> test.trf
	@echo 'This is a test paragraph.' >> test.trf
	@echo '.EDOC' >> test.trf
	./$(TARGET) test.trf test.txt
	@echo "Test completed. Check test.txt for output."
	@rm -f test.trf test.txt

# Development help
help:
	@echo "STROFF Makefile - Available targets:"
	@echo ""
	@echo "  all        - Build the STROFF executable (default)"
	@echo "  docs       - Generate documentation (MANUAL.TXT, MANUAL_ENG.TXT)"
	@echo "  clean      - Remove build artifacts"
	@echo "  distclean  - Remove build artifacts and generated docs"
	@echo "  install    - Install STROFF to /usr/local/bin"
	@echo "  uninstall  - Remove STROFF from /usr/local/bin"
	@echo "  test       - Run basic functionality test"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make              # Build STROFF"
	@echo "  make docs         # Build STROFF and generate documentation"
	@echo "  make clean        # Clean build files"
	@echo "  make install      # Install system-wide (requires sudo)"
	@echo ""

# Phony targets
.PHONY: all docs clean distclean install uninstall test help

# Debug information
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)
	@echo "Debug version built with debugging symbols"
