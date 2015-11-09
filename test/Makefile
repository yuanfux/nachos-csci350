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

all: halt matmult sort testfiles exec_exittest forktest release_acquiretest yieldtest signal_waittest randomtest broadcasttest destroy_condition_locktest printinttest passportoffice multioffice TestSuit monitortest client lock_function_test lock_function_test2 condition_function_test broadcast_function_test

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

exec_exittest.o: exec_exittest.c
	$(CC) $(CFLAGS) -c exec_exittest.c
exec_exittest: exec_exittest.o start.o
	$(LD) $(LDFLAGS) start.o exec_exittest.o -o exec_exittest.coff
	../bin/coff2noff exec_exittest.coff exec_exittest

forktest.o: forktest.c
	$(CC) $(CFLAGS) -c forktest.c
forktest: forktest.o start.o
	$(LD) $(LDFLAGS) start.o forktest.o -o forktest.coff
	../bin/coff2noff forktest.coff forktest

signal_waittest.o: signal_waittest.c
	$(CC) $(CFLAGS) -c signal_waittest.c
signal_waittest: signal_waittest.o start.o
	$(LD) $(LDFLAGS) start.o signal_waittest.o -o signal_waittest.coff
	../bin/coff2noff signal_waittest.coff signal_waittest

yieldtest.o: yieldtest.c
	$(CC) $(CFLAGS) -c yieldtest.c
yieldtest: yieldtest.o start.o
	$(LD) $(LDFLAGS) start.o yieldtest.o -o yieldtest.coff
	../bin/coff2noff yieldtest.coff yieldtest

release_acquiretest.o: release_acquiretest.c
	$(CC) $(CFLAGS) -c release_acquiretest.c
release_acquiretest: release_acquiretest.o start.o
	$(LD) $(LDFLAGS) start.o release_acquiretest.o -o release_acquiretest.coff
	../bin/coff2noff release_acquiretest.coff release_acquiretest

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

destroy_condition_locktest.o: destroy_condition_locktest.c
	$(CC) $(CFLAGS) -c destroy_condition_locktest.c
destroy_condition_locktest: destroy_condition_locktest.o start.o
	$(LD) $(LDFLAGS) start.o destroy_condition_locktest.o -o destroy_condition_locktest.coff
	../bin/coff2noff destroy_condition_locktest.coff destroy_condition_locktest

printinttest.c.o: printinttest.c
	$(CC) $(CFLAGS) -c printinttest.c
printinttest: printinttest.o start.o
	$(LD) $(LDFLAGS) start.o printinttest.o -o printinttest.coff
	../bin/coff2noff printinttest.coff printinttest

passportoffice.o: passportoffice.c
	$(CC) $(CFLAGS) -c passportoffice.c
passportoffice: passportoffice.o start.o
	$(LD) $(LDFLAGS) start.o passportoffice.o -o passportoffice.coff
	../bin/coff2noff passportoffice.coff passportoffice

multioffice.o: multioffice.c
	$(CC) $(CFLAGS) -c multioffice.c
multioffice: multioffice.o start.o
	$(LD) $(LDFLAGS) start.o multioffice.o -o multioffice.coff
	../bin/coff2noff multioffice.coff multioffice

TestSuit.o: TestSuit.c
	$(CC) $(CFLAGS) -c TestSuit.c
TestSuit: TestSuit.o start.o
	$(LD) $(LDFLAGS) start.o TestSuit.o -o TestSuit.coff
	../bin/coff2noff TestSuit.coff TestSuit

monitortest.o: monitortest.c
	$(CC) $(CFLAGS) -c monitortest.c
monitortest: monitortest.o start.o
	$(LD) $(LDFLAGS) start.o monitortest.o -o monitortest.coff
	../bin/coff2noff monitortest.coff monitortest

client.o: client.c
	$(CC) $(CFLAGS) -c client.c
client: client.o start.o
	$(LD) $(LDFLAGS) start.o client.o -o client.coff
	../bin/coff2noff client.coff client

lock_function_test.o: lock_function_test.c
	$(CC) $(CFLAGS) -c lock_function_test.c
lock_function_test: lock_function_test.o start.o
	$(LD) $(LDFLAGS) start.o lock_function_test.o -o lock_function_test.coff
	../bin/coff2noff lock_function_test.coff lock_function_test

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

broadcast_function_test.o: broadcast_function_test.c
	$(CC) $(CFLAGS) -c broadcast_function_test.c
broadcast_function_test: broadcast_function_test.o start.o
	$(LD) $(LDFLAGS) start.o broadcast_function_test.o -o broadcast_function_test.coff
	../bin/coff2noff broadcast_function_test.coff broadcast_function_test

clean:
	rm -f *.o *.coff
