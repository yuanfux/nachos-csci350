# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = ../gnu/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: client1 client2 lock1 lock2 monitor1 monitor2 ApplicationClerk Cashier Customer Manager Senator PassportClerk PictureClerk 1Manager 5ApplicationClerk 5Cashier 5PassportClerk 5PictureClerk 10Senator 50Customer

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

client1.o: client1.c
	$(CC) $(CFLAGS) -c client1.c
client1: client1.o start.o
	$(LD) $(LDFLAGS) start.o client1.o -o client1.coff
	../bin/coff2noff client1.coff client1

client2.o: client2.c
	$(CC) $(CFLAGS) -c client2.c
client2: client2.o start.o
	$(LD) $(LDFLAGS) start.o client2.o -o client2.coff
	../bin/coff2noff client2.coff client2

lock1.o: lock1.c
	$(CC) $(CFLAGS) -c lock1.c
lock1: lock1.o start.o
	$(LD) $(LDFLAGS) start.o lock1.o -o lock1.coff
	../bin/coff2noff lock1.coff lock1

lock2.o: lock2.c
	$(CC) $(CFLAGS) -c lock2.c
lock2: lock2.o start.o
	$(LD) $(LDFLAGS) start.o lock2.o -o lock2.coff
	../bin/coff2noff lock2.coff lock2

monitor1.o: monitor1.c
	$(CC) $(CFLAGS) -c monitor1.c
monitor1: monitor1.o start.o
	$(LD) $(LDFLAGS) start.o monitor1.o -o monitor1.coff
	../bin/coff2noff monitor1.coff monitor1

monitor2.o: monitor2.c
	$(CC) $(CFLAGS) -c monitor2.c
monitor2: monitor2.o start.o
	$(LD) $(LDFLAGS) start.o monitor2.o -o monitor2.coff
	../bin/coff2noff monitor2.coff monitor2

ApplicationClerk.o: ApplicationClerk.c
	$(CC) $(CFLAGS) -c ApplicationClerk.c
ApplicationClerk: ApplicationClerk.o start.o
	$(LD) $(LDFLAGS) start.o ApplicationClerk.o -o ApplicationClerk.coff
	../bin/coff2noff ApplicationClerk.coff ApplicationClerk

5ApplicationClerk.o: 5ApplicationClerk.c
	$(CC) $(CFLAGS) -c 5ApplicationClerk.c
5ApplicationClerk: 5ApplicationClerk.o start.o
	$(LD) $(LDFLAGS) start.o 5ApplicationClerk.o -o 5ApplicationClerk.coff
	../bin/coff2noff 5ApplicationClerk.coff 5ApplicationClerk

Cashier.o: Cashier.c
	$(CC) $(CFLAGS) -c Cashier.c
Cashier: Cashier.o start.o
	$(LD) $(LDFLAGS) start.o Cashier.o -o Cashier.coff
	../bin/coff2noff Cashier.coff Cashier

5Cashier.o: 5Cashier.c
	$(CC) $(CFLAGS) -c 5Cashier.c
5Cashier: 5Cashier.o start.o
	$(LD) $(LDFLAGS) start.o 5Cashier.o -o 5Cashier.coff
	../bin/coff2noff 5Cashier.coff 5Cashier

Customer.o: Customer.c
	$(CC) $(CFLAGS) -c Customer.c
Customer: Customer.o start.o
	$(LD) $(LDFLAGS) start.o Customer.o -o Customer.coff
	../bin/coff2noff Customer.coff Customer

50Customer.o: 50Customer.c
	$(CC) $(CFLAGS) -c 50Customer.c
50Customer: 50Customer.o start.o
	$(LD) $(LDFLAGS) start.o 50Customer.o -o 50Customer.coff
	../bin/coff2noff 50Customer.coff 50Customer

Manager.o: Manager.c
	$(CC) $(CFLAGS) -c Manager.c
Manager: Manager.o start.o
	$(LD) $(LDFLAGS) start.o Manager.o -o Manager.coff
	../bin/coff2noff Manager.coff Manager

1Manager.o: 1Manager.c
	$(CC) $(CFLAGS) -c 1Manager.c
1Manager: 1Manager.o start.o
	$(LD) $(LDFLAGS) start.o 1Manager.o -o 1Manager.coff
	../bin/coff2noff 1Manager.coff 1Manager

PassportClerk.o: PassportClerk.c
	$(CC) $(CFLAGS) -c PassportClerk.c
PassportClerk: PassportClerk.o start.o
	$(LD) $(LDFLAGS) start.o PassportClerk.o -o PassportClerk.coff
	../bin/coff2noff PassportClerk.coff PassportClerk

5PassportClerk.o: 5PassportClerk.c
	$(CC) $(CFLAGS) -c 5PassportClerk.c
5PassportClerk: 5PassportClerk.o start.o
	$(LD) $(LDFLAGS) start.o 5PassportClerk.o -o 5PassportClerk.coff
	../bin/coff2noff 5PassportClerk.coff 5PassportClerk

PictureClerk.o: PictureClerk.c
	$(CC) $(CFLAGS) -c PictureClerk.c
PictureClerk: PictureClerk.o start.o
	$(LD) $(LDFLAGS) start.o PictureClerk.o -o PictureClerk.coff
	../bin/coff2noff PictureClerk.coff PictureClerk

5PictureClerk.o: 5PictureClerk.c
	$(CC) $(CFLAGS) -c 5PictureClerk.c
5PictureClerk: 5PictureClerk.o start.o
	$(LD) $(LDFLAGS) start.o 5PictureClerk.o -o 5PictureClerk.coff
	../bin/coff2noff 5PictureClerk.coff 5PictureClerk

Senator.o: Senator.c
	$(CC) $(CFLAGS) -c Senator.c
Senator: Senator.o start.o
	$(LD) $(LDFLAGS) start.o Senator.o -o Senator.coff
	../bin/coff2noff Senator.coff Senator

10Senator.o: 10Senator.c
	$(CC) $(CFLAGS) -c 10Senator.c
10Senator: 10Senator.o start.o
	$(LD) $(LDFLAGS) start.o 10Senator.o -o 10Senator.coff
	../bin/coff2noff 10Senator.coff 10Senator
clean:
	rm -f *.o *.coff
