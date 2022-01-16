# cembed tool
# author: nicholas mcdonald 2022

INSTALL_DIR = /usr/local/bin
INCLUDE_DIR = /usr/local/include

build:
	gcc c-embed.c -o c-embed

install:
	mv c-embed $(INSTALL_DIR)/c-embed
	cp c-embed.h $(INCLUDE_DIR)/c-embed.h

all: build install
