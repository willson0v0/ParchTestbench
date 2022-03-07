OUTPUT_ARCH(riscv)
ENTRY(_start)
BASE_ADDRESS = 0xFFFFFFFF00000000;

SECTIONS
{
	. = BASE_ADDRESS;
	.text : {
		*(.text .text.*)
	}
	
	. = ALIGN(4K);
	.rodata : {
		*(.rodata .rodata.*)
	}

	. = ALIGN(4K);
	.data : {
		*(.data .data.*)
	}

	. = ALIGN(4K);
	.sdata : {
		*(.sdata)
	}
	
	. = ALIGN(4K);
	.bss : {
		*(.bss .bss.*)
	}
}