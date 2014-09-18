.text
.align 16
.globl rdtsc
rdtsc:
.byte 0x0f, 0x31
ret
	
