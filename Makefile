obj-m+=avltree.o
avltree-objs := avl_tree.o avl.o

KDIR := /lib/modules/$(shell uname -r)/build/
PWD := $(shell pwd)

EXTRA_CFLAGS += -I$(PWD)
EXTRA_CFLAGS += -O2

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
