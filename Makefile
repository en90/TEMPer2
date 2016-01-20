HOSTCC:=$(CC)
CC:=$(CROSS_COMPILE)$(HOSTCC)
CFLAGS:=-std=c11 -Wall -O2 $(CFLAGS)

HOSTLD:=$(LD)
LD:=$(CROSS_COMPILE)$(HOSTLD)
LDFLAGS:=-L extra/lib

all:	getdegs

%.o:	%.c
	$(CC) -c $(CFLAGS) -o $@ $<

getdegs:	getdegs.o
	$(CC) $(LDFLAGS) -o $@ $^ -lusb

clean:		
	rm -f getdegs *.o

rules-install:			# must be superuser to do this
	cp 99-TEMPer2.rules /etc/udev/rules.d
