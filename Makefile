all: main.c
	cc -o build/gol main.c -lraylib -lm