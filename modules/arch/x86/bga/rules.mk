MODULES+=modules/arch/x86/bga/bga.ko
CLEAN+=modules/arch/x86/bga/main.o modules/arch/x86/bga/driver.o

modules/arch/x86/bga/bga.ko: lib/memory.o lib/string.o lib/arch/x86/io.o modules/arch/x86/bga/main.o modules/arch/x86/bga/driver.o
	$(LD) $(LDFLAGS) -o $@ $^
