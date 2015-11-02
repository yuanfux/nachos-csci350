#ifndef IPT_H
#define IPT_H

#include "copyright.h"
#include "utility.h"

#include "addrspace.h"

class IPT {
public:
	int virtualPage;

	bool valid;
	bool use;
	bool dirty;

	AddrSpace *space;
	int spaceID;
};

#endif
