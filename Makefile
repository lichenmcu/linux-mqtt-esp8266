# ESP8266 Library

TARGET = mqtt_main.exe

ASRCS =
CSRCS := $(TARGET:.exe=.c)
CSRCS +=  MQTTClient.c MQTTLinux.c netutils/esp8266.c

AOBJS = $(ASRCS:.S=.o)
COBJS = $(CSRCS:.c=.o)

SRCS = $(ASRCS) $(CSRCS)
OBJS = $(AOBJS) $(COBJS)

CC=gcc
CFLAGS =  -I. -I./netutils -I./mqtt -Wall -g -O2
#CFLAGS += -D_DEBUG

LDFLAGS = -lpthread 
LIBS   += mqtt/libmqtt.a

all: $(TARGET)

$(TARGET) : $(AOBJS) $(COBJS)
	make -C mqtt
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

.PHONY: clean 

$(AOBJS): %.o: %.S
	$(CC) $(CFLAGS) -c $^ -o $@

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f *.o *.exe *.a *~ netutils/*.o mqtt/*.o

