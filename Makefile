CC=gcc
CFLAGS=-Wall
LINK=gcc

SRC_FOLDER=src
BIN_FOLDER=bin
EXE=exe

SOURCES=$(shell find $(SRC_FOLDER) -type f -iname '*.c')
HEADERS=$(shell find $(SRC_FOLDER) -type f -iname '*.h')
OBJECTS=$(subst $(SRC_FOLDER),$(BIN_FOLDER),$(subst .c,.o,$(SOURCES)))

$(BIN_FOLDER)/%.o : $(SRC_FOLDER)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(OBJECTS) $(HEADERS)
	$(LINK) $^ -o $@


.phony: run
run: $(EXE)
	./$(EXE)


.phony: clobber
clobber:
	rm -f $(BIN_FOLDER)/*.o
	rm -f $(EXE)


.phony: test
test: 
	@echo $(SOURCES)
	@echo $(HEADERS)
	@echo $(OBJECTS)


#print number of lines
.phony: line_count
line_count:
	find . -name '*.c' -o -name '*.h' | xargs wc -l

