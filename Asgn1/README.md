Commands --
1. Compile LKM: make
2. Clean generated files (NOT source code / test files): make clean
3. Insert LKM into kernel: sudo insmod mylkm.ko
4. Remove LKM from kernel: sudo rmmod mylkm.ko
5. The procfile to communicate with for accessing the deque is: /proc/partb_1_20CS10007_20CS30016
6. Compile test files: make test
7. Remove test files: make distclean
8. See kernel messages output by mylkm using: sudo dmesg | grep MYLKM
9. Source code files-
    defs.h          [header file]
    driver.c        [main driver implementation]
    structures.c    [supporting data structures' internal implementation]
    makefile        [to run make]
    
For detailed documentation of the implementation, see comments in source code
NOTE: 
    To allow multiple threads created by the same process to access the process-wide deque, each process in the kernel space is identified by the tgid and NOT the pid, since threads of the same process have different pids in kernel space, but the same tgids

Test files' description --
1. test_basic: 
    Test the normal flow of execution and check whether the LKM handles all cases properly and throws errors appropriately
2. test_advanced: 
    Test multiprocess and multithread access to the LKM and verify that no such cases cause races/deadlocks/memory overflow in kernel space while still guaranteeing concurrency and mututal-exclusion
