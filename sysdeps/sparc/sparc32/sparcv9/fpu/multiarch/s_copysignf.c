#include <math.h>
#include <sparc-ifunc.h>

extern __typeof (copysignf) __copysignf_vis3 attribute_hidden;
extern __typeof (copysignf) __copysignf_generic attribute_hidden;

sparc_libm_ifunc (__copysignf,
		  hwcap & HWCAP_SPARC_VIS3
		  ? __copysignf_vis3 
		  : __copysignf_generic);
weak_alias (__copysignf, copysignf)
