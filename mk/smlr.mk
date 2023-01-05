TARGET2:=i386-unknown-elf

AR=$(TARGET2)-ar
AS=$(TARGET2)-as
CC=$(TARGET)cc
LD=$(TARGET)l
#LD=$(TARGET2)-ld
NASM=nasm
NM=$(TARGET2)-nm
PP=$(TARGET)pp
AR_FLAGS=rcs
AS_FLAGS=-c
CC_FLAGS=-S -no-leading-underscore
NASM_FLAGS=-f elf
NM_FLAGS=-gp
PP_FLAGS= -I$(DIR_INCLUDE) -I$(DIR_LIB) -I$(DIR_SRC)
LD_BIN=$(TARGET)l -elf -entry _start
#LD_FLAGS_KBIN=$(TARGET)l -elf -origin 0x100000 -entry mboot_init -nobss
#LD_FLAGS_KBIN=-static -nostdlib -T$(DIR_SRC)/kernel/$(ARCH)/$(LOADER)/linker.ld
#LD_FLAGS_KMOD=-static -nostdlib -T$(DIR_SRC)/modules/linker.ld -r

%.d: %.s
	@echo DP $@
	@echo $*.o: > $*.d

%.d: %.c
	@echo DP $@
	@echo $*.o: > $*.d

%.i: %.c
	@echo PP $@
	@$(PP) $(PP_FLAGS) $^ > $@

%.asm: %.i
	@echo CC $@
	@$(CC) $(CC_FLAGS) $^

%.o: %.s
	@echo AS $@
	@$(AS) $(AS_FLAGS) -o $@ $^

%.map: %
	@echo NM $@
	@$(NM) $(NM_FLAGS) $^ > $@

%.o: %.asm
	@echo NASM $@
	@$(NASM) $(NASM_FLAGS) -o $@ $^
