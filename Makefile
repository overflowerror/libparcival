CC       = gcc
CFLAGS   = -std=c99 -Wall -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE=500 -D_GNU_SOURCE -g -Isrc/
LD       = gcc
LDFLAGS  = 
AR       = ar
ARFLAGS  = rcs

LEX = flex
YACC = bison
YFLAGS = -y -d

LIB_OBJS = obj/common.o obj/templates.o

BIN = parcival
A_LIB = libparcival.a

all: $(BIN) $(A_LIB)

$(BIN): gen/y.tab.c gen/lex.yy.c src/main.c src/tree.c src/common.c
	$(CC) $(CFLAGS) -o $@ $^

$(A_LIB): CFLAGS += -fPIC
$(A_LIB): $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $^

obj/%.o: src/%.c obj
	$(CC) $(CFLAGS) -c -o $@ $<

obj:
	mkdir -p $@

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
	rm -f obj/*
	rm -f $(BIN)
	rm -f $(A_LIB)
