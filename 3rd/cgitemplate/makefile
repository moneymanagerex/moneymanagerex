#-----------------------------------------------------------------------
# config
#-----------------------------------------------------------------------

# name of the target executable
EXE := test
# list of files tp delete when cleaning
TO_CLEAN := *.o $(EXE)
# override compiler flags to give us no break
CFLAGS := -Wall -ansi -pedantic $(DEBUG)
# compiler
CC := g++

#-----------------------------------------------------------------------
# main
#-----------------------------------------------------------------------

# create the list of *.o files based on auto dependencies 
OBJECTS := $(shell $(CC) -MM $(INCLUDE) test.cpp html_template.cpp | awk -F": " '{ if(NF<2){next;} ;ORS=" "; print $$1}')
#-----------------------------------------------------------------------

all: $(EXE) 

# rule to create an object file
%.o: %.cpp %.h
	$(CC) $(CFLAGS) $(INCLUDE) $(LIB) -c $<

# create the executable
$(EXE): $(OBJECTS) 
	$(CC) $(CFLAGS) $^ -o $@ 

#-----------------------------------------------------------------------

clean:
	-rm $(TO_CLEAN)

#-----------------------------------------------------------------------

.PHONY: debug
debug:
	$(MAKE) clean
	$(MAKE) DEBUG="-g -DDEBUG -DDEBUG2 -DDEBUG3"

