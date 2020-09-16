#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Define an AVL tree
struct node {
	int key;
	int height;
	struct node *left;
	struct node *right;
};

// Get the height of the node
int height(struct node *N) {
	if (N == NULL) {
		return 0;
	} else {
		return N->height;
	}
}

// Comparsion function
int compare(int first, int second) {
	if (first > second) {
		return first;
	} else {
		return second;
	}
}

// Allocate a new node
struct node* newNode(int key) {
	struct node* node = (struct node*)malloc(sizeof(struct node));
	node -> key = key;
	node -> left = NULL;
	node -> right = NULL;
	node -> height = 1;
	return node;
}

// Right rotation
struct node *rightRotate(struct node *t) {
	struct node *lc = t -> left;
	struct node *rc = lc -> right;
	lc->right = t;
	t->left = rc;
	// update the height
	t -> height = compare(height(t->left), height(t->right))+1;
	lc -> height = compare(height(lc->left), height(lc->right))+1;
	// return new root
	return lc;
}

// Left rotation
struct node *leftRotate(struct node *x) {
	struct node *y = x -> right;
	struct node *T2 = y -> left;
	y->left = x; 
    x->right = T2; 
    //  Update heights 
    x->height = compare(height(x->left), height(x->right))+1; 
    y->height = compare(height(y->left), height(y->right))+1; 
    // Return new root 
    return y; 
}

// Get Balance factor of node N 
int getBalance(struct node *N) 
{ 
    if (N == NULL) 
        return 0; 
    return height(N->left) - height(N->right); 
}

// Recursive function to insert a key in the subtree rooted 
// with node and returns the new root of the subtree. 
struct node* insert(struct node* node, int key) 
{ 
    /* 1.  Perform the normal BST insertion */
    if (node == NULL) 
        return(newNode(key)); 
  
    if (key < node->key) 
        node->left  = insert(node->left, key); 
    else if (key > node->key) 
        node->right = insert(node->right, key); 
    else // Equal keys are not allowed in BST 
        return node; 
  
    /* 2. Update height of this ancestor node */
    node->height = 1 + compare(height(node->left), 
                           height(node->right)); 
  
    /* 3. Get the balance factor of this ancestor 
          node to check whether this node became 
          unbalanced */
    int balance = getBalance(node); 
  
    // If this node becomes unbalanced, then 
    // there are 4 cases 
  
    // Left Left Case 
    if (balance > 1 && key < node->left->key) 
        return rightRotate(node); 
  
    // Right Right Case 
    if (balance < -1 && key > node->right->key) 
        return leftRotate(node); 
  
    // Left Right Case 
    if (balance > 1 && key > node->left->key) 
    { 
        node->left =  leftRotate(node->left); 
        return rightRotate(node); 
    } 
  
    // Right Left Case 
    if (balance < -1 && key < node->right->key) 
    { 
        node->right = rightRotate(node->right); 
        return leftRotate(node); 
    } 
  
    /* return the (unchanged) node pointer */
    return node; 
}

void preOrder(struct node *root) 
{ 
    if(root != NULL) 
    { 
        printf("%d ", root->key); 
        preOrder(root->left); 
        preOrder(root->right); 
    } 
} 

int main(int argc, char ** argv) {
	struct node *testTree = NULL;
	for (int i = 0; i < 8; i++) {
		testTree = insert(testTree, i * 3);
		testTree = insert(testTree, i * 2);
	}
	testTree = insert(testTree, 30);
	printf("Preorder traversal of the tree is \n");
	return 0;
}
