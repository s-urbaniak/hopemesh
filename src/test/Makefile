CC=gcc

CFLAGS=-Wall -Wno-unused-but-set-variable
CFLAGS+=-std=gnu99 -I. -g
CFLAGS+=-DLC_INCLUDE='"../lc-addrlabels.h"'
CFLAGS+=-DF_CPU=16000000UL

LIBS=
LDFLAGS=

CFLAGS_CURSES=$(shell ncurses5-config --cflags)
LIBS_CURSES=-lpanel $(shell ncurses5-config --libs)

DEPS=\
	avr-config.o avr-rfm12.o avr-batman.o avr-mac.o avr-l4.o \
    avr-llc.o avr-debug.o avr-hamming.o avr-ringbuf.o avr-timer.o \
    avr-shell.o avr-clock.o avr-uart.o avr-util.o avr-rxthread.o \
    test-util.o mock-watchdog.o mock-crc16.o mock-io.o mock-interrupt.o mock-spi.o

all: test-shell

test-shell: CFLAGS+=-DUSE_NCURSES=1
test-shell: CFLAGS+=$(CFLAGS_CURSES)
test-shell: LIBS+=$(LIBS_CURSES)
test-shell: LDFLAGS+=-Wl,--wrap=llc_rx
test-shell: test-shell.o $(DEPS) intercept-llc.o

test-batman: LDFLAGS+=-Wl,--wrap=llc_rx
test-batman: LDFLAGS+=-Wl,--wrap=llc_tx

test-%: test-%.o $(DEPS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

avr-%.o: ../%.c
	$(CC) -o $@ -c $< $(CFLAGS)

%.deps : %.c
	$(CC) -M $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $< $(CFLAGS)

avr-%-prep.c: ../%.c
	$(CC) -E -o $@ -c $< $(CFLAGS)

clean:
	rm -f *.o
	rm -f *.deps
	find . -executable -name 'test*' -exec rm '{}' ';'

.PHONY : clean
