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

all: halt matmult sort testfiles lock_function_test1 lock_function_test2 condition_function_test serverbroadcast_function_test monitor_function_test1 monitor_function_test2 monitor_function_test3 multisort multimat client1 client2 lock1 lock2 monitor1 monitor2

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

multisort.o: multisort.c
	$(CC) $(CFLAGS) -c multisort.c
multisort: multisort.o start.o
	$(LD) $(LDFLAGS) start.o multisort.o -o multisort.coff
	../bin/coff2noff multisort.coff multisort

multimat.o: multimat.c
	$(CC) $(CFLAGS) -c multimat.c
multimat: multimat.o start.o
	$(LD) $(LDFLAGS) start.o multimat.o -o multimat.coff
	../bin/coff2noff multimat.coff multimat

lock_function_test1.o: lock_function_test1.c
	$(CC) $(CFLAGS) -c lock_function_test1.c
lock_function_test1: lock_function_test1.o start.o
	$(LD) $(LDFLAGS) start.o lock_function_test1.o -o lock_function_test1.coff
	../bin/coff2noff lock_function_test1.coff lock_function_test1

lock_function_test2.o: lock_function_test2.c
	$(CC) $(CFLAGS) -c lock_function_test2.c
lock_function_test2: lock_function_test2.o start.o
	$(LD) $(LDFLAGS) start.o lock_function_test2.o -o lock_function_test2.coff
	../bin/coff2noff lock_function_test2.coff lock_function_test2

condition_function_test.o: condition_function_test.c
	$(CC) $(CFLAGS) -c condition_function_test.c
condition_function_test: condition_function_test.o start.o
	$(LD) $(LDFLAGS) start.o condition_function_test.o -o condition_function_test.coff
	../bin/coff2noff condition_function_test.coff condition_function_test

serverbroadcast_function_test.o: serverbroadcast_function_test.c
	$(CC) $(CFLAGS) -c serverbroadcast_function_test.c
serverbroadcast_function_test: serverbroadcast_function_test.o start.o
	$(LD) $(LDFLAGS) start.o serverbroadcast_function_test.o -o serverbroadcast_function_test.coff
	../bin/coff2noff serverbroadcast_function_test.coff serverbroadcast_function_test

monitor_function_test1.o: monitor_function_test1.c
	$(CC) $(CFLAGS) -c monitor_function_test1.c
monitor_function_test1: monitor_function_test1.o start.o
	$(LD) $(LDFLAGS) start.o monitor_function_test1.o -o monitor_function_test1.coff
	../bin/coff2noff monitor_function_test1.coff monitor_function_test1

monitor_function_test2.o: monitor_function_test2.c
	$(CC) $(CFLAGS) -c monitor_function_test2.c
monitor_function_test2: monitor_function_test2.o start.o
	$(LD) $(LDFLAGS) start.o monitor_function_test2.o -o monitor_function_test2.coff
	../bin/coff2noff monitor_function_test2.coff monitor_function_test2

monitor_function_test3.o: monitor_function_test3.c
	$(CC) $(CFLAGS) -c monitor_function_test3.c
monitor_function_test3: monitor_function_test3.o start.o
	$(LD) $(LDFLAGS) start.o monitor_function_test3.o -o monitor_function_test3.coff
	../bin/coff2noff monitor_function_test3.coff monitor_function_test3

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

clean:
	rm -f *.o *.coff
