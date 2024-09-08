all: ttypx

ttypx: ttypx.c

PREFIX = $(HOME)/.local/bin

install: ttypx
	install -m 0755 ttypx $(PREFIX)/
