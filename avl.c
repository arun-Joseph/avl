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
#include <linux/export.h>

#include <linux/avl_tree.h>

extern int fg_console;
struct Node *root;
struct Node *ptr;
extern int key(struct Node*);
extern struct Node* minValueNode(struct Node*);
extern struct Node* insert(struct Node*, int);
extern struct Node* deleteNode(struct Node*, int);
extern struct Node* search(struct Node*, int);
extern struct Node* prev(struct Node*, struct Node*, int);
extern struct Node* next(struct Node*, struct Node*, int);
extern void preOrder(struct Node*);
extern void inOrder(struct Node*);

struct timer_list my_timer;
struct tty_driver *my_driver;

char buf[512];
int counter, tim, curr;

#define BLINK_DELAY 2*HZ
#define ALL_LEDS_ON 0x07
#define RESTORE_LEDS 0xFF

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
			printk("Inserting : %d", num);
			root=insert(root, num);
		}
		else if(buf[counter] == 'r'){
			counter+=2;
			num=0;
			while(buf[counter] != '\n' && buf[counter] != '\0'){
				num=num*10 + (int)buf[counter] - 48;
				counter+=1;
			}
			printk("Deleting : %d", num);
			root=deleteNode(root, num);
		}
		else{
			counter+=1;
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
		curr=key(ptr);
		printk(KERN_INFO "Current process : %d", curr);
	}
	else{
		curr=key(ptr);
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

	for(i=0;i<512;i++)
		buf[i]=0;

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
		curr=key(ptr);
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

	while(root != NULL)
		root=deleteNode(root, key(root));
}

module_init(init);
module_exit(clean_up);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Adarsh Sunilkumar, Arun Joseph, Athul Ajesh");
MODULE_DESCRIPTION("Scheduling using AVL Trees");
