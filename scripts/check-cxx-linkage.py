#!/usr/bin/python3
# Check that installed headers declare functions with C language linkage.
# Copyright (C) 2026 Free Software Foundation, Inc.
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

"""Check that the functions declared by the installed headers have C
language linkage when the headers are used from C++.

Each header is compiled as C++ with the GCC -fdump-lang-raw option,
and the function declarations in the tree dump are checked.  The test
is unsupported if the compiler does not provide the option.
"""

import argparse
import os
import re
import shlex
import subprocess
import sys
import tempfile

# Headers that check-installed-headers.sh also skips.
SKIP_HEADERS = re.compile(r'^(bits/.*|regexp\.h|rpcsvc/.*\.x|sys/elf\.h'
                          r'|finclude/.*)$')

# Functions with C++ language linkage declared by headers not provided
# by glibc, as (source file name, function name).
ALLOWED = {
    # <linux/soundcard.h>, included by <sys/soundcard.h>.
    ('soundcard.h', 'seqbuf_dump'),
}

# Headers installed by the hurd and mach subdirectories, whose functions
# may all have C++ language linkage.  They include the MIG generated RPC
# stubs and the GNU Mach headers, none of which declare C language
# linkage.
ALLOWED_HEADERS = re.compile(r'^((hurd|mach|device)/.*|hurd\.h'
                             r'|faultexc_server\.h|mach\.h|mach_init\.h'
                             r'|mach_error\.h|mach-shortcuts\.h'
                             r'|lock-intern\.h|spin-lock\.h|machine-sp\.h)$')


# The -fdump-lang-raw output lists one tree node per entry.  An entry
# starts with '@ID KIND' followed by 'NAME: VALUE' fields, which may
# continue on the following indented lines.  For instance:
#
#   @29     function_decl    name: @44      type: @45      scpe: @3
#                            srcp: pidfd.h:120             chain: @46
#                            args: @47      body: undefined
#                            link: extern
#   @44     identifier_node  strg: pidfd_open              lngt: 10
#
# NODE_RE matches the first line of an entry, FIELD_RE each field
# (whose value ends at the first space), and STRG_RE the string of an
# identifier_node, which may contain spaces (such as 'long int').
NODE_RE = re.compile(r'^@(\d+)\s+(\S+)\s*(.*)$')
FIELD_RE = re.compile(r'\b(\w+): (\S+)')
STRG_RE = re.compile(r'strg: (.*?)\s+lngt:')


class Node:
    def __init__(self, kind, text):
        self.kind = kind
        self.text = text
        self.fields = {}
        for name, value in FIELD_RE.findall(text):
            # Some fields, such as body, may appear more than once.
            self.fields.setdefault(name, []).append(value)

    def field(self, name):
        values = self.fields.get(name)
        return values[0] if values else None


def compile_dump(cxx, source, dump):
    """Compile SOURCE with the CXX command, dumping its tree to DUMP.
    Return the compiler output if the compilation fails, or None."""
    if os.path.exists(dump):
        os.unlink(dump)
    cmd = cxx + ['-fsyntax-only', '-fdump-lang-raw=' + dump, '-x', 'c++', '-']
    proc = subprocess.Popen(cmd, stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            universal_newlines=True)
    output = proc.communicate(source)[0]
    if proc.returncode != 0 or not os.path.exists(dump):
        return output
    return None


def parse_dump(dump):
    """Return the dump nodes as a dict of '@ID' -> Node, joining each node's
    continuation lines."""
    texts = {}
    cur = None
    with open(dump, errors='replace') as f:
        # Group the lines into one string per node, for instance the dump:
        #
        # @29     function_decl    name: @44      type: @45      scpe: @3
        #                          srcp: pidfd.h:120             chain: @46
        #                          args: @47      body: undefined
        #                          link: extern
        #
        # texts['@29'][1] holds the whole node on one line:
        #
        #   name: @44 type: @45 scpe: @3 srcp: pidfd.h:120 chain: @46 args: ...
        for line in f:
            m = NODE_RE.match(line)
            if m:
                cur = '@' + m.group(1)
                texts[cur] = [m.group(2), m.group(3)]
            elif cur is not None:
                texts[cur][1] += ' ' + line.strip()
    # The Node constructor runs FIELD_RE.findall on the joined text and stores
    # each field in a list (some fields appear more than once).  With the above
    # example:
    #
    # nodes['@29'].kind            - 'function_decl'
    # nodes['@29'].field('scpe')   - '@3'
    # nodes['@29'].field('srcp')   - 'pidfd.h:120'
    # nodes['@29'].fields['body']  - ['undefined']  (an inline function
    #                                also has '@N')
    # nodes['@29'].text            - the full joined string
    return {ref: Node(kind, text) for ref, (kind, text) in texts.items()}


def cxx_linkage_functions(nodes):
    def identifier(ref):
        ident = nodes.get(ref)
        if ident is None or ident.kind != 'identifier_node':
            return None
        m = STRG_RE.search(ident.text)
        return m.group(1) if m else None

    for node in nodes.values():
        # Variables don't matter here, in the Itanium C++ ABI, global variables
        # aren't mangled, so their linkage doesn't affect linking.
        if node.kind != 'function_decl':
            continue
        # Functions with internal linkage never need an external symbol. The
        # 'lang' equal 'C' is the core test: GCC writes 'lang' C only for
        # functions declared inside extern "C" (__BEGIN_DECLS).
        if node.field('link') != 'extern' or node.field('lang') == 'C':
            continue
        # Only functions in the global namespace.
        scope = nodes.get(node.field('scpe'))
        if scope is None or scope.kind != 'translation_unit_decl':
            continue
        # Inline definitions and templates do not require a symbol.
        if any(v.startswith('@') for v in node.fields.get('body', [])):
            continue
        if 'pseudo tmpl' in node.text:
            continue
        # extern "C++" overloads with an asm label that refers to the C
        # symbol, such as the const-correct strchr.
        mangled = identifier(node.field('mngl'))
        if mangled is not None and not mangled.startswith('_Z'):
            continue
        location = node.field('srcp') or '?'
        if location.startswith('<built-in>'):
            continue
        name = identifier(node.field('name')) or '?'
        if (location.split(':')[0], name) in ALLOWED:
            continue
        yield name, location


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cxx', required=True,
                        help='C++ compiler command, including options')
    parser.add_argument('--fortify', type=int, default=0,
                        help='also check with this _FORTIFY_SOURCE level')
    parser.add_argument('headers', nargs='*', help='headers to check')
    args = parser.parse_args()
    cxx = shlex.split(args.cxx)

    modes = ['']
    if args.fortify > 0:
        modes.append('#define _FORTIFY_SOURCE %d\n' % args.fortify)

    failed = False
    with tempfile.TemporaryDirectory() as tmpdir:
        dump = os.path.join(tmpdir, 'dump.raw')
        if compile_dump(cxx, '', dump) is not None:
            print('compiler does not support -fdump-lang-raw')
            sys.exit(77)
        for header in args.headers:
            if SKIP_HEADERS.match(header) or ALLOWED_HEADERS.match(header):
                continue
            functions = set()
            for mode in modes:
                source = ('#undef _LIBC\n'
                          '#undef _GNU_SOURCE\n'
                          '#undef _FORTIFY_SOURCE\n'
                          '#define _GNU_SOURCE 1\n'
                          '%s#include <%s>\n' % (mode, header))
                if compile_dump(cxx, source, dump) is not None:
                    # check-installed-headers.sh reports compilation
                    # failures.
                    print('%s: skipped, compilation failed' % header)
                    break
                functions.update(cxx_linkage_functions(parse_dump(dump)))
            for name, location in sorted(functions):
                print('%s: %s (%s) does not have C language linkage'
                      % (header, name, location))
                failed = True
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
