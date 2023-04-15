OBJS	= myGit.o
SOURCE	= myGit.c
HEADER	= 
OUT	= myGit
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

myGit.o: myGit.c
	$(CC) $(FLAGS) myGit.c 


clean:
	rm -f $(OBJS) $(OUT)