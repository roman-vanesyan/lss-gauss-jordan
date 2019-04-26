all:
	gcc main.c lss.c -std=c11 -Wall -O3 -o lss

.PHONY: all
.DEFAULT_GOAL := all
