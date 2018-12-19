#ifndef _COSC_H_
#define _COSC_H_

#include <inttypes.h>

#define MAX_CORE_PER_COS (1024)
#define MAX_COSC (1024)

struct Cosc{
	unsigned int cos_id;
	uint64_t mask;
	int core_nb;
	unsigned int core_list[MAX_CORE_PER_COS];
}; 
typedef struct Cosc Cosc;

#endif

