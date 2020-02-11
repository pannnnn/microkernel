#
# Makefile for microkernel
#
XDIR=/u/cs452/public/xdev
XBINDIR=$(XDIR)/bin
XLIBDIR1=$(XDIR)/arm-none-eabi/lib
XLIBDIR2=$(XDIR)/lib/gcc/arm-none-eabi/9.2.0
CC = $(XBINDIR)/arm-none-eabi-gcc
AR = $(XBINDIR)/arm-none-eabi-ar
AS = $(XBINDIR)/arm-none-eabi-as
LD = $(XBINDIR)/arm-none-eabi-ld

# -g: include debug information for gdb
# -S: only compile and emit assembly
# -fPIC: emit position-independent code
# -Wall: report all warnings
# -mcpu=arm920t: generate code for the 920t architecture
# -msoft-float: no FP co-processor
CFLAGS = -O3 -g -S -fPIC -Wall -mcpu=arm920t -msoft-float -I. -I./include

# -static: force static linking
# -e: set entry point
# -nmagic: no page alignment
# -T: use linker script
LDFLAGS = -O3 -static -e main -nmagic -T linker.ld -L ./lib -L $(XLIBDIR1) -L $(XLIBDIR2)

CSOURCES = $(wildcard src/lib/*.c) $(wildcard src/user/*.c)  $(wildcard src/kernel/*.c)
ASMSOURCES = $(wildcard src/kernel/*.S)
ASMFILES = $(CSOURCES:.c=.s)
OBJECTS = $(CSOURCES:.c=.o) $(ASMSOURCES:.S=.o)
MAIN = main
EXEC = k4

all: clean $(ASMFILES) $(OBJECTS) $(EXEC).elf

debug: CFLAGS += -DDEBUG=1
debug: all

$(MAIN).s: $(MAIN).c
	$(CC) -S $(CFLAGS) $(MAIN).c

$(MAIN).o: $(MAIN).s
	$(AS) $(ASFLAGS) -o $(MAIN).o $(MAIN).s

$(EXEC).elf: $(MAIN).o $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ -lc -lgcc

src/%.s: src/%.c
	$(CC) -S $(CFLAGS) -o $@ $<

src/%.o: src/%.s
	$(AS) $(ASFLAGS) -o $@ $<

src/%.o: src/%.S
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	-rm -f *.elf *.s *.o
	-rm -f $(OBJECTS)
	-rm -f $(ASMFILES)
