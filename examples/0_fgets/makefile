# c-embed build system

# data directory to embed
DAT = data

# c-embed configuration clags
CEF = -include /usr/local/include/c-embed.h -DCEMBED_TRANSLATE

# build rules
.PHONY: embedded relative

embedded: CF = $(shell c-embed $(DAT)) $(CEF)
embedded: all

relative: CF =
relative: all

build:
	gcc main.c $(CF) -o main

all: build
