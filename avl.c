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
		printk(KERN_INFO "buf:%s", buf);
		filp_close(f,NULL);
	}
	set_fs(fs);

	return 0;
}

static void __exit clean_up(void){
	printk(KERN_INFO "Exitting module...\n");
}

module_init(init);
module_exit(clean_up);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arun Joseph<arunjosephkv2@gmail.com>");
MODULE_DESCRIPTION("AVL Tree Scheduler");
