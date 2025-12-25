.PHONY: all clean build

# Detect OS and set platform/extension accordingly
ifeq ($(UNAME_S),)
  UNAME_S := $(shell uname -s)
endif

ifeq ($(UNAME_S),Darwin)
  PLATFORM := macos
  DLLEXT := dylib
else
  PLATFORM := linux-amd64
  DLLEXT := so
endif

# Directories and files
SOURCE := ./lib/extension.c
BUILD_DIR := ./native/$(PLATFORM)
TARGET := $(BUILD_DIR)/extension.$(DLLEXT)
CC = clang

# Compiler flags
CFLAGS := -fPIC -shared
INCLUDES := -isystem ./include -isystem ./ -isystem ./include/mruby -I ./nativefiledialog-extended/src/include -L ./nativefiledialog-extended/build ./nativefiledialog-extended/build/src/libnfd.a -framework Cocoa -framework UniformTypeIdentifiers


all: clean build

watch:
	fswatch -ext cpp,c,h make -f Makefile

build: $(SOURCE)
	mkdir -p $(BUILD_DIR)
	$(CC) $(INCLUDES) $(CFLAGS) $(SOURCE) -o $(TARGET)

clean:
	rm -rf mygame/native/
