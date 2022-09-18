# c-embed
# embed virtual file systems into an c program
# - at build time
# - with zero dependencies
# - with zero code modifications
# - with zero clutter in your program
# author: nicholas mcdonald 2022

INSTALL_DIR = /usr/local/bin
INCLUDE_DIR = /usr/local/include

build:
	gcc -g c-embed.c -o c-embed

install:
	mv c-embed $(INSTALL_DIR)/c-embed
	cp c-embed.h $(INCLUDE_DIR)/c-embed.h

all: build install
