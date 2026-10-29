/* Terminate the frame unwind info section with a 4byte 0 as a sentinel;
   this would be the 'length' field in a real FDE.  PT_GNU_EH_FRAME only
   provides the start of .eh_frame, so unwinders that scan it linearly
   need the sentinel even when there is an .eh_frame_hdr.  Set the alignment
   explicitly so that the compiler does not over-align the array.  */
typedef unsigned int ui32 __attribute__ ((mode (SI)));
static const ui32 __FRAME_END__[1]
  __attribute__ ((used, section (".eh_frame"), aligned (__alignof__ (ui32))))
  = { 0 };
