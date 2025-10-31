/*
    *** User-defined linker script ***

    If you want to extend the UninaSoC.ld script, place here your code.
    If you want to redefine your own linker script, remove the UninaSoC include.

*/

INCLUDE ../../common/UninaSoC.ld

/* 

vector_table_start,defined in uninasoc.ld,is the address of reset handler
trap_vector_start is the base address for the interrupt handlers

*/

_trap_vectors_start = _vector_table_start + 0x0000000000000004;

SECTIONS {

	  .bss :  /*  _text_end has been already aligned to 32 bits in UninaSoC.ld */
	  {
	     _bss_start = .;
	     *(.bss*);
	     . = ALIGN(32);
	     _bss_end = .;
	  }> BRAM

	  .data : 
	  {
	     . = ALIGN(32);
	     _data_start = .;
	     *(.data*);
	     . = ALIGN(32);
	     _data_end = .;
	  }> BRAM

	  .rodata : 
	  {
	     . = ALIGN(32);
	     _rodata_start = .;
	     *(.rodata*);
	     . = ALIGN(32);
	     _rodata_end = .;
	  } > BRAM

}
