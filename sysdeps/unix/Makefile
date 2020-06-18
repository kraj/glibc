# Copyright (C) 1991-2020 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>.

ifndef inhibit-unix-syscalls

# Sysdep dirs unix/... can contain a file syscalls.list,
# which specifies objects to be compiled as simple Unix system calls.

-include $(common-objpfx)sysd-syscalls-c

ifeq (misc,$(subdir))
sysdep_routines += $(unix-extra-syscalls)
endif

compile-syscall-c = $(COMPILE.c) -o $@ $(compile-mkdep-flags)

ifndef avoid-generated
arch-syscall-h = $(firstword $(wildcard $(addsuffix /arch-syscall.h,$(sysdirs) .)))

$(common-objpfx)sysd-syscalls-c: $(..)sysdeps/unix/gen-syscalls.py  \
			         $(wildcard $(+sysdep_dirs:%=%/syscalls.def)) \
			         $(wildcard $(+sysdep_dirs:%=%/arch-syscall.h)) \
			         $(common-objpfx)libc-modules.stmp
	$(PYTHON) $(dir $<)$(notdir $<) $(arch-syscall-h) $(common-objpfx)misc \
					$(sysdirs) > $@T
	mv -f $@T $@
endif

postclean-generated += sysd-syscalls-c

endif
