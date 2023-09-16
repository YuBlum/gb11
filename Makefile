bin=gb11
flags=-Wall -Wextra -Werror -ansi -pedantic
libs=-lglfw -lGL -lm
target=linux

ifeq ($(target), linux)
	cc=cc
	defs=-DLINUX
else ifeq ($(target), windows)
	cc=x86_64-w64-mingw32-gcc
	defs=-Dwindows
else
$(error unknown build target)
endif

out=build/$(bin)_$(target)

.PHONY: clean all

all:
	$(cc) $(flags) -o $(out) $(libs) $(defs) main.c

clean:
	rm -rf ./build/*
