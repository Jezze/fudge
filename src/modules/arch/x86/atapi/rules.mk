M:=\
    $(DIR_SRC)/modules/arch/x86/atapi/atapi.ko.0 \

O:=\
    $(DIR_SRC)/modules/arch/x86/atapi/main.o \

L:=\
    $(DIR_LIB)/fudge/fudge.a \

include $(DIR_MK)/kmod.mk
