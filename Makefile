CC=gcc
CFLAGS = -Wall -D 

APP = all

all:
	$(CC) $(CFLAGS) -o $(APP) $(APP).c

clean:
	$(RM) $(APP) 
