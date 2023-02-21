CC = gcc

CFLAGS += -Wall -O3

DFLAGS +=

CDFLAGS += ${CFLAGS} ${DFLAGS} ${INCLUDE}

LIBS += -lpthread

LDFLAGS += ${LIBS}

TARGET1 = main

OBJECT1 = \
		${TARGET1:=.o} \
		cli.o \
		term.o \
		thread.o \
		mutex.o


TARGETS += ${TARGET1}

.PHONY: all clean

all: ${TARGETS}

${TARGET1}: ${OBJECT1}
	${CC} -o $@ ${OBJECT1} ${LDFLAGS}

.c.o:
	@${CC} ${CDFLAGS} -c $<

clean:
	@rm -rf *.o ${TARGETS}