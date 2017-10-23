//AVL Tree Scheduler

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

struct Node{
	int key;
	struct Node *left, *right;
	int height;
}*root=NULL;

static struct timer_list my_timer;

#define BLINK_DELAY 10*HZ

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

struct Node* insert(struct Node* node, int key){
	int balance;

	printk(KERN_INFO "Inserting...");

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

void preOrder(struct Node *root){
	if(root != NULL){
		printk(KERN_INFO "%d ", root->key);
		preOrder(root->left);
		preOrder(root->right);
	}
}

void inOrder(struct Node *root){
	if(root != NULL){
		inOrder(root->left);
		printk(KERN_INFO "%d", root->key);
		inOrder(root->right);
	}
}

static void my_timer_func(unsigned long data){
	root=insert(root, 1);
	inOrder(root);
	my_timer.expires=jiffies+BLINK_DELAY;
	add_timer(&my_timer);
}

static int __init init(void){
	struct file *f;
	char buf[512];
	char *argv[4],*envp[4];
	mm_segment_t fs;
	int i,ret;

	for(i=0;i<512;i++)
		buf[i]=0;

	printk(KERN_INFO "Loaded module...\n");

	argv[0]="/bin/bash";
	argv[1]="-c";
	argv[2]=" /bin/ls /home/arun/Desktop/test > /home/arun/Desktop/avl/file.txt";
	argv[3]=NULL;

	envp[0]="HOME=/";
	envp[1]="TERM=linux";
	envp[2]="PATH=/sbin:/usr/sbin:/usr/bin";
	envp[3]=NULL;

	call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);

	fs=get_fs();
	set_fs(get_ds());
	f=filp_open("/home/arun/Desktop/avl/file.txt", O_RDONLY, 0);
	if(f==NULL)
		printk(KERN_ALERT "filp_open error!!!");
	else{
		printk(KERN_INFO "File opened");
		ret=vfs_read(f, buf, 512, &f->f_pos);

		init_timer(&my_timer);
		my_timer.function=my_timer_func;
		my_timer.data=0;
		my_timer.expires=jiffies+BLINK_DELAY;
		add_timer(&my_timer);

		filp_close(f,NULL);
	}
	set_fs(fs);

	return 0;
}

static void __exit clean_up(void){
	printk(KERN_INFO "Exitting module...\n");
	del_timer(&my_timer);
}

module_init(init);
module_exit(clean_up);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arun Joseph<arunjosephkv2@gmail.com>");
MODULE_DESCRIPTION("AVL Tree Scheduler");
