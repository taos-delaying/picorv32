// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.

#include "firmware.h"
#include "fft32_q15.h"

void hello(void)
{
	print_str("PicoRV32 software FFT test\n");
	if (fft32_q15_demo() != 0) {
		/* Stop before start.S can report the normal tests-passed marker. */
		__asm__ volatile ("ebreak");
		for (;;) { }
	}
}

