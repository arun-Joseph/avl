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
}*root=NULL, *ptr=NULL;

extern int fg_console;

struct timer_list my_timer;
struct tty_driver *my_driver;

char buf[512];
int counter, tim, curr;

#define BLINK_DELAY 1*HZ
#define ALL_LEDS_ON 0x07
#define RESTORE_LEDS 0xFF

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

struct Node* search(struct Node* root, int key){
	if (root == NULL)
		return root;

	if ( key < root->key )
		root->left = search(root->left, key);

	else if( key > root->key )
		root->right = search(root->right, key);

	return root;
}

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

void list_files(void){
	char *argv[4],*envp[4];

	argv[0]="/bin/bash";
	argv[1]="-c";
	argv[2]=" /bin/ls /home/arun/Desktop/test > /home/arun/Desktop/avl/file.txt";
	argv[3]=NULL;

	envp[0]="HOME=/";
	envp[1]="TERM=linux";
	envp[2]="PATH=/sbin:/usr/sbin:/usr/bin";
	envp[3]=NULL;

	call_usermodehelper(argv[0], argv, envp, UMH_WAIT_EXEC);
}

static void my_timer_func(unsigned long data){
	int num=0;
	
	((my_driver->ops)->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
	
	while(tim == data){
		((my_driver->ops)->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, ALL_LEDS_ON);

		counter+=1;
		if(buf[counter] == 'i'){
			counter+=2;
			num=0;
			while(buf[counter] != '\n' && buf[counter] != '\0'){
				num=num*10 + (int)buf[counter] - 48;
				counter+=1;
			}
			root=insert(root, num);
		}
		else if(buf[counter] == 'r'){
			counter+=2;
			while(buf[counter] != '\n' && buf[counter] != '\0'){
				num=num*10 + (int)buf[counter] - 48;
				counter+=1;
			}
			root=deleteNode(root, num);
		}
		else{			
			printk(KERN_INFO "PreOrder : ");
			preOrder(root);
			printk(KERN_INFO "Inorder : ");
			inOrder(root);
		}

		if(buf[counter] != '\0'){
			counter+=1;
			tim=0;
			while(buf[counter] != ' '){
				tim=tim*10 + (int)buf[counter] - 48;
				counter+=1;
			}
		}
		else
			tim=0;
	}

	printk(KERN_INFO "Time : %ld ; ", data);
	ptr=next(root, NULL, curr);

	if (root == NULL && ptr == NULL)
		curr=-1;
	else if (ptr == NULL){
		ptr=minValueNode(root);
		curr=ptr->key;
		printk(KERN_INFO "Current process : %d", curr);
	}
	else{
		curr=ptr->key;
		printk(KERN_INFO "Current process : %d", curr);
	}

	my_timer.data=data+1;
	my_timer.expires=jiffies+BLINK_DELAY;
	add_timer(&my_timer);
}

static int __init init(void){
	struct file *f;
	mm_segment_t fs;
	int i,ret,num;

	for(i=0;i<512;i++)
		buf[i]=0;

	printk(KERN_INFO "Loading module...\n");

	list_files();

	fs=get_fs();
	set_fs(get_ds());
	f=filp_open("/home/arun/Desktop/avl/file.txt", O_RDONLY, 0);
	if(f==NULL)
		printk(KERN_ALERT "filp_open error!!!");
	else{
		ret=vfs_read(f, buf, 512, &f->f_pos);
		filp_close(f,NULL);
	}
	set_fs(fs);

	i=0;
	num=0;
	while(buf[i] != '\0'){
		if(buf[i] == '\n'){
			root=insert(root, num);
			num=0;
		}
		else{
			num=num*10 + (int)buf[i] - 48;
		}
		i++;
	}

	if(i!=0)
		root=insert(root, num);

	fs=get_fs();
	set_fs(get_ds());
	f=filp_open("/home/arun/Desktop/avl/input.txt", O_RDONLY, 0);
	if(f==NULL)
		printk(KERN_ALERT "filp_open error!!!");
	else{
		ret=vfs_read(f, buf, 512, &f->f_pos);
		filp_close(f,NULL);
	}
	set_fs(fs);

	tim=0;
	counter=0;
	if(buf[counter] != '\0'){
		while(buf[counter] != ' '){
			tim=tim*10 + (int)buf[counter] - 48;
			counter+=1;
		}
	}

	ptr=minValueNode(root);
	if (ptr != NULL)
		curr=ptr->key;
	else
		curr=-1;

	printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);
	for (i = 0; i < MAX_NR_CONSOLES; i++) {
		if (!vc_cons[i].d)
			break;
	}

	my_driver = vc_cons[fg_console].d->port.tty->driver;
	printk(KERN_INFO "kbleds: tty driver magic %x\n", my_driver->magic);

	init_timer(&my_timer);
	my_timer.function=my_timer_func;
	my_timer.data=1;
	my_timer.expires=jiffies+BLINK_DELAY;
	add_timer(&my_timer);

	return 0;
}

static void __exit clean_up(void){
	printk(KERN_INFO "Exitting module...\n");
	del_timer(&my_timer);
	((my_driver->ops)->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
}

module_init(init);
module_exit(clean_up);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arun Joseph<arunjosephkv2@gmail.com>");
MODULE_DESCRIPTION("AVL Tree Scheduler");
