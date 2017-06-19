# ESP8266 Library

CSRCS = $(wildcard ./*.c)
COBJS = $(CSRCS:.c=.o)

CC=gcc
AR=ar

CFLAGS =  -I. -Wall -g -O2 
LDFLAGS = 

all: libmqtt.a

libmqtt.a: $(COBJS)
	$(AR) rv $@ $?

.PHONY: clean 

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f *.o *.exe *~ *.a

