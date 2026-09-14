#!/usr/bin/python3
# Copyright (C) 2026 Free Software Foundation, Inc.
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
'''
This script compares the week data in cldr/common/supplemental/supplementalData.xml
with the week data in the glibc locales.

Documentation for glibc: https://sourceware.org/glibc/wiki/Locales#LC_TIME
'''
import glob
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

# CLDR names -> glibc LC_TIME day numbers.
# glibc's day list is:
#   1 = Sunday
#   2 = Monday
#   3 = Tuesday
#   4 = Wednesday
#   5 = Thursday
#   6 = Friday
#   7 = Saturday
DAY_NUMBERS = {
    'sun': 1,
    'mon': 2,
    'tue': 3,
    'wed': 4,
    'thu': 5,
    'fri': 6,
    'sat': 7,
}

def parse_territories(value: str) -> list[str]:
    '''Split a CLDR territories attribute into territory codes.'''
    return value.split()

def parse_week_data(xml_file: Path) -> tuple[
    dict[str, int],
    dict[str, int],
    dict[str, int],
    dict[str, int],
]:
    '''Parse CLDR supplementalData.xml weekData.

    glibc doesn’t use weekend_start but we are parsing it anyway
    just to make sure the CLDR data is well formed.
    '''
    tree = ET.parse(xml_file)
    root = tree.getroot()
    week_data = root.find('.//weekData')
    if week_data is None:
        raise ValueError('Could not find <weekData>')
    min_days: dict[str, int] = {}
    first_day: dict[str, int] = {}
    weekend_start: dict[str, int] = {}
    weekend_end: dict[str, int] = {}
    for element in week_data:
        # Ignore entries such as:
        #
        #   <firstDay ... alt="variant" .../>
        #
        # We want the normal/default CLDR data.
        if 'alt' in element.attrib:
            continue
        territories = parse_territories(element.get('territories', ''))
        if element.tag == 'minDays':
            count = int(element.attrib['count'])
            for territory in territories:
                min_days[territory] = count
        elif element.tag in {'firstDay', 'weekendStart', 'weekendEnd'}:
            day = element.attrib['day']
            try:
                day_number = DAY_NUMBERS[day]
            except KeyError as exc:
                raise ValueError(f'Unknown CLDR day: {day!r}') from exc
            target = {
                'firstDay': first_day,
                'weekendStart': weekend_start,
                'weekendEnd': weekend_end,
            }[element.tag]

            for territory in territories:
                target[territory] = day_number
        else:
            # Ignore other weekData elements if CLDR adds any in future.
            continue
    return min_days, first_day, weekend_start, weekend_end

def extract_lc_time(text: str) -> str | None:
    '''Return the contents of the LC_TIME section.'''
    match = re.search(r'(?ms)^LC_TIME\b(.*?)(?=^END LC_TIME\b)', text)
    if match is None:
        return None
    return match.group(1)

def parse_lc_time(lc_time: str) -> tuple[str | None, int | None, int | None, int | None]:
    '''
    Parse copy, week, first_weekday, and first_workday
    from an LC_TIME section.

    Returns:
        (copy_locale, min_days, first_weekday, first_workday)
    '''
    copy_match = re.search(r'(?m)^\s*copy\s+"([^"]+)"', lc_time)
    copy_locale = copy_match.group(1) if copy_match else None
    week_match = re.search(r"(?m)^\s*week\s+([^#\n]+)", lc_time)
    min_days = None
    if week_match:
        fields = week_match.group(1).strip().split(";")
        if len(fields) != 3:
            print('WARNING: malformed week line:', week_match.group(0).strip())
        else:
            try:
                min_days = int(fields[2])
            except ValueError:
                print('WARNING: invalid MINWEEKLEN in:', week_match.group(0).strip())
    first_weekday_match = re.search(r'(?m)^\s*first_weekday\s+(\d+)', lc_time)
    first_weekday = (
        int(first_weekday_match.group(1))
        if first_weekday_match
        else None
    )
    first_workday_match = re.search(r'(?m)^\s*first_workday\s+(\d+)', lc_time)
    first_workday = (
        int(first_workday_match.group(1))
        if first_workday_match
        else None
    )
    return (copy_locale, min_days, first_weekday, first_workday)

def expected_value(
    values: dict[str, int],
    territory: str,
) -> int:
    '''Get a CLDR value, using territory 001 as the default.'''
    return values.get(territory, values['001'])

def expected_first_workday(
    weekend_end: dict[str, int],
    territory: str,
) -> int:
    '''
    The first workday is the day after CLDR weekendEnd.

    With glibc day numbers:
        Sunday    = 1
        ...
        Saturday  = 7
    '''
    end_day = expected_value(weekend_end, territory)
    return end_day % 7 + 1

# For the ICU/CLDR locale pattern see: http://userguide.icu-project.org/locale
# That pattern works for glibc locale names as well.
# (We ignore the variant code here)
_cldr_locale_pattern = re.compile(
    # language must be 2 or 3 lower case letters:
    '^(?P<language>[a-z]{2,3}'
    # language is only valid if
    +'(?=$|@' # locale string ends here or only options follow
    +'|_[A-Z][a-z]{3}(?=$|@|_[A-Z0-9]{2,3}(?=$|@))' # valid script follows
    +'|_[A-Z0-9]{2,3}(?=$|@)' # valid territory follows
    +'))'
    # script must be 1 upper case letter followed by
    # 3 lower case letters:
    +'(?:_(?P<script>[A-Z][a-z]{3})'
    # script is only valid if
    +'(?=$|@' # locale string ends here or only options follow
    +'|_[A-Z0-9]{2,3}(?=$|@)' # valid territory follows
    +')){0,1}'
    # territory must be 2 upper case letters or 3 digits:
    +'(?:_(?P<territory>[A-Z0-9]{2,3})'
    # territory is only valid if
    +'(?=$|@' # locale string ends here or only options follow
    +')){0,1}')

def territory_from_name(name: str) -> str:
    '''Gets the 2 letter territory code from a locale name

    Returns:

    - 2 letter territory code if it is a valid locale name with a territory
    - '001' if it is a valid locale name but has no territory
      ('001' means “World”)
    - '' if name is not a valid locale name
    '''
    if name == '':
        return ''
    if name in ('C', 'POSIX'):
        # ignore C and POSIX locales for the moment:
        return ''
    match = _cldr_locale_pattern.match(name)
    if not match:
        return ''
    if match.group('territory'):
        return match.group('territory')
    return '001'

def check_locale(
    filename: str,
    *,
    min_days: dict[str, int],
    first_day: dict[str, int],
    weekend_end: dict[str, int],
) -> bool:
    '''Check one glibc locale file.

    Returns True if min_days, first_day, and weekend_end of the locale file agree with CLDR
    Returns False if it does not agree or glibc uses defaults
    '''
    result = True
    path = Path(filename)
    # For *_CC locale names, the territory is the final two characters.
    # Examples:
    #   pt_PT -> PT
    #   chr_US -> US
    #   ber_MA -> MA
    territory = territory_from_name(path.name)
    if territory == '':
        print(f'WARNING: {path.name}: not a valid locale name')
        return False
    text = path.read_text(encoding='utf-8')
    lc_time = extract_lc_time(text)
    if lc_time is None:
        print(f'WARNING: {path.name}: no LC_TIME section')
        return False
    (
        copy_locale,
        locale_min_days,
        locale_first_weekday,
        locale_first_workday,
    ) = parse_lc_time(lc_time)
    expected_min_days = expected_value(min_days, territory)
    expected_first_weekday = expected_value(first_day, territory)
    expected_workday = expected_first_workday(weekend_end, territory)
    if copy_locale is not None:
        copy_territory = territory_from_name(copy_locale)
        if expected_min_days != expected_value(min_days, copy_territory):
            print(f'WARNING: {path.name}: LC_TIME copied from {copy_locale} but expected CLDR minDays differs.')
            result = False
        if expected_first_weekday != expected_value(first_day, copy_territory):
            print(f'WARNING: {path.name}: LC_TIME copied from {copy_locale} but expected CLDR firstDay differs.')
            result = False
        if expected_workday != expected_first_workday(weekend_end, copy_territory):
            print(f'WARNING: {path.name}: LC_TIME copied from {copy_locale} but expected CLDR weekendEnd differs.')
            result = False
        return result
    # Check that all three values exist.
    missing = []
    if locale_min_days is None:
        missing.append('week')
    if locale_first_weekday is None:
        missing.append('first_weekday')
    if locale_first_workday is None:
        missing.append('first_workday')
    if missing:
        print(f'WARNING: {path.name}: missing (default used): ' + ', '.join(missing))
        result = False
    if locale_min_days is not None and locale_min_days != expected_min_days:
        print(f'WARNING: {path.name}: '
              f'week MINWEEKLEN = {locale_min_days}, '
              f'CLDR minDays = {expected_min_days}')
        result = False
    if locale_first_weekday is not None and locale_first_weekday != expected_first_weekday:
        print(f'WARNING: {path.name}: '
              f'first_weekday = {locale_first_weekday}, '
              f'CLDR firstDay = {expected_first_weekday}')
        result = False
    if locale_first_workday is not None and locale_first_workday != expected_workday:
        print(f'WARNING: {path.name}: '
              f'first_workday = {locale_first_workday}, '
              f'CLDR weekendEnd implies {expected_workday}')
        result = False
    return result

def main() -> None:
    if len(sys.argv) != 3:
        print(f'usage: {sys.argv[0]} '
              "supplementalData.xml '../locales/*'",
              file=sys.stderr)
        sys.exit(2)
    xml_file = Path(sys.argv[1])
    locale_pattern = sys.argv[2]
    (
        min_days,
        first_day,
        weekend_start,
        weekend_end,
    ) = parse_week_data(xml_file)
    # Make sure CLDR has its 001 defaults.
    for name, values in (
        ('min_days', min_days),
        ('first_day', first_day),
        ('weekend_start', weekend_start),
        ('weekend_end', weekend_end),
    ):
        if '001' not in values:
            raise ValueError(f'{name} has no CLDR 001 default')
    filenames = sorted(glob.glob(locale_pattern))
    locales_checked: list[str] = []
    locales_count = 0
    problem_count = 0
    for filename in filenames:
        path = Path(filename)
        if territory_from_name(path.name) == '':
            # Ignore invalid locale names
            continue
        check_result = check_locale(
            filename,
            min_days=min_days,
            first_day=first_day,
            weekend_end=weekend_end,
        )
        locales_count += 1
        locales_checked.append(path.name)
        if not check_result:
            problem_count += 1
    print(f'Locales checked: {locales_checked}')
    print(f'Number of locales checked: {locales_count}')
    print(f'Number of locales which have problems: {problem_count}.')
    sys.exit(0 if problem_count == 0 else 1)

if __name__ == '__main__':
    main()
