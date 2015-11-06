#ifndef PAGETABLE_H
#define PAGETABLE_H

#include "copyright.h"
#include "utility.h"

enum Location{EXECUTABLE, DISK, SWAPFILE, MEMORY};

class PageTable: public TranslationEntry {
public:

	int byteOffset;
	Location location;
};

#endif
