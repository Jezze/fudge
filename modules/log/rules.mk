BUILD+=modules/log/log.ko
CLEAN+=$(OBJ_modules/log/log.ko) modules/log/log.ko

OBJ_modules/log/log.ko:=modules/log/main.o modules/log/driver.o

modules/log/log.ko: GCCFLAGS+=-Iinclude
modules/log/log.ko: LDFLAGS+=-Tmodules/linker.ld -r lib/memory.o lib/string.o
modules/log/log.ko: lib $(OBJ_modules/log/log.ko)
	$(LD) $(LDFLAGS) -o $@ $(OBJ_modules/log/log.ko)

