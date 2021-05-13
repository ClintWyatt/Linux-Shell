cc=gcc
shell=Shell.c

all:
	$(cc) $(shell) -o subShell

clean:
	rm subShell
