obj-m:=cheni_device.o
KERNEL_SOURCE=~/linux-socfpga
CFLAGS_MODULE:=-fno-pic
all :
	make -C $(KERNEL_SOURCE) M=$(PWD) modules CFLAGS_MODULE=$(CFLAGS_MODULE)
clean :
	make -C $(KERNEL_SOURCE) M=$(PWD) clean
install :
	make −C $(KERNEL_SOURCE) M=$(PWD) modules install CFLAGS_MODULE=$(CFLAGS_MODULE)

