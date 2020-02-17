CC=gcc
OBJS=regex.o test-regex.o
EXEC=test

all : $(OBJS)
	$(CC) -o $(EXEC) $(OBJS)

clean :
	rm -f *.o $(EXEC)

%.o : %.c
	$(CC) -c -o $@ $+
