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

all: halt matmult sort testfiles exectest forktest releaseacquiretest yieldtest signalwaittest randomtest broadcasttest destroyconditionlocktest printinttest threadtest

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

signalwaittest.o: signalwaittest.c
	$(CC) $(CFLAGS) -c signalwaittest.c
signalwaittest: signalwaittest.o start.o
	$(LD) $(LDFLAGS) start.o signalwaittest.o -o signalwaittest.coff
	../bin/coff2noff signalwaittest.coff signalwaittest

yieldtest.o: yieldtest.c
	$(CC) $(CFLAGS) -c yieldtest.c
yieldtest: yieldtest.o start.o
	$(LD) $(LDFLAGS) start.o yieldtest.o -o yieldtest.coff
	../bin/coff2noff yieldtest.coff yieldtest

releaseacquiretest.o: releaseacquiretest.c
	$(CC) $(CFLAGS) -c releaseacquiretest.c
releaseacquiretest: releaseacquiretest.o start.o
	$(LD) $(LDFLAGS) start.o releaseacquiretest.o -o releaseacquiretest.coff
	../bin/coff2noff releaseacquiretest.coff releaseacquiretest

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

randomtest.o: randomtest.c
	$(CC) $(CFLAGS) -c randomtest.c
randomtest: randomtest.o start.o
	$(LD) $(LDFLAGS) start.o randomtest.o -o randomtest.coff
	../bin/coff2noff randomtest.coff randomtest

broadcasttest.o: broadcasttest.c
	$(CC) $(CFLAGS) -c broadcasttest.c
broadcasttest: broadcasttest.o start.o
	$(LD) $(LDFLAGS) start.o broadcasttest.o -o broadcasttest.coff
	../bin/coff2noff broadcasttest.coff broadcasttest

destroyconditionlocktest.o: destroyconditionlocktest.c
	$(CC) $(CFLAGS) -c destroyconditionlocktest.c
destroyconditionlocktest: destroyconditionlocktest.o start.o
	$(LD) $(LDFLAGS) start.o destroyconditionlocktest.o -o destroyconditionlocktest.coff
	../bin/coff2noff destroyconditionlocktest.coff destroyconditionlocktest

printinttest.c.o: printinttest.c
	$(CC) $(CFLAGS) -c printinttest.c
printinttest: printinttest.o start.o
	$(LD) $(LDFLAGS) start.o printinttest.o -o printinttest.coff
	../bin/coff2noff printinttest.coff printinttest

threadtest.o: threadtest.c
	$(CC) $(CFLAGS) -c threadtest.c
threadtest: threadtest.o start.o
	$(LD) $(LDFLAGS) start.o threadtest.o -o threadtest.coff
	../bin/coff2noff threadtest.coff threadtest



clean:
	rm -f *.o *.coff
