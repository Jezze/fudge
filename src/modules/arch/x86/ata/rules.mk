M:=\
    $(DIR_SRC)/modules/arch/x86/ata/ata.ko \

N:=\
    $(DIR_SRC)/modules/arch/x86/ata/ata.ko.map \

O:=\
    $(DIR_SRC)/modules/arch/x86/ata/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
