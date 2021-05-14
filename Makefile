cc=gcc
shell=Shell.c
history=history.c
signaling=signalHandler.c
redirection=redirection.c
simple=simpleCommand.c
all:
	$(cc) $(simple) $(history) $(signaling) $(redirection) $(shell) -o subShell

clean:
	rm subShell
