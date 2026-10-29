#!/usr/bin/python3
# Verify that the .eh_frame section ends with a zero terminator.
# Copyright The GNU Toolchain Authors.
# This file is part of the GNU C Library.
#
# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>.

"""Verify that the .eh_frame section of a shared object ends with a
zero terminator.

PT_GNU_EH_FRAME only gives the start of .eh_frame, so unwinders that
scan .eh_frame linearly rely on the terminator to find its end.

"""

import os.path
import sys

# Make available glibc Python modules.
sys.path.append(os.path.join(
    os.path.dirname(os.path.realpath(__file__)), os.path.pardir, 'scripts'))

import glibcelf

def main(argv):
    path, = argv
    img = glibcelf.Image.readfile(path)
    if img.ehdr.e_ident.ei_data == glibcelf.ElfData.ELFDATA2LSB:
        byteorder = 'little'
    else:
        byteorder = 'big'

    for shdr in img.shdrs():
        if shdr.sh_name == b'.eh_frame':
            break
    else:
        sys.stdout.write('{}: error: no .eh_frame section\n'.format(path))
        sys.exit(1)

    # Skip over the CIE and FDE records using their length fields, as
    # a linear scan by an unwinder does, until a zero length.
    data = img.image[shdr.sh_offset:shdr.sh_offset + shdr.sh_size]
    offset = 0
    while offset + 4 <= len(data):
        length = int.from_bytes(data[offset:offset + 4], byteorder)
        if length == 0:
            break
        if length == 0xffffffff:
            # 64-bit DWARF: the actual length follows in 8 bytes.
            length = 8 + int.from_bytes(data[offset + 4:offset + 12],
                                        byteorder)
        offset += 4 + length

    if offset + 4 != len(data):
        sys.stdout.write('{}: error: .eh_frame of size 0x{:x} does not end'
                         ' with a zero terminator\n'.format(path, len(data)))
        sys.exit(1)

if __name__ == '__main__':
    main(sys.argv[1:])
