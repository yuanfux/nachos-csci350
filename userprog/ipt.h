#ifndef IPT_H
#define IPT_H

#include "copyright.h"
#include "utility.h"

#include "addrspace.h"

class IPT: public TranslationEntry{
public:

	AddrSpace *space;
	int spaceID;
};

#endif
