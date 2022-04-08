#!/usr/bin/python3
# Verify that certain symbols are covered by RELRO.
# Copyright (C) 2022 Free Software Foundation, Inc.
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

"""Analyze a (shared) object to verify that certain symbols are
present and covered by the PT_GNU_RELRO segment.

"""

import argparse
import os.path
import sys

# Make available glibc Python modules.
sys.path.append(os.path.join(
    os.path.dirname(os.path.realpath(__file__)), os.path.pardir, 'scripts'))

import glibcelf

def find_relro(path: str, img: glibcelf.ElfImage) -> (int, int):
    """Discover the address range of the PT_GNU_RELRO segment."""

    for phdr in img.phdrs():
        if phdr.p_type == glibcelf.ElfPt.PT_GNU_RELRO:
            return phdr.p_vaddr, phdr.p_vaddr + phdr.p_memsz
    sys.stdout.write('{}: error: no PT_GNU_RELRO segment\n'.format(path))

def get_parser():
    """Return an argument parser for this script."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('object', help='path to object file to check')
    parser.add_argument('symbols', help='name of the symbol to move',
                        nargs='+')
    return parser

def main(argv):
    """The main entry point."""
    parser = get_parser()
    opts = parser.parse_args(argv)
    with open(opts.object, 'rb') as inp:
        img = glibcelf.ElfImage(memoryview(inp.read()))

    check_symbols = frozenset([sym.encode('UTF-8') for sym in opts.symbols])

    # Tracks the symbols in check_symbols that have been found.
    symbols_found = set()

    # Discover the extent of the RELRO segment.
    relro_begin, relro_end = find_relro(opts.object, img)
    symbol_table_found = False

    errors = False
    def error(msg: str) -> None:
        """Record an error condition and write a message to standard output."""
        nonlocal errors
        errors = True
        sys.stdout.write('{}: error: {}\n'.format(opts.object, msg))

    # Iterate over section headers to find the symbol table.
    for shdr in img.shdrs():
        if shdr.sh_type == glibcelf.ElfSht.SHT_SYMTAB:
            symbol_table_found = True
            for sym in img.syms(shdr):
                symbol_name = img.symbol_name(shdr, sym)
                if symbol_name in check_symbols:
                    symbols_found.add(symbol_name)

                    # Validate symbol type, section, and size.
                    if sym.st_info.type != glibcelf.ElfStt.STT_OBJECT:
                        error('symbol {!r} has wrong type {}'.format(
                            symbol_name.decode('UTF-8'), sym.st_info.type))
                    if sym.st_shndx in glibcelf.ElfShn:
                        error('symbol {!r} has reserved section {}'.format(
                            symbol_name.decode('UTF-8'), sym.st_shndx))
                        continue
                    if sym.st_size == 0:
                        error('symbol {!r} has size zero'.format(
                            symbol_name.decode('UTF-8')))
                        continue

                    # Compute the extent of the symbol
                    symbol_start = sym.st_value
                    symbol_end = symbol_start + sym.st_size - 1

                    # Check if the symbol completely falls within the
                    # RELRO segment.
                    if not (relro_begin <= symbol_start < symbol_end
                            < relro_end):
                        error(
                            'symbol {!r} of size {} at 0x{:x} is not in RELRO range [0x{:x}, 0x{:x})'.format(
                                symbol_name.decode('UTF-8'),
                                sym.st_size, sym.st_value,
                                relro_begin, relro_end))

    if symbols_found != check_symbols:
        for sym in sorted(check_symbols - symbols_found):
            error('symbol {!r} not found'.format(sym.decode('UTF-8')))

    if errors:
        sys.exit(1)

    if not symbol_table_found:
        sys.stdout.write(
            '{}: warning: no symbol table found (stripped object)\n'.format(
                opts.object))
        sys.exit(77)

if __name__ == '__main__':
    main(sys.argv[1:])
