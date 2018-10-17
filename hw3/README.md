WHAT IS THIS:
    It's the Extend-Mini-Library for lauguage C,
    and can handle Signals.

BEFORE START:
    You should fellow the funtion prototype defined in Extend-Mini-Library
    and then write your C code.

HOW TO COMPILE/REMOVE EXECUTABLE CODE:
    1. use 'make [testfile]' to compile the code in static lib.
    2. use 'make [testfile+s]' to compile the code in shared lib.
    2. use 'make clean' to remove the executable code.

HOW TO RUN THIS PROGROM:
    1. For static library: use './[testfile]'
    2. For shared library: use 'LD_LIBRARY_PATH=. ./[testfile+s]'

FUNCTIONALITY THAT THIS Extend-Mini-Library SUPPORT:
    1. write: write data to a specific file descriptor
    2. setjmp: prepare for long jump by saving the current CPU state. 
               In addition, preserve the signal mask of the current process.
    3. longjmp: perform the long jump by restoring a saved CPU state. 
                In addition, restore the preserved signal mask.
    4. sigaction: setup the handler of a signal.
    5. sigprocmask: can be used to block/unblock signals, and get/set the current signal mask.
    6. alarm: setup a timer for the current process.
    7. pause: suspend the execution of the current process until a signal is delivered.
    8. sleep: suspend the execution of the current process for a given period.
    9. exit: terminate the current process.

RUNNING EXAMPLE:
    $ cat test0.c
#include "libminic.h"
int main() {
    write(1, "sleeping ... ", 13);
    sleep(3);
    write(1, "done\n", 5);
    return 0;
}

    $ make test0
yasm -f elf64 start.asm -o start.o
yasm -f elf64 libminic.asm -o libminic.o
gcc -c -g -Wall -fno-stack-protector test0.c
gcc -nostdlib -o test0 test0.o start.o libminic.o
rm test0.o start.o

    $ ./test0
sleeping ... done

    $ make test0s
yasm -f elf64 start.asm -o start.o
ld -shared -o libminic.so libminic.o
gcc -c -g -Wall -fno-stack-protector test0.c
gcc -nostdlib -o test0s test0.o start.o libminic.so
rm test0.o start.o

    $ LD_LIBRARY_PATH=. ./test0s
sleeping ... done
