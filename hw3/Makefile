start.o: start.asm
	yasm -f elf64 start.asm -o start.o
libminic.o: libminic.asm
	yasm -f elf64 libminic.asm -o libminic.o
libminic.so: libminic.o
	ld -shared -o libminic.so libminic.o
test%.o: test%.c
	gcc -c -g -Wall -fno-stack-protector $<

test%s: test%.o start.o libminic.so
	gcc -nostdlib -o $@ $^
	rm $< start.o
test%: test%.o start.o libminic.o
	gcc -nostdlib -o $@ $^
	rm $< start.o

clean:
	rm *.o
