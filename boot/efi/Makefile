all:
	gcc -c boot.c -o boot.o -fno-stack-protector -fcf-protection=none -fpie
	x86_64-pe-ld boot.o -o boot.pe -mi386pep --image-base 0x400000 --subsystem=10
	x86_64-pe-objcopy -R .comment -R .eh_fram -R .note.gn boot.pe -S bootx64.efi
clean:
	rm -f boot.o boot.pe bootx64.efi
