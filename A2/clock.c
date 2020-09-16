#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int clock_index;


/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
	// Note: as always the frame are used up one by one
	// thus when it invokes evict, the handle will start at 0
	
	// Frist find the clock_index of evict canadiate 
	// 1. find the next frame that has a ref status of 0
	while(coremap[clock_index].pte->frame & PG_REF) {
		// 2. if the curr frame is 1, we will need to set its ref to 0 as 
		// result of update ref----> this can implemented the second chance algorithm
		coremap[clock_index].pte->frame = coremap[clock_index].pte->frame &~ PG_REF;
		clock_index ++;
		// since we are going around circle, the clock_index may exceed the array
		// we need to set it 
		if (clock_index >= memsize) {
			clock_index = clock_index % memsize;
		}
 	}
	// store the frame index that need to be evict
	int res = clock_index;
	// pass handler to next clock_index
	clock_index ++;
	// using % to avoid out of indexed issue
	if (clock_index >= memsize) {
		clock_index = clock_index % memsize;
	}
	return res;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	// no action need here, since the PG_REF status is being handle on the pagetable.c
	// and the frame are used up one by one again
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	clock_index = 0;
}
