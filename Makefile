obj-m += main.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc user.c -lm

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

