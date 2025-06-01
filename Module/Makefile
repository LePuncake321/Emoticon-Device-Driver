KERNELDIR=/lib/modules/`uname -r`/build
BASENAME= EmoteCipher


MODULES = $(BASENAME).ko 
obj-m += $(BASENAME).o 

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

