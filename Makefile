#
# Makefile for busy-wait IO library
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
CFLAGS = -g -S -fPIC -Wall -mcpu=arm920t -msoft-float -I. -I./include

# -static: force static linking
# -e: set entry point
# -nmagic: no page alignment
# -T: use linker script
LDFLAGS = -static -e main -nmagic -T linker.ld -L ./lib -L $(XLIBDIR2)

REQS = trains.o task.o

all: trains.elf

task.s: task.c
	$(CC) -S $(CFLAGS) task.c

task.o: task.s
	$(AS) $(ASFLAGS) -o task.o task.s

trains.s: trains.c
	$(CC) -S $(CFLAGS) trains.c

trains.o: trains.s
	$(AS) $(ASFLAGS) -o trains.o trains.s

trains.elf: trains.o task.o
	$(LD) $(LDFLAGS) -o $@ $^ -lgcc
}

clean:
	-rm -f trains.elf *.s *.o
