CONFIG_MODULE_SIG=n		# Sign the LKM
obj-m += mylkm.o		# LKM file name is "mylkm.ko"
mylkm-y := driver.o structures.o
all: defs.h driver.c structures.c
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
test: test_basic.c test_advanced.c
	gcc -Wall test_basic.c -o test_basic.out
	gcc -Wall test_advanced.c -o test_advanced.out -lpthread
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
distclean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
	rm -rf ./*.out
