#================================
# Splash Execution Mode Makefile
#================================

#Compiler Settings
CC = g++ -std=c++11
DAT = data           #data directory to embed

CIF = -include /usr/local/include/c-embed.h
.PHONY: embedded relative

embedded: CF = $(shell c-embed $(DAT)) c-embed.o -DCEMBED_TRANSLATE
embedded: all

relative: CF =
relative: CIF =
relative: all

build:
	$(CC) main.cpp -o main ${CF} $(CIF)

all: build
