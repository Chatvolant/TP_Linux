obj-m:=hello_noyau.o
KERNEL_SOURCE=/usr/bin/arm-linux-gnueabihf-gcc
CFLAGS_MODULE=-fno-pic

all :
	make -C $(KERNEL_SOURCE) M=$(PWD) modules
clean :
	make -C $(KERNEL_SOURCE) M=$(PWD) clean
install :
	make −C $(KERNEL_SOURCE) M=$(PWD) modules install

