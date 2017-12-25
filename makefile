CC=gcc 
CFLAGS=-c -Wall -march=x86-64 -pipe -Os
LDFLAGS=
SOURCES=dse.c rijndael-alg-fst.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dse
all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm *o $(EXECUTABLE)