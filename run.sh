#!/bin/bash
make
cp ./microkernel.elf /u/cs452/tftp/ARM/temp/y247pan/microkernel.elf
chmod o=r /u/cs452/tftp/ARM/temp/y247pan/microkernel.elf