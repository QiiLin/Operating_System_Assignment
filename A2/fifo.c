#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap; // store all the pysical frame and their status and page 


// the index of the frame that will be evicted
int fifo_start;

/* Page to evict is chosen using the fifo algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int fifo_evict() {
    int result = fifo_start;
	// since the pagetable entries are being attached to the coremap sequentially, so the order of eviction
	// should also be increasing 1 by 1 as well, goes back to 0 when it has already went over all the
	// indices
    fifo_start = (fifo_start + 1) % memsize;
    return result;
}

/* This function is called on each access to a page to update any information
 * needed by the fifo algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void fifo_ref(pgtbl_entry_t *p) {
	// there is no need to change anything when a new entry entered, because we just need to evict
	// by the index fifo_start
	return;
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
	// init the index starting from 0
	fifo_start = 0;
}
