#!/usr/bin/python3

import argparse
import os
import re
from enum import Enum
from collections import OrderedDict

# File name with syscalls definitions.
SYSLIST_NAME = 'syscalls.def'
# Default header to include.
DEFAULT_HDRS = ['sysdep.h']
# Header to include for compat alias creation.
SHLIB_HDR    = 'shlib-compat.h'

SYSCALL_ENTRY_RE = re.compile(r'\[[^\]]*\][:\S+]*|\S+')

ALIAS_PREFIX = '__'
REDIR_PREFIX = '__redirect_'

DEFAULT_RETTYPE = 'int'

SYSCALL_LIBRARIES = ['libc', 'librt', 'libpthread']
SYSCALL_LIBRARIES_IS_IN = \
  ' || '.join(map(lambda x: 'IS_IN({})'.format(x), SYSCALL_LIBRARIES))

STUB_SYSCALL_IMPL = 'stub-syscalls-c'

class Alias(object):
  class Type(Enum):
    DEFAULT=1
    COMPAT=2
    STRONG=3
    HIDDEN=4
    WEAK=5

  compat_counter = -1

  def __counter(self):
    Alias.compat_counter += 1
    return Alias.compat_counter

  def __init__(self, alias):
    if '@@' in alias:
      self.name, self.ver = alias.split('@@')
      self.type = Alias.Type.DEFAULT
    elif '@' in alias:
      sym, self.obsolete = alias.split(':')
      self.name, self.ver = sym.split('@')
      self.type = Alias.Type.COMPAT
    elif alias.startswith('!'):
      self.name = alias[1:]
      self.type = Alias.Type.STRONG
    elif alias.startswith('$'):
      self.name = alias[1:]
      self.type = Alias.Type.HIDDEN
    else:
      self.name = alias
      self.type = Alias.Type.WEAK

  def basename(self):
    if self.name.startswith(ALIAS_PREFIX):
      return self.name[len(ALIAS_PREFIX):]
    return self.name

  def str(self, funcname):
    if self.type == Alias.Type.DEFAULT:
      # The default_symbol_version emits a strong_alias for !SHARED and it
      # cause linkspace issues.
      return '#if {3}\n' \
             'versioned_symbol (libc, {0}, {1}, {2});\n' \
             '#else\n' \
             'weak_alias ({0}, {1});\n' \
             '#endif\n' \
             .format(funcname,
                     self.name,
                     self.ver.replace('.', '_'),
                     SYSCALL_LIBRARIES_IS_IN)
    elif self.type == Alias.Type.COMPAT:
      talias = '{0}{1}'.format(funcname, self.__counter())
      salias = 'strong_alias ({0}, {1});\n' \
                .format(funcname,
                        talias)
      return '#if SHLIB_COMPAT (libc, {0}, {1})\n' \
             '{4}' \
             'compat_symbol (libc, {2}, {3}, {0});\n' \
             '#endif\n' \
             .format(self.ver.replace('.', '_'),
                     self.obsolete.replace('.', '_'),
                     talias,
                     self.name,
                     salias)
    elif self.type == Alias.Type.STRONG:
      return 'strong_alias ({0}, {1});\n' \
             .format(funcname,
                     self.name)
    elif self.type == Alias.Type.HIDDEN:
      return '#if {1}\n' \
             'hidden_def ({0});\n' \
             '#endif\n' \
             .format(self.name,
                     SYSCALL_LIBRARIES_IS_IN)
    elif self.type == Alias.Type.WEAK:
      return 'weak_alias ({0}, {1});\n' \
             .format(funcname, self.name)


def parse_arguments(args):
  def read_list_arguments(entry):
    # Remove empty entries and strip each element
    return list(map(lambda x: x.strip(), filter(None, entry[1:len(entry)-1].split(','))))

  if ':' in args:
    args,rettype = args.split(':')
    return read_list_arguments(args),rettype
  else:
    return read_list_arguments(args),DEFAULT_RETTYPE

def parse_list(args):
  return filter(lambda x: x != '-', args.split(','))

class Syscall(object):
  def __init__(self, fields, alt_source):
    self.filename   = fields[0]
    self.sysname    = fields[1]
    self.symbolname = Alias(fields[3])
    self.aliases    = [Alias(entry) for entry in parse_list(fields[4])]
    self.fields     = fields
    self.flags      = '' if len(fields) <= 6 else fields[6]
    self.alt_source = alt_source

  def extra(self):
    return 'E' in self.flags

  def generate_implementation(self, outputdir):
    args,rettype = parse_arguments(self.fields[2])
    includes     = list(map(lambda x: '#include <{}>'.format(x),
                            list(parse_list(self.fields[5])) + DEFAULT_HDRS))

    # Add the compat header if the alias requires it.
    for alias in self.aliases:
      if alias.type == Alias.Type.DEFAULT or alias.type == Alias.Type.COMPAT:
        includes.append('#include <{}>'.format(SHLIB_HDR))
        break

    # Alias creation requires to rename the symbol to an internal prefixed name
    # and add an extra weak alias.
    if len (self.aliases) > 0:
      funcname = '{0}{1}'.format(ALIAS_PREFIX, self.symbolname.name)

      def check_duplicate_alias():
        for alias in self.aliases:
          # Default version alias already creates the requires alias.
          if alias.name == self.symbolname.name \
             and alias.type == Alias.Type.DEFAULT:
            return True
          if alias.name == self.symbolname.name \
            and alias.type == Alias.Type.WEAK:
            return True
        return False

      if not check_duplicate_alias():
        self.aliases.append(Alias(self.symbolname.name))

      # Remove duplicated weak alias.
      self.aliases = list(filter(lambda x: not (x.name == funcname \
                                           and x.type == Alias.Type.WEAK),
                          self.aliases))
    else:
      funcname = self.symbolname.name

    # Add the libc_hidden_def for the strong alias.
    if self.symbolname.type == Alias.Type.HIDDEN:
      self.aliases.append(self.symbolname)

    # Handle alias with potentially different argument types but where it is
    # valid.  For instance, LFS symbols alias to default name counterparts
    # for ABIs where off_t and off64_t are the same size.
    for alias in self.aliases:
      if alias.basename() != self.symbolname.basename():
         includes.insert(0, '#define {0} {1}{0}'
                            .format(alias.name, REDIR_PREFIX))
         includes.append('#undef {0}'.format(alias.name))

    with open(os.path.join(outputdir, self.filename + '.c'), 'w+') as f:
      for inc in includes:
         f.write('{}\n'.format(inc))

      funcargs = ', '.join(['{} arg{}'.format(args[i], i)
                           for i in range(0, len(args))])
      syscargs = ', '.join(['arg{}'.format(i)
                            for i in range(0, len(args))])

      if rettype == DEFAULT_RETTYPE:
        retstr = 'return '
      elif rettype == 'void':
        retstr = ''
      else:
        retstr = 'return ({}) '.format(rettype)

      f.write('\n{0} {1} ({2})\n'
              '{{\n'
              '  {3}inline_syscall (__NR_{4}{5});\n'
              '}}\n'
              .format(rettype,
                      funcname,
                      funcargs if funcargs else 'void',
                      retstr,
                      self.sysname,
                      ', {}'.format(syscargs) if syscargs else ''))

      for alias in self.aliases:
        f.write(alias.str(funcname))

  def print_makefile_rule(self, create, builddir):
    print('#### CALL={0} SOURCE={1}'.format(self.filename, self.alt_source))
    if create:
      extra_syscall = 'unix-extra-syscalls += {0}\n'.format(self.filename) \
                       if 'E' in self.flags else ''
      print('#### ENTRY={0}\n'
            'ifeq (,$(filter {1},$(unix-syscalls)))\n'
            'unix-syscalls += {1}\n'
            '{2}'
            '$(foreach p,$(sysd-rules-targets),$(foreach o, $(object-suffixes),'
              '$(objpfx)$(patsubst %,$p,{1})$o)): \\\n'
            '\t\t$(..)sysdeps/unix/gen-syscalls.py\n'
            '\t$(make-target-directory)\n'
            '\t$(compile-syscall-c) $(common-objpfx){3}/{1}.c\n'
            'endif'
            .format('  '.join(self.fields),
                    self.filename,
                    extra_syscall,
                    builddir))
    print()


class ArchSyscall(object):
  ARCH_SYSCALL_ENTRY_RE = re.compile(r'__NR_\S+')
  def __init__(self, arch_syscall):
    with open(arch_syscall, 'r') as fin:
      self.syscalls = self.ARCH_SYSCALL_ENTRY_RE.findall(fin.read())

  def is_defined(self, syscall):
    return '__NR_' + syscall in self.syscalls


# Check if there is an override source file for syscall in the SYSDEPS paths.
# Return the found source path if found or None otherwise.
def find_alternative_source(syscall, sysdeps):
  for sysdep in sysdeps:
    for f in [(lambda x: os.path.join(sysdep[0], syscall + '.' + ext))(ext) \
                         for ext in ['c', 'S']]:
      if os.path.isfile(f):
        return f
  return None


def create_stub_syscall(stubs, outputdir):
  if len(stubs) == 0:
    return

  with open(os.path.join(outputdir, STUB_SYSCALL_IMPL + '.c'), 'w+') as f:
    for stub in stubs:
      f.write('#define {0} {1}{0}\n'
              .format(stub.symbolname.name,
                      REDIR_PREFIX))
    for include in DEFAULT_HDRS + [SHLIB_HDR]:
      f.write('#include <{0}>\n'.format(include))
    for stub in stubs:
      f.write('#undef {0}\n'
              .format(stub.symbolname.name))

    funcname = '__no_syscall'
    f.write('{0} {1} (void)\n' \
            '{{\n' \
            '  __set_errno (ENOSYS);\n' \
            '  return -1;\n'
            '}}\n'
            .format(DEFAULT_RETTYPE,
                   funcname))

    for alias in (alias for alias in (stub.aliases for stub in stubs)):
      f.write(alias[0].str(funcname))

    print('unix-extra-syscalls += {}\n'.format(STUB_SYSCALL_IMPL))


def main():
  parser = argparse.ArgumentParser(description='Generate unix syscalls.')
  parser.add_argument('arch_syscall',
                      help='arch-syscall.h header')
  parser.add_argument('output',
                      help='output directory')
  parser.add_argument('sysdeps',
                      help='sysdeps folders which might contain a syscall'
                           'description file',
                      nargs='*');
  args = parser.parse_args()

  archsyscalls = ArchSyscall(args.arch_syscall)

  sysdeps = []
  for s in args.sysdeps:
    sysentry = os.path.join(s, SYSLIST_NAME)
    sysdeps.append((s, os.path.isfile (sysentry), {}))

  syscalls = OrderedDict()
  for i, s in enumerate(sysdeps):
    if s[1]:
      with open(os.path.join(s[0], SYSLIST_NAME)) as fin:
        for line in fin:
          if line.lstrip().startswith('#') or not line.strip():
            continue
          fields = SYSCALL_ENTRY_RE.findall(line)
          if fields[0] in syscalls:
            continue
          alt_source = find_alternative_source(fields[0], [s] + sysdeps[:i])
          syscalls[fields[0]] = Syscall(fields, alt_source)

  if not os.path.exists(args.output):
    os.makedirs(args.output)

  stub_syscalls = []

  builddir = os.path.basename(args.output)
  for name, syscall in syscalls.items():
     create_rule = False
     if not syscall.alt_source:
       if archsyscalls.is_defined(syscall.sysname):
         syscall.generate_implementation(args.output)
         create_rule = True
       elif syscall.extra():
         # The extra syscalls that does not have an alternative implementation
         # and also not a correspondent syscall number are aliases to a symbol
         # that returns -1 and set ENOSYS.
         stub_syscalls.append(syscall)

     syscall.print_makefile_rule(create_rule, builddir)

  create_stub_syscall(stub_syscalls, args.output)

if __name__ == '__main__':
  main()
