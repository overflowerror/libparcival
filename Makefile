CC       = gcc
CFLAGS   = -std=c99 -Wall -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -g -Isrc/
LD       = gcc
LDFLAGS  = 
AR       = ar
ARFLAGS  = rcs

LEX = flex
YACC = bison
YFLAGS = -y -d

BIN = bin

all: $(BIN)

$(BIN): gen/y.tab.c gen/lex.yy.c src/main.c src/tree.c src/common.c
	$(CC) $(CFLAGS) -o $@ $^


gen: 
	mkdir -p $@
	
gen/y.tab.c gen/y.tab.h: src/parser.y gen
	$(YACC) $(YFLAGS) $<
	mv y.tab.c y.tab.h gen/
	
gen/lex.yy.c: src/scanner.l gen/y.tab.h gen
	$(LEX) $<
	mv lex.yy.c gen/
	
	
clean:
	rm -f gen/*
