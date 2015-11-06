// addrspace.cc
//  Routines to manage address spaces (executing user programs).
//
//  In order to run a user program, you must:
//
//  1. link with the -N -T 0 option
//  2. run coff2noff to convert the object file to Nachos format
//      (Nachos object code format is essentially just a simpler
//      version of the UNIX executable object code format)
//  3. load the NOFF file into the Nachos file system
//      (if you haven't implemented the file system yet, you
//      don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
#include "synch.h"

extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s), numElements(0) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
        delete table;
        table = 0;
    }
    if (lock) {
        delete lock;
        lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >= 0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;  // to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1) {
        numElements++;
        table[i] = f;

    }
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f = 0;

    if ( i >= 0 && i < size ) {
        lock->Acquire();
        if ( map.Test(i) ) {
            map.Clear(i);
            f = table[i];
            table[i] = 0;
            numElements--;
        }
        lock->Release();
    }
    return f;
}

int Table::GetNumElements() {
    return numElements;
}

//----------------------------------------------------------------------
// SwapHeader
//  Do little endian to big endian conversion on the bytes in the
//  object file header, in case the file was generated on a little
//  endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//  Create an address space to run a user program.
//  Load the program from a file "executable", and set everything
//  up so that we can start executing user instructions.
//
//  Assumes that the object code file is in NOFF format.
//
//  "executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles) {
    NoffHeader noffH;
    unsigned int i, size, physicalPage;
    //initialize number of threads in a process
    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);
    lock = new Lock("lock");
    privateExecutable = executable;
    privateExecutable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
            (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);

    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
    numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize, PageSize);
    // we need to increase the size
    // to leave room for the stack
    DEBUG('a', "pageSize: %d, numPages: %d, NumPhysPages: %d, size: %d\n", PageSize, numPages, NumPhysPages, size);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);       // check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
          numPages, size);
    numThread = 0;

    unsigned int inExec = divRoundUp(noffH.code.size + noffH.initData.size, PageSize);
    pageTable = new PageTable[numPages];

    for (i = 0; i < numPages; i++) {
        // lock->Acquire();
        // physicalPage = memoryMap.Find();
        // lock->Release();
        pageTable[i].virtualPage = i;
        // pageTable[i].physicalPage = physicalPage;
        pageTable[i].valid = FALSE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;  // if the code segment was entirely on
        // a separate page, we could set its
        // pages to be read-only
        if (i < inExec) {
            pageTable[i].byteOffset = noffH.code.inFileAddr + pageTable[i].virtualPage * PageSize;
            pageTable[i].location = EXECUTABLE; 
        }
        else {
            pageTable[i].byteOffset = -1;
            pageTable[i].location = DISK;
        }
        // executable->ReadAt(&(machine->mainMemory[pageTable[i].physicalPage * PageSize]),
        // PageSize, noffH.code.inFileAddr + pageTable[i].virtualPage * PageSize);

        // ipt[physicalPage].virtualPage = i;
        // ipt[physicalPage].valid = TRUE;
        // ipt[physicalPage].space = this;
    }
    printf("addr constructor byteOffset: %d\n", pageTable[0].byteOffset);

    DEBUG('a', "Finish address space initialization\n");
// zero out the entire address space, to zero the unitialized data segment
// and the stack segment
    //bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory
    // if (noffH.code.size > 0) {
    //     DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",
    //           noffH.code.virtualAddr, noffH.code.size);
    //     executable->ReadAt(&(machine->mainMemory[pageTable[0].physicalPage*PageSize]),
    //                        noffH.code.size, noffH.code.inFileAddr);
    // }
    // if (noffH.initData.size > 0) {
    //     DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",
    //           noffH.initData.virtualAddr, noffH.initData.size);
    //     executable->ReadAt(&(machine->mainMemory[pageTable[0].physicalPage * PageSize + noffH.code.size]),
    //                        noffH.initData.size, noffH.initData.inFileAddr);
    // }

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
//  Dealloate an address space.  release pages, page tables, ts
//  and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
    delete privateExecutable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//  Set the initial values for the user-level register set.
//
//  We write these directly into the "machine" registers, so
//  that we can immediately jump to user code.  Note that these
//  will be saved/restored into the currentThread->userRegisters
//  when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);

}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//  On a context switch, save any machine state, specific
//  to this address space, that needs saving.
//
//  For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
#ifdef USE_TLB
    for (int i = 0; i < TLBSize; i++)
    {
        machine->tlb[i].valid = FALSE;
    }
#endif
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//  On a context switch, restore the machine state so that
//  this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
#ifdef USE_TLB
    machine->pageTable = NULL;
    for (int i = 0; i < TLBSize; i++)
    {
        machine->tlb[i].valid = FALSE;
    }
#endif
}

void AddrSpace::AllocateSpaceForNewThread() {

    numPages += 8;
    PageTable *newPageTable = new PageTable[numPages];
    int physicalPage;

    for (unsigned int i = 0; i < numPages - 8; i++) {
        newPageTable[i].virtualPage = pageTable[i].virtualPage;
        newPageTable[i].physicalPage = pageTable[i].physicalPage;
        newPageTable[i].valid = pageTable[i].valid;
        newPageTable[i].use = pageTable[i].use;
        newPageTable[i].dirty = pageTable[i].dirty;
        newPageTable[i].readOnly = pageTable[i].readOnly;
        newPageTable[i].location = pageTable[i].location;
        newPageTable[i].byteOffset = pageTable[i].byteOffset;

        physicalPage = pageTable[i].physicalPage;
        if (physicalPage > -1) {
            ipt[physicalPage].virtualPage = pageTable[i].virtualPage;
            ipt[physicalPage].valid = pageTable[i].valid;
            ipt[physicalPage].dirty = pageTable[i].dirty;
            ipt[physicalPage].space = this;
        }
    }

    for (unsigned int i = numPages - 8; i < numPages; i++) {
        // lock->Acquire();
        // physicalPage = memoryMap.Find();
        // lock->Release();
        newPageTable[i].virtualPage = i;
        // newPageTable[i].physicalPage = physicalPage;
        newPageTable[i].valid = FALSE;
        newPageTable[i].use = FALSE;
        newPageTable[i].dirty = FALSE;
        newPageTable[i].readOnly = FALSE;
        newPageTable[i].location = DISK;

        // ipt[physicalPage].virtualPage = i;
        // ipt[physicalPage].valid = TRUE;
        // ipt[physicalPage].space = this;
    }

    delete [] pageTable;

    pageTable = newPageTable;

    numThread++;
    currentThread->space->RestoreState();

}

void AddrSpace::DeallocateSpaceForThread() {

    // numPages -= 8;

    // TranslationEntry *newPageTable = new TranslationEntry[0];

    // for (unsigned int i = 0; i < numPages; i++) {
    //     newPageTable[i] = pageTable[i];
    // }

    delete pageTable;

    // pageTable = newPageTable;

    // numThread--;

}

int AddrSpace::GetNumThread() {

    return numThread;

}

int AddrSpace::GetSpaceID() {

    return spaceID;

}

void AddrSpace::SetSpaceID(int spaceid) {

    spaceID = spaceid;

}

int AddrSpace::GetMemorySize() {
    return numPages * PageSize;
}

void AddrSpace::UpdateThreadNum() {
    numThread--;
}

PageTable* AddrSpace::GetPageTable() {
    return pageTable;
}

Lock *AddrSpace::GetLock() {
    return lock;
}

OpenFile *AddrSpace::GetExecutable() {
    return privateExecutable;
}

int AddrSpace::AllocatePhysicalPage(){

    int physicalPage;

    lock->Acquire();
    physicalPage = memoryMap.Find();
    lock->Release();
    return physicalPage;
}

void AddrSpace::PopulateIPT(int vpn, int physicalPage) {

    if (pageTable[vpn].location == EXECUTABLE) {
        printf("before readat\n");
        printf("byteOffset: %d, vpn: %d\n", pageTable[vpn].byteOffset, vpn);
        currentThread->space->GetExecutable()->ReadAt(&(machine->mainMemory[physicalPage * PageSize]),
                PageSize, pageTable[vpn].byteOffset);
        printf("after readat\n");
    }

    pageTable[vpn].location = MEMORY;
    pageTable[vpn].valid = TRUE;
    pageTable[vpn].physicalPage = physicalPage;

    ipt[physicalPage].valid = pageTable[vpn].valid;
    ipt[physicalPage].dirty = pageTable[vpn].dirty;
    ipt[physicalPage].virtualPage = vpn;
    ipt[physicalPage].space = this;
}