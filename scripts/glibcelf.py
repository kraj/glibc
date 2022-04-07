#!/usr/bin/python3
# ELF support functionality for Python.
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

import collections
import enum
import struct

class OpenIntEnum(enum.IntEnum):
    "Integer enumeration that supports arbitrary int values."
    @classmethod
    def _missing_(cls, value):
        # See enum.IntFlag._create_pseudo_member_.  This allows
        # creating of enum constants with arbitrary integer values.
        pseudo_member = int.__new__(cls, value)
        pseudo_member._name_ = None
        pseudo_member._value_ = value
        return pseudo_member

    def __repr__(self):
        name = self._name_
        if name is not None:
            # The names have prefixes like SHT_, implying their type.
            return name
        return '{}({})'.format(self.__class__.__name__, self._value_)

    def __str__(self):
        name = self._name_
        if name is not None:
            return name
        return str(self._value_)

class ElfClass(OpenIntEnum):
    "ELF word size.  Type of EI_CLASS values."
    ELFCLASSNONE = 0
    ELFCLASS32 = 1
    ELFCLASS64 = 2

class ElfData(OpenIntEnum):
    "ELF endianess.  Type of EI_DATA values."
    ELFDATANONE = 0
    ELFDATA2LSB = 1
    ELFDATA2MSB = 2

class ElfMachine(OpenIntEnum):
    "ELF machine type.  Type of values in ElfEhdr.e_machine field."
    EM_NONE = 0
    EM_M32 = 1
    EM_SPARC = 2
    EM_386 = 3
    EM_68K = 4
    EM_88K = 5
    EM_IAMCU = 6
    EM_860 = 7
    EM_MIPS = 8
    EM_S370 = 9
    EM_MIPS_RS3_LE = 10
    EM_PARISC = 15
    EM_VPP500 = 17
    EM_SPARC32PLUS = 18
    EM_960 = 19
    EM_PPC = 20
    EM_PPC64 = 21
    EM_S390 = 22
    EM_SPU = 23
    EM_V800 = 36
    EM_FR20 = 37
    EM_RH32 = 38
    EM_RCE = 39
    EM_ARM = 40
    EM_FAKE_ALPHA = 41
    EM_SH = 42
    EM_SPARCV9 = 43
    EM_TRICORE = 44
    EM_ARC = 45
    EM_H8_300 = 46
    EM_H8_300H = 47
    EM_H8S = 48
    EM_H8_500 = 49
    EM_IA_64 = 50
    EM_MIPS_X = 51
    EM_COLDFIRE = 52
    EM_68HC12 = 53
    EM_MMA = 54
    EM_PCP = 55
    EM_NCPU = 56
    EM_NDR1 = 57
    EM_STARCORE = 58
    EM_ME16 = 59
    EM_ST100 = 60
    EM_TINYJ = 61
    EM_X86_64 = 62
    EM_PDSP = 63
    EM_PDP10 = 64
    EM_PDP11 = 65
    EM_FX66 = 66
    EM_ST9PLUS = 67
    EM_ST7 = 68
    EM_68HC16 = 69
    EM_68HC11 = 70
    EM_68HC08 = 71
    EM_68HC05 = 72
    EM_SVX = 73
    EM_ST19 = 74
    EM_VAX = 75
    EM_CRIS = 76
    EM_JAVELIN = 77
    EM_FIREPATH = 78
    EM_ZSP = 79
    EM_MMIX = 80
    EM_HUANY = 81
    EM_PRISM = 82
    EM_AVR = 83
    EM_FR30 = 84
    EM_D10V = 85
    EM_D30V = 86
    EM_V850 = 87
    EM_M32R = 88
    EM_MN10300 = 89
    EM_MN10200 = 90
    EM_PJ = 91
    EM_OPENRISC = 92
    EM_ARC_COMPACT = 93
    EM_XTENSA = 94
    EM_VIDEOCORE = 95
    EM_TMM_GPP = 96
    EM_NS32K = 97
    EM_TPC = 98
    EM_SNP1K = 99
    EM_ST200 = 100
    EM_IP2K = 101
    EM_MAX = 102
    EM_CR = 103
    EM_F2MC16 = 104
    EM_MSP430 = 105
    EM_BLACKFIN = 106
    EM_SE_C33 = 107
    EM_SEP = 108
    EM_ARCA = 109
    EM_UNICORE = 110
    EM_EXCESS = 111
    EM_DXP = 112
    EM_ALTERA_NIOS2 = 113
    EM_CRX = 114
    EM_XGATE = 115
    EM_C166 = 116
    EM_M16C = 117
    EM_DSPIC30F = 118
    EM_CE = 119
    EM_M32C = 120
    EM_TSK3000 = 131
    EM_RS08 = 132
    EM_SHARC = 133
    EM_ECOG2 = 134
    EM_SCORE7 = 135
    EM_DSP24 = 136
    EM_VIDEOCORE3 = 137
    EM_LATTICEMICO32 = 138
    EM_SE_C17 = 139
    EM_TI_C6000 = 140
    EM_TI_C2000 = 141
    EM_TI_C5500 = 142
    EM_TI_ARP32 = 143
    EM_TI_PRU = 144
    EM_MMDSP_PLUS = 160
    EM_CYPRESS_M8C = 161
    EM_R32C = 162
    EM_TRIMEDIA = 163
    EM_QDSP6 = 164
    EM_8051 = 165
    EM_STXP7X = 166
    EM_NDS32 = 167
    EM_ECOG1X = 168
    EM_MAXQ30 = 169
    EM_XIMO16 = 170
    EM_MANIK = 171
    EM_CRAYNV2 = 172
    EM_RX = 173
    EM_METAG = 174
    EM_MCST_ELBRUS = 175
    EM_ECOG16 = 176
    EM_CR16 = 177
    EM_ETPU = 178
    EM_SLE9X = 179
    EM_L10M = 180
    EM_K10M = 181
    EM_AARCH64 = 183
    EM_AVR32 = 185
    EM_STM8 = 186
    EM_TILE64 = 187
    EM_TILEPRO = 188
    EM_MICROBLAZE = 189
    EM_CUDA = 190
    EM_TILEGX = 191
    EM_CLOUDSHIELD = 192
    EM_COREA_1ST = 193
    EM_COREA_2ND = 194
    EM_ARCV2 = 195
    EM_OPEN8 = 196
    EM_RL78 = 197
    EM_VIDEOCORE5 = 198
    EM_78KOR = 199
    EM_56800EX = 200
    EM_BA1 = 201
    EM_BA2 = 202
    EM_XCORE = 203
    EM_MCHP_PIC = 204
    EM_INTELGT = 205
    EM_KM32 = 210
    EM_KMX32 = 211
    EM_EMX16 = 212
    EM_EMX8 = 213
    EM_KVARC = 214
    EM_CDP = 215
    EM_COGE = 216
    EM_COOL = 217
    EM_NORC = 218
    EM_CSR_KALIMBA = 219
    EM_Z80 = 220
    EM_VISIUM = 221
    EM_FT32 = 222
    EM_MOXIE = 223
    EM_AMDGPU = 224
    EM_RISCV = 243
    EM_BPF = 247
    EM_CSKY = 252
    EM_NUM = 253
    EM_ALPHA = 0x9026

class ElfEt(OpenIntEnum):
    "ELF file type.  Type of ET_* values and the Ehdr.e_type field."
    ET_NONE = 0
    ET_REL = 1
    ET_EXEC = 2
    ET_DYN = 3
    ET_CORE = 4

class ElfShn(OpenIntEnum):
    "ELF reserved section indices."
    SHN_UNDEF = 0
    SHN_ABS = 0xfff1
    SHN_COMMON = 0xfff2
    SHN_XINDEX = 0xffff

class ElfSht(OpenIntEnum):
    "ELF section types.  Type of SHT_* values."
    SHT_NULL = 0
    SHT_PROGBITS = 1
    SHT_SYMTAB = 2
    SHT_STRTAB = 3
    SHT_RELA = 4
    SHT_HASH = 5
    SHT_DYNAMIC = 6
    SHT_NOTE = 7
    SHT_NOBITS = 8
    SHT_REL = 9
    SHT_DYNSYM = 11
    SHT_INIT_ARRAY = 14
    SHT_FINI_ARRAY = 15
    SHT_PREINIT_ARRAY = 16
    SHT_GROUP = 17
    SHT_SYMTAB_SHNDX = 18
    SHT_GNU_ATTRIBUTES = 0x6ffffff5
    SHT_GNU_HASH = 0x6ffffff6
    SHT_GNU_LIBLIST = 0x6ffffff7
    SHT_CHECKSUM = 0x6ffffff8
    SHT_GNU_verdef = 0x6ffffffd
    SHT_GNU_verneed = 0x6ffffffe
    SHT_GNU_versym = 0x6fffffff

class ElfPf(enum.IntFlag):
    "Program header flags.  Type of ElfPhdr.p_flags values."
    PF_X = 1
    PF_W = 2
    PF_R = 4

class ElfShf(enum.IntFlag):
    "Section flags.  Type of ElfShdr.sh_type values."
    SHF_WRITE = 1 << 0
    SHF_ALLOC = 1 << 1
    SHF_EXECINSTR = 1 << 2
    SHF_MERGE = 1 << 4
    SHF_STRINGS = 1 << 5
    SHF_INFO_LINK = 1 << 6
    SHF_LINK_ORDER = 1 << 7
    SHF_OS_NONCONFORMING = 256
    SHF_GROUP = 1 << 9
    SHF_TLS = 1 << 10
    SHF_COMPRESSED = 1 << 11
    SHF_GNU_RETAIN = 1 << 21
    SHF_ORDERED = 1 << 30
    SHF_RETAIN = 1 << 31

class ElfStb(OpenIntEnum):
    "ELF symbol binding type."
    STB_LOCAL = 0
    STB_GLOBAL = 1
    STB_WEAK = 3
    STB_GNU_UNIQUE = 10

class ElfStt(OpenIntEnum):
    "ELF symbol type."
    STT_NOTYPE = 0
    STT_OBJECT = 1
    STT_FUNC = 2
    STT_SECTION = 3
    STT_FILE = 4
    STT_COMMON = 5
    STT_TLS = 6
    STT_GNU_IFUNC = 10

class ElfStInfo:
    "ELF symbol binding and type.  Type of the ElfSym.st_info field."
    def __init__(self, arg0, arg1=None):
        if type(arg0) is int and arg1 is None:
            self.bind = ElfStb(arg0 >> 4)
            self.type = ElfStt(arg0 & 15)
        else:
            self.bind = ElfStb(arg0)
            self.type = ElfStt(arg1)

    def value(self):
        return (self.bind.value() << 4) | (self.type.value())

def _define_variants(baseclass: type, layout32: str, layout64: str,
                     types: dict[str, type] | None=None,
                     fields32: tuple[str] | None=None):
    struct32 = struct.Struct(layout32)
    struct64 = struct.Struct(layout32)

    # Check that the struct formats yield the right number of components.
    for s in (struct32, struct64):
        example = s.unpack(b' ' * s.size)
        if len(example) != len(baseclass._fields):
            raise ValueError('{!r} yields wrong field count: {} != {}'.format(
                s.format, len(example),  len(baseclass._fields)))

    # Check that field names in types are correct.
    if types is None:
        types = ()
    for n in types:
        if n not in baseclass._fields:
            raise ValueError('{} does not have field {!r}'.format(
                baseclass.__name__, n))

    if fields32 is not None \
       and set(fields32) != set(baseclass._fields):
        raise ValueError('{!r} is not a permutation of the fields {!r}'.format(
            fields32, baseclass._fields))

    def unique_name(name, used_names = (set((baseclass.__name__,))
                                        | set(baseclass._fields)
                                        | {n.__name__
                                           for n in (types or {}).values()})):
        "Find a name that is not used for a class or field name."
        candidate = name
        n = 0
        while candidate in used_names:
            n += 1
            candidate = '{}{}'.format(name, n)
        used_names.add(candidate)
        return candidate
    blob_name = unique_name('blob')
    struct_unpack_name = unique_name('struct_unpack')
    comps_name = unique_name('comps')

    classes = {}
    for (bits, elfclass, layout, fields) in (
            (32, ElfClass.ELFCLASS32, layout32, fields32),
            (64, ElfClass.ELFCLASS64, layout64, None),
    ):
        for (elfdata, structprefix, classsuffix) in (
                (ElfData.ELFDATA2LSB, '<', 'LE'),
                (ElfData.ELFDATA2MSB, '>', 'BE'),
        ):
            env = {
                baseclass.__name__: baseclass,
                struct_unpack_name: struct.unpack,
            }

            # Add the type converters.
            if types:
                for cls in types.values():
                    env[cls.__name__] = cls

            classname = '{}{}{}'.format(baseclass.__name__, bits, classsuffix)

            code = '''
class {classname}({baseclass}):
    @staticmethod
    def unpack({blob_name}):
'''.format(classname=classname, baseclass=baseclass.__name__,
           blob_name='blob')

            indent = ' ' * 8
            unpack_call = '{}({!r}, {})'.format(
                struct_unpack_name, layout, blob_name)
            field_names = ', '.join(baseclass._fields)
            if types is None and fields is None:
                code += '{}return {}({})\n'.format(
                    indent, baseclass.__name__, unpack_call)
            else:
                # Destructuring tuple assignment.
                if fields is None:
                    code += '{}{} = {}\n'.format(
                        indent, field_names, unpack_call)
                else:
                    # Use custom field order.
                    code += '{}{} = {}\n'.format(
                        indent, ', '.join(fields), unpack_call)

                # Perform the type conversions.
                for n in baseclass._fields:
                    if n in types:
                        code += '{}{} = {}({})\n'.format(
                            indent, n, types[n].__name__, n)
                # Create the named tuple.
                code += '{}return {}({})\n'.format(
                    indent, baseclass.__name__, field_names)

            print(code)
            exec(code, env)
            cls = env[classname]
            print(cls)
            cls.size = struct.calcsize(layout)
            classes[(elfclass, elfdata)] = cls
    baseclass.variants = classes


# Corresponds to EI_* indices into Elf*_Ehdr.e_indent.
class ElfIdent(collections.namedtuple('ElfIdent',
    'ei_mag ei_class ei_data ei_version ei_osabi ei_abiversion ei_pad')):

    def __new__(cls, *args):
        if len(args) == 1:
            return cls.unpack(args[0])
        return cls.__base__.__new__(cls, *args)

    @staticmethod
    def unpack(blob):
        ei_mag, ei_class, ei_data, ei_version, ei_osabi, ei_abiversion, \
            ei_pad = struct.unpack('4s5B7s', blob)
        return ElfIdent(ei_mag, ElfClass(ei_class), ElfData(ei_data),
                        ei_version, ei_osabi, ei_abiversion, ei_pad)
    size = 16

# Corresponds to Elf32_Ehdr and Elf64_Ehdr.
ElfEhdr = collections.namedtuple('ElfEhdr',
   'e_ident e_type e_machine e_version e_entry e_phoff e_shoff e_flags'
    + ' e_ehsize e_phentsize e_phnum e_shentsize e_shnum e_shstrndx')
_define_variants(ElfEhdr,
                 layout32='16s2H5I6H',
                 layout64='16s2HI3QI6H',
                 types=dict(e_ident=ElfIdent,
                            e_machine=ElfMachine,
                            e_type=ElfEt))

# Corresponds to Elf32_Phdr and Elf64_Pdhr.  Order follows the latter.
ElfPhdr = collections.namedtuple('ElfPhdr',
    'p_type p_flags p_offset p_vaddr p_paddr p_filesz p_memsz p_align')
_define_variants(ElfPhdr,
                 layout32='8I',
                 fields32=('p_type', 'p_offset', 'p_vaddr', 'p_paddr',
                           'p_filesz', 'p_memsz', 'p_flags', 'p_align'),
                 layout64='2I6Q',
                 types=dict(p_flags=ElfPf))


# Corresponds to Elf32_Shdr and Elf64_Shdr.
ElfShdr = collections.namedtuple('ElfShdr',
    'sh_name sh_type sh_flags sh_addr sh_offset sh_size sh_link sh_info'
    + ' sh_addralign sh_entsize')
_define_variants(ElfShdr,
                 layout32='10I',
                 layout64='2I4Q2I2Q',
                 types=dict(sh_flags=ElfShf))

# Corresponds to Elf32_Sym and Elf64_Sym.
ElfSym = collections.namedtuple('ElfSym',
    'st_name st_info st_other st_shndx st_value st_size')
_define_variants(ElfSym,
                 layout32='3I2BH',
                 layout64='Q2BH2Q',
                 fields32=('st_name', 'st_value', 'st_size', 'st_info',
                           'st_other', 'st_shndx'),
                 types=dict(st_shndx=ElfShn,
                            st_info=ElfStInfo))

# Corresponds to Elf32_Rel and Elf64_Rel.
ElfRel = collections.namedtuple('ElfRel', 'r_offset r_info')
_define_variants(ElfRel,
                 layout32='2I',
                 layout64='2Q')

# Corresponds to Elf32_Rel and Elf64_Rel.
ElfRela = collections.namedtuple('ElfRela', 'r_offset r_info r_addend')
_define_variants(ElfRela,
                 layout32='3I',
                 layout64='3Q')

class ElfImage:
    "ELF image parser."
    def __init__(self, image):
        """Create an ELF image from binary image data.

        image: a memoryview-like object that supports efficient range
        subscripting.

        """
        self.image = image
        ident = self.read(ElfIdent, 0)
        classdata = (ident.ei_class, ident.ei_data)
        # Set self.Ehdr etc. to the subtypes with the right parsers.
        for typ in (ElfEhdr, ElfPhdr, ElfShdr, ElfSym, ElfRel, ElfRela):
            setattr(self, typ.__name__[3:], typ.variants.get(classdata, None))

        if self.Ehdr is not None:
            self.ehdr = self.read(self.Ehdr, 0)
        else:
            self.ehdr = None

    def read(self, typ, offset):
        return typ.unpack(self.image[offset: offset + typ.size])

# Only Elf names are exported.
__all__ = [name for name in dir() if name.startswith('Elf')]

with open('/usr/bin/ld.so', 'rb') as inp:
    img = ElfImage(memoryview(inp.read()))
print(img.ehdr)
print(img.read(img.Shdr, img.ehdr.e_shoff))
print(img.read(img.Shdr, img.ehdr.e_shoff + img.ehdr.e_shentsize))
