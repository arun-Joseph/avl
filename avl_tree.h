#ifndef AVL_TREE_H
#define AVL_TREE_H

struct Node{
	int key;
	struct Node *left, *right;
	int height;
};

int key(struct Node*);
struct Node* minValueNode(struct Node*);
struct Node* insert(struct Node*, int);
struct Node* deleteNode(struct Node*, int);
struct Node* search(struct Node*, int);
struct Node* prev(struct Node*, struct Node*, int);
struct Node* next(struct Node*, struct Node*, int);
void preOrder(struct Node*);
void inOrder(struct Node*);

#endif
