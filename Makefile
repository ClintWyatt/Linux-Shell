cc=gcc
shell=Shell.c
history=history.c
signaling=signalHandler.c
redirection=redirection.c
simple=simpleCommand.c
directory=directoryChange.c
pipe=pipeLine.c
all:
	$(cc) $(simple) $(directory) $(history) $(signaling) $(redirection) $(pipe) $(shell) -o subShell

clean:
	rm subShell
