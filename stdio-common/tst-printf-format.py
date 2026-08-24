#!/usr/bin/python3
# Verification of formatted printf output.
# Copyright (C) 2024-2026 Free Software Foundation, Inc.
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

"""Verify records of formatted printf output.

The record stream produced by tst-printf-format-skeleton.c is read from
standard input and each record's output is checked against a reference
value computed here.  Diagnostics for records that do not match are written
to standard error and a nonzero exit status is returned.

This is deliberately an independent implementation of the ISO C format
processing rules: every conversion is computed with exact integer and
rational arithmetic and nothing defers to the C library, so a bug in printf
cannot verify itself.  Working exactly also means no dependence on the
range or precision of any host floating-point type, so the wider types are
handled without arbitrary-precision support having to be built into the
interpreter.
"""

import io
import os
import sys
from fractions import Fraction

# Conversions grouped by the C type of the corresponding argument.
FLOAT_CONVS = frozenset("eEfFgG")
INT_CONVS = frozenset("bBdiouxX")

# The conversion specifier selects the base an integer is written in.
INT_FORMATS = {"b": "b", "B": "b", "o": "o", "u": "d", "x": "x", "X": "X"}

# Flags and length modifiers accepted in a conversion specification.
FLAG_CHARS = "-+ #0"
LENGTH_CHARS = "hlLqjzt"

# log10(2), used only to seed the decimal exponent search.
LOG10_2 = 0.3010299956639812


def diagnose(text):
    """Report TEXT on standard error.

    Records hold arbitrary bytes, taken as Latin-1 on the way in, so put
    them back the same way rather than through whatever encoding the
    locale would otherwise select."""
    sys.stderr.buffer.write(text.encode("latin-1") + b"\n")
    sys.stderr.buffer.flush()


def round_ratio(num, den):
    """Round the non-negative ratio NUM/DEN to the nearest integer, with
    ties going to even, matching the default rounding mode."""
    quot, rem = divmod(num, den)
    rem *= 2
    if rem > den or (rem == den and quot & 1):
        quot += 1
    return quot


def scale_ratio(num, den, power):
    """Return NUM/DEN multiplied by 10**POWER, as a pair of integers."""
    if power >= 0:
        return num * 10 ** power, den
    return num, den * 10 ** -power


def decimal_to_binary(text, mant_bits):
    """Convert the decimal string TEXT to the nearest value having MANT_BITS
    of significand, returned as an exact Fraction.

    The generator prints reference values with enough digits to reproduce
    the original, so this recovers the exact value converted."""
    fr = Fraction(text)
    if fr == 0:
        return fr
    neg = fr < 0
    num, den = (-fr.numerator if neg else fr.numerator), fr.denominator
    # Scale so that 2**(mant_bits-1) <= num/den < 2**mant_bits.
    exp = num.bit_length() - den.bit_length() - mant_bits
    if exp >= 0:
        den <<= exp
    else:
        num <<= -exp
    while num >= den << mant_bits:
        den <<= 1
        exp += 1
    while num < den << (mant_bits - 1):
        num <<= 1
        exp -= 1
    mant = round_ratio(num, den)
    if mant >> mant_bits:
        mant >>= 1
        exp += 1
    fr = Fraction(mant << exp) if exp >= 0 else Fraction(mant, 1 << -exp)
    return -fr if neg else fr


def decimal_digits(fr, ndigits):
    """Round the positive Fraction FR to NDIGITS significant decimal digits.

    Return the digit string and the decimal exponent X such that the value
    is D[0].D[1:] * 10**X, i.e. X is the exponent an 'e' conversion prints."""
    num, den = fr.numerator, fr.denominator
    # Seed from the binary magnitude; the estimate is within one decade.
    x = int((num.bit_length() - den.bit_length()) * LOG10_2)
    while True:
        hi_num, hi_den = scale_ratio(num, den, -(x + 1))
        if hi_num < hi_den:
            break
        x += 1
    while True:
        lo_num, lo_den = scale_ratio(num, den, -x)
        if lo_num >= lo_den:
            break
        x -= 1
    digits = round_ratio(*scale_ratio(num, den, ndigits - 1 - x))
    if digits >= 10 ** ndigits:
        # Rounding carried into the next decade, e.g. 9.99 -> 10.0.
        digits //= 10
        x += 1
    return str(digits).zfill(ndigits), x


class Spec:
    """A parsed conversion specification."""

    __slots__ = ("minus", "plus", "space", "alt", "zero", "width", "prec",
                 "conv", "neg_zero")

    def __init__(self, fmt, args):
        """Parse FMT, taking values for any '*' from ARGS in order."""
        self.minus = self.plus = self.space = self.alt = self.zero = False
        self.neg_zero = False
        i = 1                                   # skip '%'
        while i < len(fmt) and fmt[i] in FLAG_CHARS:
            char = fmt[i]
            self.minus |= char == "-"
            self.plus |= char == "+"
            self.space |= char == " "
            self.alt |= char == "#"
            self.zero |= char == "0"
            i += 1

        self.width = 0
        if i < len(fmt) and fmt[i] == "*":
            self.width = int(args.pop(0))
            i += 1
            # A negative field width is a '-' flag with a positive width.
            if self.width < 0:
                self.minus = True
                self.width = -self.width
        else:
            start = i
            while i < len(fmt) and fmt[i].isdigit():
                i += 1
            if i > start:
                self.width = int(fmt[start:i])

        self.prec = None
        if i < len(fmt) and fmt[i] == ".":
            i += 1
            if i < len(fmt) and fmt[i] == "*":
                self.prec = int(args.pop(0))
                i += 1
                # A negative precision is taken as if it were omitted.
                if self.prec < 0:
                    self.prec = None
            else:
                start = i
                while i < len(fmt) and fmt[i].isdigit():
                    i += 1
                self.prec = int(fmt[start:i]) if i > start else 0

        # Length modifiers only select the C type of the argument, which the
        # generator has already applied; they do not affect the output.
        while i < len(fmt) and fmt[i] in LENGTH_CHARS:
            i += 1
        self.conv = fmt[i]

    def pad(self, body, zero_ok=True, sign=""):
        """Apply the field width to SIGN followed by BODY."""
        count = self.width - len(sign) - len(body)
        if count <= 0:
            return sign + body
        if self.minus:
            return sign + body + " " * count
        if self.zero and zero_ok:
            return sign + "0" * count + body
        return " " * count + sign + body

    def sign_of(self, neg):
        """The character introducing a signed conversion of a value whose
        sign is NEG, honoring the '+' and space flags."""
        if neg:
            return "-"
        if self.plus:
            return "+"
        if self.space:
            return " "
        return ""


def convert_int(value, spec):
    conv = spec.conv
    if conv in "di":
        neg = value < 0
        digits = str(-value if neg else value)
        sign = spec.sign_of(neg)
    else:
        digits = format(value, INT_FORMATS[conv])
        sign = ""
    if spec.prec is not None:
        # A zero value converted with a precision of zero produces no
        # characters at all.
        if value == 0 and spec.prec == 0:
            digits = ""
        digits = digits.rjust(spec.prec, "0")
    if spec.alt:
        if conv == "o" and not digits.startswith("0"):
            digits = "0" + digits
        elif conv in "bBxX" and value != 0:
            # The base prefix precedes any '0' flag padding, so it pads
            # along with the sign rather than with the digits.
            sign = "0" + conv
    # An explicit precision defeats the '0' flag.
    return spec.pad(digits, zero_ok=spec.prec is None, sign=sign)


def convert_string(value, spec):
    if spec.prec is not None:
        value = value[:spec.prec]
    return spec.pad(value, zero_ok=False)


def convert_char(value, spec):
    return spec.pad(value[:1], zero_ok=False)


def convert_special(kind, neg, spec):
    """Convert an infinity or a NaN.  The field is padded with spaces
    whether or not the '0' flag was given, and the alternative form has no
    effect."""
    body = kind.upper() if spec.conv in "EFG" else kind
    return spec.pad(body, zero_ok=False, sign=spec.sign_of(neg))


def render_f(fr, prec, alt, strip):
    """Render the non-negative Fraction FR in the style of 'f'."""
    scaled = round_ratio(*scale_ratio(fr.numerator, fr.denominator, prec))
    text = str(scaled).zfill(prec + 1)
    if prec:
        whole, frac = text[:-prec], text[-prec:]
    else:
        whole, frac = text, ""
    if strip:
        frac = frac.rstrip("0")
    if frac or alt:
        return whole + "." + frac
    return whole


def render_e(fr, prec, alt, upper, strip):
    """Render the non-negative Fraction FR in the style of 'e'."""
    if fr == 0:
        digits, exp = "0" * (prec + 1), 0
    else:
        digits, exp = decimal_digits(fr, prec + 1)
    whole, frac = digits[0], digits[1:]
    if strip:
        frac = frac.rstrip("0")
    mant = whole + "." + frac if frac or alt else whole
    return "%s%s%s%02d" % (mant, "E" if upper else "e",
                           "-" if exp < 0 else "+", abs(exp))


def convert_float(value, spec, cache):
    """Convert VALUE, an exact Fraction.  CACHE memoizes rendered digits for
    the value currently being converted."""
    conv = spec.conv
    neg = value < 0 or (value == 0 and spec.neg_zero)
    fr = -value if value < 0 else value
    prec = 6 if spec.prec is None else spec.prec
    upper = conv in "EFG"

    # The generator iterates a large number of flag and field width
    # combinations over each value, so the same digits are called for over
    # and over.  They depend only on the magnitude, the precision and the
    # alternative form, which is what keeps the wider types inexpensive
    # despite the arithmetic being exact.
    key = (conv, prec, spec.alt)
    body = cache.get(key)
    if body is None:
        if conv in "gG":
            sig = 1 if prec == 0 else prec
            _, exp = decimal_digits(fr, sig) if fr != 0 else ("", 0)
            if -4 <= exp < sig:
                body = render_f(fr, sig - 1 - exp, spec.alt,
                                strip=not spec.alt)
            else:
                body = render_e(fr, sig - 1, spec.alt, upper,
                                strip=not spec.alt)
        elif conv in "eE":
            body = render_e(fr, prec, spec.alt, upper, strip=False)
        else:
            body = render_f(fr, prec, spec.alt, strip=False)
        cache[key] = body

    return spec.pad(body, zero_ok=True, sign=spec.sign_of(neg))


class Block:
    """The state a run of records shares: the C type they exercise and the
    single value they all convert."""

    __slots__ = ("mant_bits", "val_text", "value", "special",
                 "neg_zero", "cache")

    def __init__(self):
        self.mant_bits = 0
        self.set_value("")

    def set_value(self, text):
        """Begin a run of records converting the value written as TEXT."""
        self.val_text = text
        self.value = None
        self.special = None
        self.neg_zero = False
        self.cache = {}      # digits memoized by convert_float

    def interpret(self, conv):
        """Interpret the value text as the C type conversion CONV takes."""
        if conv in FLOAT_CONVS:
            lowered = self.val_text.lower()
            if "inf" in lowered or "nan" in lowered:
                self.special = "nan" if "nan" in lowered else "inf"
            else:
                self.value = decimal_to_binary(self.val_text, self.mant_bits)
            self.neg_zero = self.val_text.lstrip().startswith("-")
        elif conv in INT_CONVS:
            self.value = int(self.val_text)
        else:
            self.value = self.val_text

    def expect(self, spec):
        """The output SPEC is required to produce for this block's value."""
        conv = spec.conv
        # Which C type the value has follows from the conversion specifier,
        # which is not known until the first record of the block, so the
        # text is interpreted here rather than where it was read.
        if self.value is None and self.special is None:
            self.interpret(conv)
        spec.neg_zero = self.neg_zero
        if conv in FLOAT_CONVS:
            if self.special is not None:
                return convert_special(self.special, self.neg_zero, spec)
            return convert_float(self.value, spec, self.cache)
        if conv in INT_CONVS:
            return convert_int(self.value, spec)
        if conv == "c":
            return convert_char(self.value, spec)
        return convert_string(self.value, spec)


def check(raw, block):
    """Check one record against the value BLOCK holds, reporting a mismatch
    on standard error.  Return whether the record was in order."""
    # Records are colon separated and end with an empty field, included to
    # make the data easier to read.  What precedes the output are the format
    # and then any width and precision supplied as arguments.
    fields = raw.split(":")
    fields.pop()
    fmt, actual, args = fields[0], fields[-1], fields[1:-1]

    expect = block.expect(Spec(fmt, list(args)))
    if expect == actual:
        return True
    diagnose('("%s"%s, %s) => "%s", expected "%s"'
             % (fmt, "".join(", " + arg for arg in args), block.val_text,
                actual, expect))
    return False


def main():
    # The wider types at the precisions exercised produce integers well past
    # the default limit on conversion to a string.  Versions that predate
    # the limit have no such call and need no lifting.
    if hasattr(sys, "set_int_max_str_digits"):
        sys.set_int_max_str_digits(0)

    # Records hold arbitrary bytes, so read them as Latin-1 rather than
    # through whatever encoding the locale would otherwise select.
    stdin = io.TextIOWrapper(sys.stdin.buffer, encoding="latin-1",
                             newline="\n")

    block = Block()
    status = 0

    for raw in stdin:
        raw = raw.rstrip("\n")

        if raw.startswith("prec:"):
            block.mant_bits = int(raw[5:])
        elif raw.startswith("val:"):
            block.set_value(raw[4:])
        elif raw.startswith("%"):
            if not check(raw, block):
                status = 1
        else:
            diagnose('unrecognized input: "%s"' % raw)
            status = 1

    return status


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BrokenPipeError:
        # The wrappers pipe into 'head', which stops reading once it has
        # the first diagnostic.  Point what is left of standard error at
        # the null device, so that flushing at exit does not run into the
        # same failure and report it a second time.
        os.dup2(os.open(os.devnull, os.O_WRONLY), sys.stderr.fileno())
        sys.exit(1)
