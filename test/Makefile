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

all: halt matmult sort testfiles exectest forktest releasetest releasetest2 waittest yieldtest signaltest multithread

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

exectest.o: exectest.c
	$(CC) $(CFLAGS) -c exectest.c
exectest: exectest.o start.o
	$(LD) $(LDFLAGS) start.o exectest.o -o exectest.coff
	../bin/coff2noff exectest.coff exectest

forktest.o: forktest.c
	$(CC) $(CFLAGS) -c forktest.c
forktest: forktest.o start.o
	$(LD) $(LDFLAGS) start.o forktest.o -o forktest.coff
	../bin/coff2noff forktest.coff forktest

waittest.o: waittest.c
	$(CC) $(CFLAGS) -c waittest.c
waittest: waittest.o start.o
	$(LD) $(LDFLAGS) start.o waittest.o -o waittest.coff
	../bin/coff2noff waittest.coff waittest

signaltest.o: signaltest.c
	$(CC) $(CFLAGS) -c signaltest.c
signaltest: signaltest.o start.o
	$(LD) $(LDFLAGS) start.o signaltest.o -o signaltest.coff
	../bin/coff2noff signaltest.coff signaltest

yieldtest.o: yieldtest.c
	$(CC) $(CFLAGS) -c yieldtest.c
yieldtest: yieldtest.o start.o
	$(LD) $(LDFLAGS) start.o yieldtest.o -o yieldtest.coff
	../bin/coff2noff yieldtest.coff yieldtest

releasetest.o: releasetest.c
	$(CC) $(CFLAGS) -c releasetest.c
releasetest: releasetest.o start.o
	$(LD) $(LDFLAGS) start.o releasetest.o -o releasetest.coff
	../bin/coff2noff releasetest.coff releasetest

releasetest2.o: releasetest2.c
	$(CC) $(CFLAGS) -c releasetest2.c
releasetest2: releasetest2.o start.o
	$(LD) $(LDFLAGS) start.o releasetest2.o -o releasetest2.coff
	../bin/coff2noff releasetest2.coff releasetest2

multithread.o: multithread.c
	$(CC) $(CFLAGS) -c multithread.c
multithread: multithread.o start.o
	$(LD) $(LDFLAGS) start.o multithread.o -o multithread.coff
	../bin/coff2noff multithread.coff multithread

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

clean:
	rm -f *.o *.coff
