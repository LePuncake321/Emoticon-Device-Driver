ROOTNAME= EmoticonCypher_SampleProgram
RUNOPTIONS=
CC=gcc
CFLAGS= -g -I.
LIBS =-l pthread
DEPS = 
OBJ = $(ROOTNAME).o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) 

$(ROOTNAME)$(HW): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm *.o $(ROOTNAME)$(HW)

run: $(ROOTNAME)
	./$(ROOTNAME) $(RUNOPTIONS)

vrun: $(ROOTNAME)
	valgrind ./$(ROOTNAME) $(RUNOPTIONS)