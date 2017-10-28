//AVL Tree Functions

#include <linux/module.h>	//Needed by all modules
#include <linux/init.h>		//Needed for the macros
#include <linux/kernel.h>	//Needed for KERN_INFO
#include <linux/fs.h>		//Needed by filp
#include <asm/uaccess.h>	//Needed by segment descriptors
#include <linux/kmod.h>		//Needed for executing shell commands
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <asm/segment.h>
#include <linux/buffer_head.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/console_struct.h>
#include <linux/export.h>

#include "avl_tree.h"

int key(struct Node *N){
	if(N == NULL)
		return 0;
	return N->key;
}
EXPORT_SYMBOL(key);

int height(struct Node *N){
	if (N == NULL)
		return 0;
	return N->height;
}

struct Node* newNode(int key){
	struct Node* node = (struct Node*)kmalloc(sizeof(struct Node), GFP_ATOMIC);
	node->key = key;
	node->left = NULL;
	node->right = NULL;
	node->height = 1;
	return(node);
}

struct Node *rightRotate(struct Node *y){
	struct Node *x = y->left;
	struct Node *T2 = x->right;

	x->right = y;
	y->left = T2;

	y->height = max(height(y->left), height(y->right))+1;
	x->height = max(height(x->left), height(x->right))+1;

	return x;
}

struct Node *leftRotate(struct Node *x){
	struct Node *y = x->right;
	struct Node *T2 = y->left;

	y->left = x;
	x->right = T2;

	x->height = max(height(x->left), height(x->right))+1;
	y->height = max(height(y->left), height(y->right))+1;

	return y;
}

int getBalance(struct Node *N){
	if (N == NULL)
		return 0;
	return height(N->left) - height(N->right);
}

struct Node* minValueNode(struct Node* node){
	struct Node* curr = node;

	if (node == NULL)
		return NULL;

	while (curr->left != NULL)
		curr = curr->left;

	return curr;
}
EXPORT_SYMBOL(minValueNode);

struct Node* insert(struct Node* node, int key){
	int balance;

	if (node == NULL)
        	return(newNode(key));

	if (key < node->key)
		node->left  = insert(node->left, key);
	else if (key > node->key)
		node->right = insert(node->right, key);
	else
	return node;

	node->height = 1 + max(height(node->left), height(node->right));
	balance = getBalance(node);

	if (balance > 1 && key < node->left->key)
		return rightRotate(node);

	if (balance < -1 && key > node->right->key)
		return leftRotate(node);

	if (balance > 1 && key > node->left->key){
		node->left =  leftRotate(node->left);
		return rightRotate(node);
	}

	if (balance < -1 && key < node->right->key){
		node->right = rightRotate(node->right);
		return leftRotate(node);
	}

	return node;
}
EXPORT_SYMBOL(insert);

struct Node* deleteNode(struct Node* root, int key){
	int balance;

	if (root == NULL)
		return root;

	if ( key < root->key )
		root->left = deleteNode(root->left, key);

	else if( key > root->key )
		root->right = deleteNode(root->right, key);

	else{
		if( (root->left == NULL) || (root->right == NULL) ){
			struct Node *temp = root->left ? root->left : root->right;

			if (temp == NULL){
				temp = root;
				root = NULL;
			}
			else
				*root = *temp;
			kfree(temp);
		}
		else{
			struct Node* temp = minValueNode(root->right);
			root->key = temp->key;
			root->right = deleteNode(root->right, temp->key);
		}
	}

	if (root == NULL)
		return root;

	root->height = 1 + max(height(root->left), height(root->right));
	balance = getBalance(root);

	if (balance > 1 && getBalance(root->left) >= 0)
		return rightRotate(root);

	if (balance > 1 && getBalance(root->left) < 0){
		root->left =  leftRotate(root->left);
		return rightRotate(root);
	}

	if (balance < -1 && getBalance(root->right) <= 0)
		return leftRotate(root);

	if (balance < -1 && getBalance(root->right) > 0){
		root->right = rightRotate(root->right);
		return leftRotate(root);
	}

	return root;
}
EXPORT_SYMBOL(deleteNode);

struct Node* search(struct Node* root, int key){
	if (root == NULL)
		return root;

	if ( key < root->key )
		root->left = search(root->left, key);

	else if( key > root->key )
		root->right = search(root->right, key);

	return root;
}
EXPORT_SYMBOL(search);

struct Node* prev(struct Node* root, struct Node* pre, int key){
	struct Node* tmp;

	if (root == NULL)
		return NULL;

	if (root->key == key){
		if (root->left != NULL){
			tmp = root->left;

			while (tmp->right)
				tmp = tmp->right;
			return tmp;
		}
		else
			return pre;
	}

	if (root->key > key)
		return prev(root->left, root, key);
	else
		return prev(root->right, pre, key);

	return NULL;
}
EXPORT_SYMBOL(prev);

struct Node* next(struct Node* root, struct Node* suc, int key){
	struct Node* tmp;

	if (root == NULL)
		return NULL;

	if (root->key == key){
		if (root->right != NULL){
			tmp = root->right;

			while (tmp->left)
				tmp = tmp->left;
			return tmp;
		}
		else
			return suc;
	}

	if (root->key < key)
		return next(root->right, suc, key);
	else
		return next(root->left, root, key);

	return NULL;
}
EXPORT_SYMBOL(next);

void preOrder(struct Node *root){
	if(root != NULL){
		printk(KERN_INFO "%d ", root->key);
		preOrder(root->left);
		preOrder(root->right);
	}
}
EXPORT_SYMBOL(preOrder);

void inOrder(struct Node *root){
	if(root != NULL){
		inOrder(root->left);
		printk(KERN_INFO "%d", root->key);
		inOrder(root->right);
	}
}
EXPORT_SYMBOL(inOrder);
