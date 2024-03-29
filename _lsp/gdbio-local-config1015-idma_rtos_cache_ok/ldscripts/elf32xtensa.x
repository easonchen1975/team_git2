/* This linker script generated from xt-genldscripts.tpp for LSP . */
/* Linker Script for default link */
MEMORY
{
  dram0_0_seg :                       	org = 0x00080000, len = 0x40000
  dram1_0_seg :                       	org = 0x000C0000, len = 0x40000
  srom0_seg :                         	org = 0x40080000, len = 0x1000000
  sram0npu_seg :                      	org = 0x70000000, len = 0x10000000
  iram0_0_seg :                       	org = 0x80000000, len = 0x20000
  sram0_seg :                         	org = 0x80020000, len = 0x100000
  sram1_seg :                         	org = 0x80120000, len = 0x180
  sram2_seg :                         	org = 0x80120180, len = 0x40
  sram3_seg :                         	org = 0x801201C0, len = 0x40
  sram4_seg :                         	org = 0x80120200, len = 0x40
  sram5_seg :                         	org = 0x80120240, len = 0x40
  sram6_seg :                         	org = 0x80120280, len = 0x40
  sram7_seg :                         	org = 0x801202C0, len = 0x80
  sram8_seg :                         	org = 0x80120340, len = 0x63FCC0
}

PHDRS
{
  dram0_0_phdr PT_LOAD;
  dram0_0_bss_phdr PT_LOAD;
  dram1_0_phdr PT_LOAD;
  dram1_0_bss_phdr PT_LOAD;
  srom0_phdr PT_LOAD;
  sram0npu_phdr PT_LOAD;
  iram0_0_phdr PT_LOAD;
  sram0_phdr PT_LOAD;
  sram0_bss_phdr PT_LOAD;
  sram1_phdr PT_LOAD;
  sram2_phdr PT_LOAD;
  sram3_phdr PT_LOAD;
  sram4_phdr PT_LOAD;
  sram5_phdr PT_LOAD;
  sram6_phdr PT_LOAD;
  sram7_phdr PT_LOAD;
  sram8_phdr PT_LOAD;
  sram8_bss_phdr PT_LOAD;
}


/*  Default entry point:  */
ENTRY(_ResetVector)


/*  Memory boundary addresses:  */
_memmap_mem_dram0_start = 0x80000;
_memmap_mem_dram0_end   = 0xc0000;
_memmap_mem_dram1_start = 0xc0000;
_memmap_mem_dram1_end   = 0x100000;
_memmap_mem_srom_start = 0x40080000;
_memmap_mem_srom_end   = 0x41080000;
_memmap_mem_iram0_start = 0x80000000;
_memmap_mem_iram0_end   = 0x80020000;
_memmap_mem_sram_start = 0x80020000;
_memmap_mem_sram_end   = 0x80760000;
_memmap_mem_sramnpu_start = 0x70000000;
_memmap_mem_sramnpu_end   = 0x80000000;

/*  Memory segment boundary addresses:  */
_memmap_seg_dram0_0_start = 0x80000;
_memmap_seg_dram0_0_max   = 0xc0000;
_memmap_seg_dram1_0_start = 0xc0000;
_memmap_seg_dram1_0_max   = 0x100000;
_memmap_seg_srom0_start = 0x40080000;
_memmap_seg_srom0_max   = 0x41080000;
_memmap_seg_sram0npu_start = 0x70000000;
_memmap_seg_sram0npu_max   = 0x80000000;
_memmap_seg_iram0_0_start = 0x80000000;
_memmap_seg_iram0_0_max   = 0x80020000;
_memmap_seg_sram0_start = 0x80020000;
_memmap_seg_sram0_max   = 0x80120000;
_memmap_seg_sram1_start = 0x80120000;
_memmap_seg_sram1_max   = 0x80120180;
_memmap_seg_sram2_start = 0x80120180;
_memmap_seg_sram2_max   = 0x801201c0;
_memmap_seg_sram3_start = 0x801201c0;
_memmap_seg_sram3_max   = 0x80120200;
_memmap_seg_sram4_start = 0x80120200;
_memmap_seg_sram4_max   = 0x80120240;
_memmap_seg_sram5_start = 0x80120240;
_memmap_seg_sram5_max   = 0x80120280;
_memmap_seg_sram6_start = 0x80120280;
_memmap_seg_sram6_max   = 0x801202c0;
_memmap_seg_sram7_start = 0x801202c0;
_memmap_seg_sram7_max   = 0x80120340;
_memmap_seg_sram8_start = 0x80120340;
_memmap_seg_sram8_max   = 0x80760000;

_rom_store_table = 0;
PROVIDE(_memmap_reset_vector = 0x80000000);
PROVIDE(_memmap_vecbase_reset = 0x80120000);
/* Various memory-map dependent cache attribute settings: */
_memmap_cacheattr_wb_base = 0x00011104;
_memmap_cacheattr_wt_base = 0x00033304;
_memmap_cacheattr_bp_base = 0x00044404;
_memmap_cacheattr_unused_mask = 0xFFF000F0;
_memmap_cacheattr_wb_trapnull = 0x44411144;
_memmap_cacheattr_wba_trapnull = 0x44411144;
_memmap_cacheattr_wbna_trapnull = 0x44422244;
_memmap_cacheattr_wt_trapnull = 0x44433344;
_memmap_cacheattr_bp_trapnull = 0x44444444;
_memmap_cacheattr_wb_strict = 0x00011104;
_memmap_cacheattr_wt_strict = 0x00033304;
_memmap_cacheattr_bp_strict = 0x00044404;
_memmap_cacheattr_wb_allvalid = 0x44411144;
_memmap_cacheattr_wt_allvalid = 0x44433344;
_memmap_cacheattr_bp_allvalid = 0x44444444;
_memmap_region_map = 0x0000001D;
PROVIDE(_memmap_cacheattr_reset = _memmap_cacheattr_wb_trapnull);

SECTIONS
{

  .dram0.rodata : ALIGN(4)
  {
    _dram0_rodata_start = ABSOLUTE(.);
    *(.dram0.rodata)
    . = ALIGN (4);
    _dram0_rodata_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .clib.rodata : ALIGN(4)
  {
    _clib_rodata_start = ABSOLUTE(.);
    *(.clib.rodata)
    . = ALIGN (4);
    _clib_rodata_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .rtos.rodata : ALIGN(4)
  {
    _rtos_rodata_start = ABSOLUTE(.);
    *(.rtos.rodata)
    . = ALIGN (4);
    _rtos_rodata_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .rodata : ALIGN(4)
  {
    _rodata_start = ABSOLUTE(.);
    *(.rodata)
    *(SORT(.rodata.sort.*))
    KEEP (*(SORT(.rodata.keepsort.*) .rodata.keep.*))
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    __XT_EXCEPTION_TABLE__ = ABSOLUTE(.);
    KEEP (*(.xt_except_table))
    KEEP (*(.gcc_except_table))
    *(.gnu.linkonce.e.*)
    *(.gnu.version_r)
    KEEP (*(.eh_frame))
    /*  C++ constructor and destructor tables, properly ordered:  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    /*  C++ exception handlers table:  */
    __XT_EXCEPTION_DESCS__ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    *(.dynamic)
    *(.gnu.version_d)
    . = ALIGN(4);		/* this table MUST be 4-byte aligned */
    _bss_table_start = ABSOLUTE(.);
    LONG(_clib_bss_start)
    LONG(_clib_bss_end)
    LONG(_dram1_bss_start)
    LONG(_dram1_bss_end)
    LONG(_sram_bss_start)
    LONG(_sram_bss_end)
    LONG(_bss_start)
    LONG(_bss_end)
    _bss_table_end = ABSOLUTE(.);
    . = ALIGN (4);
    _rodata_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .clib.data : ALIGN(4)
  {
    _clib_data_start = ABSOLUTE(.);
    *(.clib.data)
    . = ALIGN (4);
    _clib_data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .clib.percpu.data : ALIGN(4)
  {
    _clib_percpu_data_start = ABSOLUTE(.);
    *(.clib.percpu.data)
    . = ALIGN (4);
    _clib_percpu_data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .rtos.percpu.data : ALIGN(4)
  {
    _rtos_percpu_data_start = ABSOLUTE(.);
    *(.rtos.percpu.data)
    . = ALIGN (4);
    _rtos_percpu_data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .rtos.data : ALIGN(4)
  {
    _rtos_data_start = ABSOLUTE(.);
    *(.rtos.data)
    . = ALIGN (4);
    _rtos_data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .dram0.data : ALIGN(4)
  {
    _dram0_data_start = ABSOLUTE(.);
    *(.dram0.data)
    . = ALIGN (4);
    _dram0_data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .data : ALIGN(4)
  {
    _data_start = ABSOLUTE(.);
    *(.data)
    *(SORT(.data.sort.*))
    KEEP (*(SORT(.data.keepsort.*) .data.keep.*))
    *(.data.*)
    *(.gnu.linkonce.d.*)
    KEEP(*(.gnu.linkonce.d.*personality*))
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.sdata2)
    *(.sdata2.*)
    *(.gnu.linkonce.s2.*)
    KEEP(*(.jcr))
    *(__llvm_prf_cnts)
    *(__llvm_prf_data)
    *(__llvm_prf_vnds)
    . = ALIGN (4);
    _data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .clib.bss (NOLOAD) : ALIGN(8)
  {
    . = ALIGN (8);
    _clib_bss_start = ABSOLUTE(.);
    *(.clib.bss)
    *(.clib.percpu.bss)
    . = ALIGN (8);
    _clib_bss_end = ABSOLUTE(.);
    _stack_sentry = ALIGN(0x8);
    _memmap_seg_dram0_0_end = ALIGN(0x8);
  } >dram0_0_seg :dram0_0_bss_phdr

  PROVIDE(__stack = 0xc0000);

  .dram1.rodata : ALIGN(4)
  {
    _dram1_rodata_start = ABSOLUTE(.);
    *(.dram1.rodata)
    . = ALIGN (4);
    _dram1_rodata_end = ABSOLUTE(.);
  } >dram1_0_seg :dram1_0_phdr

  .dram1.data : ALIGN(4)
  {
    _dram1_data_start = ABSOLUTE(.);
    *(.dram1.data)
    . = ALIGN (4);
    _dram1_data_end = ABSOLUTE(.);
  } >dram1_0_seg :dram1_0_phdr

  .dram1.bss (NOLOAD) : ALIGN(8)
  {
    . = ALIGN (8);
    _dram1_bss_start = ABSOLUTE(.);
    *(.dram1.bss)
    . = ALIGN (8);
    _dram1_bss_end = ABSOLUTE(.);
    _memmap_seg_dram1_0_end = ALIGN(0x8);
  } >dram1_0_seg :dram1_0_bss_phdr


  .srom.rodata : ALIGN(4)
  {
    _srom_rodata_start = ABSOLUTE(.);
    *(.srom.rodata)
    . = ALIGN (4);
    _srom_rodata_end = ABSOLUTE(.);
  } >srom0_seg :srom0_phdr

  .srom.text : ALIGN(4)
  {
    _srom_text_start = ABSOLUTE(.);
    *(.srom.literal .srom.text)
    . = ALIGN (4);
    _srom_text_end = ABSOLUTE(.);
    _memmap_seg_srom0_end = ALIGN(0x8);
  } >srom0_seg :srom0_phdr


  .sram.npu : ALIGN(4)
  {
    _sram_npu_start = ABSOLUTE(.);
    *(.sram.npu)
    . = ALIGN (4);
    _sram_npu_end = ABSOLUTE(.);
    _memmap_seg_sram0npu_end = ALIGN(0x8);
  } >sram0npu_seg :sram0npu_phdr


  .ResetVector.text : ALIGN(4)
  {
    _ResetVector_text_start = ABSOLUTE(.);
    KEEP (*(.ResetVector.text))
    . = ALIGN (4);
    _ResetVector_text_end = ABSOLUTE(.);
  } >iram0_0_seg :iram0_0_phdr

  .ResetHandler.text : ALIGN(4)
  {
    _ResetHandler_text_start = ABSOLUTE(.);
    *(.ResetHandler.literal .ResetHandler.text)
    . = ALIGN (4);
    _ResetHandler_text_end = ABSOLUTE(.);
  } >iram0_0_seg :iram0_0_phdr

  .clib.text : ALIGN(4)
  {
    _clib_text_start = ABSOLUTE(.);
    *(.clib.literal .clib.text)
    . = ALIGN (4);
    _clib_text_end = ABSOLUTE(.);
  } >iram0_0_seg :iram0_0_phdr

  .rtos.text : ALIGN(4)
  {
    _rtos_text_start = ABSOLUTE(.);
    *(.rtos.literal .rtos.text)
    . = ALIGN (4);
    _rtos_text_end = ABSOLUTE(.);
  } >iram0_0_seg :iram0_0_phdr

  .idma_text : ALIGN(4)
  {
    _idma_text_start = ABSOLUTE(.);
    *(.idma_text)
    . = ALIGN (4);
    _idma_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_0_end = ALIGN(0x8);
  } >iram0_0_seg :iram0_0_phdr


  .sram.rodata : ALIGN(4)
  {
    _sram_rodata_start = ABSOLUTE(.);
    *(.sram.rodata)
    . = ALIGN (4);
    _sram_rodata_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .sram.text : ALIGN(4)
  {
    _sram_text_start = ABSOLUTE(.);
    *(.sram.literal .sram.text)
    . = ALIGN (4);
    _sram_text_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .sram.data : ALIGN(4)
  {
    _sram_data_start = ABSOLUTE(.);
    *(.sram.data)
    . = ALIGN (4);
    _sram_data_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  __llvm_prf_names : ALIGN(4)
  {
    __llvm_prf_names_start = ABSOLUTE(.);
    *(__llvm_prf_names)
    . = ALIGN (4);
    __llvm_prf_names_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .text.process_eason : ALIGN(4)
  {
    _text_process_eason_start = ABSOLUTE(.);
    *(.text.process_eason)
    . = ALIGN (4);
    _text_process_eason_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .text.processData : ALIGN(4)
  {
    _text_processData_start = ABSOLUTE(.);
    *(.text.processData)
    . = ALIGN (4);
    _text_processData_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .sram.bss (NOLOAD) : ALIGN(8)
  {
    . = ALIGN (8);
    _sram_bss_start = ABSOLUTE(.);
    *(.sram.bss)
    . = ALIGN (8);
    _sram_bss_end = ABSOLUTE(.);
    _memmap_seg_sram0_end = ALIGN(0x8);
  } >sram0_seg :sram0_bss_phdr


  .WindowVectors.text : ALIGN(4)
  {
    _WindowVectors_text_start = ABSOLUTE(.);
    KEEP (*(.WindowVectors.text))
    . = ALIGN (4);
    _WindowVectors_text_end = ABSOLUTE(.);
  } >sram1_seg :sram1_phdr

  .Level2InterruptVector.literal : ALIGN(4)
  {
    _Level2InterruptVector_literal_start = ABSOLUTE(.);
    *(.Level2InterruptVector.literal)
    . = ALIGN (4);
    _Level2InterruptVector_literal_end = ABSOLUTE(.);
    _memmap_seg_sram1_end = ALIGN(0x8);
  } >sram1_seg :sram1_phdr


  .Level2InterruptVector.text : ALIGN(4)
  {
    _Level2InterruptVector_text_start = ABSOLUTE(.);
    KEEP (*(.Level2InterruptVector.text))
    . = ALIGN (4);
    _Level2InterruptVector_text_end = ABSOLUTE(.);
  } >sram2_seg :sram2_phdr

  .Level3InterruptVector.literal : ALIGN(4)
  {
    _Level3InterruptVector_literal_start = ABSOLUTE(.);
    *(.Level3InterruptVector.literal)
    . = ALIGN (4);
    _Level3InterruptVector_literal_end = ABSOLUTE(.);
    _memmap_seg_sram2_end = ALIGN(0x8);
  } >sram2_seg :sram2_phdr


  .Level3InterruptVector.text : ALIGN(4)
  {
    _Level3InterruptVector_text_start = ABSOLUTE(.);
    KEEP (*(.Level3InterruptVector.text))
    . = ALIGN (4);
    _Level3InterruptVector_text_end = ABSOLUTE(.);
  } >sram3_seg :sram3_phdr

  .DebugExceptionVector.literal : ALIGN(4)
  {
    _DebugExceptionVector_literal_start = ABSOLUTE(.);
    *(.DebugExceptionVector.literal)
    . = ALIGN (4);
    _DebugExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_sram3_end = ALIGN(0x8);
  } >sram3_seg :sram3_phdr


  .DebugExceptionVector.text : ALIGN(4)
  {
    _DebugExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.DebugExceptionVector.text))
    . = ALIGN (4);
    _DebugExceptionVector_text_end = ABSOLUTE(.);
  } >sram4_seg :sram4_phdr

  .NMIExceptionVector.literal : ALIGN(4)
  {
    _NMIExceptionVector_literal_start = ABSOLUTE(.);
    *(.NMIExceptionVector.literal)
    . = ALIGN (4);
    _NMIExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_sram4_end = ALIGN(0x8);
  } >sram4_seg :sram4_phdr


  .NMIExceptionVector.text : ALIGN(4)
  {
    _NMIExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.NMIExceptionVector.text))
    . = ALIGN (4);
    _NMIExceptionVector_text_end = ABSOLUTE(.);
  } >sram5_seg :sram5_phdr

  .KernelExceptionVector.literal : ALIGN(4)
  {
    _KernelExceptionVector_literal_start = ABSOLUTE(.);
    *(.KernelExceptionVector.literal)
    . = ALIGN (4);
    _KernelExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_sram5_end = ALIGN(0x8);
  } >sram5_seg :sram5_phdr


  .KernelExceptionVector.text : ALIGN(4)
  {
    _KernelExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.KernelExceptionVector.text))
    . = ALIGN (4);
    _KernelExceptionVector_text_end = ABSOLUTE(.);
  } >sram6_seg :sram6_phdr

  .UserExceptionVector.literal : ALIGN(4)
  {
    _UserExceptionVector_literal_start = ABSOLUTE(.);
    *(.UserExceptionVector.literal)
    . = ALIGN (4);
    _UserExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_sram6_end = ALIGN(0x8);
  } >sram6_seg :sram6_phdr


  .UserExceptionVector.text : ALIGN(4)
  {
    _UserExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.UserExceptionVector.text))
    . = ALIGN (4);
    _UserExceptionVector_text_end = ABSOLUTE(.);
  } >sram7_seg :sram7_phdr

  .DoubleExceptionVector.literal : ALIGN(4)
  {
    _DoubleExceptionVector_literal_start = ABSOLUTE(.);
    *(.DoubleExceptionVector.literal)
    . = ALIGN (4);
    _DoubleExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_sram7_end = ALIGN(0x8);
  } >sram7_seg :sram7_phdr


  .DoubleExceptionVector.text : ALIGN(4)
  {
    _DoubleExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.DoubleExceptionVector.text))
    . = ALIGN (4);
    _DoubleExceptionVector_text_end = ABSOLUTE(.);
  } >sram8_seg :sram8_phdr

  .text : ALIGN(4)
  {
    _stext = .;
    _text_start = ABSOLUTE(.);
    *(.entry.text)
    *(.init.literal)
    KEEP(*(.init))
    *(.literal.sort.* SORT(.text.sort.*))
    KEEP (*(.literal.keepsort.* SORT(.text.keepsort.*) .literal.keep.* .text.keep.* .literal.*personality* .text.*personality*))
    *(.literal .text .literal.* .text.* .stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
    *(.fini.literal)
    KEEP(*(.fini))
    *(.gnu.version)
    . = ALIGN (4);
    _text_end = ABSOLUTE(.);
    _etext = .;
  } >sram8_seg :sram8_phdr

  .iram0.text : ALIGN(4)
  {
    _iram0_text_start = ABSOLUTE(.);
    *(.iram0.literal .iram.literal .iram.text.literal .iram0.text .iram.text)
    . = ALIGN (4);
    _iram0_text_end = ABSOLUTE(.);
  } >sram8_seg :sram8_phdr

  .bss (NOLOAD) : ALIGN(8)
  {
    . = ALIGN (8);
    _bss_start = ABSOLUTE(.);
    *(.dynsbss)
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    *(.sbss2)
    *(.sbss2.*)
    *(.gnu.linkonce.sb2.*)
    *(.dynbss)
    *(.bss)
    *(SORT(.bss.sort.*))
    KEEP (*(SORT(.bss.keepsort.*) .bss.keep.*))
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    *(.rtos.percpu.bss)
    *(.rtos.bss)
    *(.dram0.bss)
    . = ALIGN (8);
    _bss_end = ABSOLUTE(.);
    _end = ALIGN(0x8);
    PROVIDE(end = ALIGN(0x8));
    _memmap_seg_sram8_end = ALIGN(0x8);
  } >sram8_seg :sram8_bss_phdr

  _heap_sentry = 0x80760000;
  .debug  0 :  { *(.debug) }
  .line  0 :  { *(.line) }
  .debug_srcinfo  0 :  { *(.debug_srcinfo) }
  .debug_sfnames  0 :  { *(.debug_sfnames) }
  .debug_aranges  0 :  { *(.debug_aranges) }
  .debug_pubnames  0 :  { *(.debug_pubnames) }
  .debug_info  0 :  { *(.debug_info) }
  .debug_abbrev  0 :  { *(.debug_abbrev) }
  .debug_line  0 :  { *(.debug_line) }
  .debug_frame  0 :  { *(.debug_frame) }
  .debug_str  0 :  { *(.debug_str) }
  .debug_loc  0 :  { *(.debug_loc) }
  .debug_macinfo  0 :  { *(.debug_macinfo) }
  .debug_weaknames  0 :  { *(.debug_weaknames) }
  .debug_funcnames  0 :  { *(.debug_funcnames) }
  .debug_typenames  0 :  { *(.debug_typenames) }
  .debug_varnames  0 :  { *(.debug_varnames) }
  .xt.insn 0 :
  {
    KEEP (*(.xt.insn))
    KEEP (*(.gnu.linkonce.x.*))
  }
  .xt.prop 0 :
  {
    KEEP (*(.xt.prop))
    KEEP (*(.xt.prop.*))
    KEEP (*(.gnu.linkonce.prop.*))
  }
  .xt.lit 0 :
  {
    KEEP (*(.xt.lit))
    KEEP (*(.xt.lit.*))
    KEEP (*(.gnu.linkonce.p.*))
  }
  .debug.xt.callgraph 0 :
  {
    KEEP (*(.debug.xt.callgraph .debug.xt.callgraph.* .gnu.linkonce.xt.callgraph.*))
  }
}

