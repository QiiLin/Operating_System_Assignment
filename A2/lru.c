#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;


// double linked struct that represent a frame
struct pgtbl_entry_node {
	struct pgtbl_entry_node* prev;
	struct pgtbl_entry_node* next;
	unsigned int frame;
};

struct pgtbl_entry_node* start;
struct pgtbl_entry_node* end;





/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
    struct pgtbl_entry_node* temp = end;
	// update end node
	end = temp-> prev;
	end->next = NULL;
	// store target frame index 
	int res = temp-> frame;
	// free temp node, since it is being evict, we don't need to keep track of it anymore
	free(temp);
	// return target frame number
	return res;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	// init varaibles
	struct pgtbl_entry_node* temp;
	struct pgtbl_entry_node* curr = start;
	int found = 0;
	// try to assign temp to hold the result
	if (start == NULL) {
		// if nothing is currently in the list, create a new node
		temp = (struct pgtbl_entry_node *) malloc(sizeof(struct pgtbl_entry_node));
		if (temp == NULL) {
			fprintf(stderr, "Memory allocation failed");
			exit(1);
		}
		// set values for temp node
		temp -> frame = p->frame >> PAGE_SHIFT;
		temp -> next = NULL;
		temp -> prev = NULL;
		// set start node and end node to it since there were nothing in the list before
		start = temp;
		end = temp;
		return;
	} else {
		// if there is some node in the linked list
		// try to find if the frame is already being recorded
		while (curr != NULL) {
			if (curr->frame == (p->frame >> PAGE_SHIFT)) {
				found = 1;
				break;
			}
			curr = curr -> next;
		}
		// if we found it
		if (found) {
			// handle the case it is the head, no action needed
			if (start == curr) {
				return;
			}
			// if it is the ned
			if (curr == end) {
				// remove from linked list
				end = curr -> prev;
				end-> next = NULL;
				curr-> prev = NULL;
				// put it back to the linked list as the head
				curr-> next = start;
				start-> prev = curr; 
				start = curr;
				return;
			}
			// it is found in the middle of linked list 
			temp = curr;
			// remove it from linked list
			curr->prev->next = curr->next;
			curr->next->prev = curr->prev;
			curr->next = NULL;
			curr->prev = NULL;
			
			// and place it to the head
			temp-> next = start;
			start-> prev = temp; 
			start = temp;
			return;
		} else {
			// if we didn't found it, we need to record it 
			temp = (struct pgtbl_entry_node *) malloc(sizeof(struct pgtbl_entry_node));
			if (temp == NULL) {
				fprintf(stderr, "Memory allocation failed");
				exit(1);
			}
			// set values in the temp node
			temp -> frame = p->frame >> PAGE_SHIFT;
		    temp -> next = NULL;
		    temp -> prev = NULL;
			// add to the head of the linked list
			temp-> next = start;
			start-> prev = temp; 
			start = temp;
			return;
		}
	}
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	start = NULL;
	end = NULL;
}
