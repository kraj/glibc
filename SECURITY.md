# The GNU C Library Security Policy

This document describes the policy followed by the GNU C Library maintainers
to handle bugs that may have a security impact.  This includes determining if a
bug has a security impact, reporting such bugs to the community and handling
such bugs all the way to resolution.  This policy may evolve over time, so if
you're reading this from a release tarball, be sure to check the latest copy of
the [SECURITY.md in the
repository](https://sourceware.org/git/?p=glibc.git;a=blob;f=SECURITY.md),
especially for instructions on reporting issues privately.

## What is a security bug?

Most security vulnerabilities in the GNU C Library materialize only after an
application uses functionality in a specific way.  Therefore, it is sometimes
difficult to determine if a defect in the GNU C Library constitutes a
vulnerability as such.  The following guidelines can help with a decision.

* Buffer overflows should be treated as security bugs if it is conceivable that
  the data triggering them can come from an untrusted source.
* Other bugs that cause memory corruption which is likely exploitable should be
  treated as security bugs.
* Information disclosure can be a security bug, especially if exposure through
  applications can be determined.
* Memory leaks and races are security bugs if they cause service breakage.
* Stack overflow through unbounded alloca calls or variable-length arrays are
  security bugs if it is conceivable that the data triggering the overflow
  could come from an untrusted source.
* Stack overflow through deep recursion and other crashes are security bugs if
  they cause service breakage.
* Bugs that cripple the whole system (so that it doesn't even boot or does not
  run most applications) are not security bugs because they will not be
  exploitable in practice, due to general system instability.
* Bugs that crash `nscd` are generally security bugs, except if they can only
  be triggered by a trusted data source (DNS is not trusted, but NIS and LDAP
  are).
* Bugs that can only be triggered through trusted inputs, such as the system
  configuration, are not security bugs.  The section on trusted and untrusted
  inputs below gives examples.
* Bugs that only occur when an application breaks the documented interface
  are not security bugs.  Examples include passing flag bits that the manual
  does not document (some are reserved for internal use), and operations that
  ISO C or POSIX leave undefined, such as reading from a stream right after
  writing to it without a call to `fflush` or a file positioning function in
  between.
* A report should show the problem through the public interfaces of the
  library.  A proof of concept that copies library code into a test program,
  or calls internal functions with arguments that the library never passes to
  them, does not show a vulnerability.
* The [Security Exceptions](#SecurityExceptions) section below describes
  subsystems for which determining the security status of bugs is especially
  complicated.
* For consistency, if the bug has received a CVE name attributing it to the GNU
  C library, it should be flagged `security+`.
* Duplicates of security bugs (flagged with `security+`) should be flagged
  `security-`, to avoid cluttering the reporting.

In this context, _service breakage_ means client-side privilege escalation
(code execution) or server-side denial of service or privilege escalation
through actual, concrete, non-synthetic applications. Or put differently, if
the GNU C Library causes a security bug in an application (and the application
uses the library in a standard-conforming manner or according to the manual),
the GNU C Library bug should be treated as security-relevant.

### Trusted and untrusted inputs

Whether a bug is a security bug usually depends on who controls the data that
triggers it.  The GNU C Library assumes that the system administrator controls
the system configuration and the files installed in system locations, and that
the user who starts a process controls that process, unless the process runs
in secure-execution mode (`AT_SECURE`).  Bugs that only these trusted parties
can trigger are not security bugs because they do not cross a trust boundary.

The following inputs are untrusted:

* Data that applications process on behalf of other parties: network data
  (including DNS responses), file contents, and the strings that applications
  pass to the library for conversion or parsing (for example to `iconv`, the
  `scanf` family or `getaddrinfo`).  This includes the names of character sets,
  which applications often take from the documents or messages they process.
* The environment of an `AT_SECURE` process.  The library removes or ignores
  variables such as `LD_PRELOAD`, `LD_LIBRARY_PATH`, `GLIBC_TUNABLES`,
  `GCONV_PATH`, `LOCPATH` and `NLSPATH` in such processes, and a bug that lets
  them take effect is a security bug.
* The current directory of an `AT_SECURE` process.
* The installation path of an `AT_SECURE` process.  That is, the directory in
  which the program is installed, and all its parent directories.
* The initial open file descriptors of an `AT_SECURE` process.
* Locale names, including the values of `LANG`, `LC_ALL`, the other `LC_*`
  variables and `LANGUAGE`.  These variables commonly cross trust boundaries
  in processes that are not `AT_SECURE`, for example when an SSH server accepts
  them from clients, so they are untrusted in every process.
* Configuration values that network configuration software commonly copies
  from the network, such as the name servers and search domains that a DHCP
  or VPN server supplies for `/etc/resolv.conf`.
* Fields that unprivileged users may change for their own account, in the
  files databases or in identity management services, such as the GECOS
  field (with `chfn`), the login shell (with `chsh`) or the password.

The following inputs are trusted:

* The dynamic loader configuration: `/etc/ld.so.conf` and the files it
  includes, `/etc/ld.so.cache`, `/etc/ld.so.preload` and `/etc/tunables.conf`.
* Executables and shared objects, including the objects that programs load
  with `dlopen`.  The section on shared objects, `ldd` and `ldconfig` below
  has more details.
* The name service configuration and the databases that the administrator
  maintains: `/etc/nsswitch.conf`, the files read by the `files` service (such
  as `/etc/passwd`, `/etc/group`, `/etc/hosts` and `/etc/services`),
  `/etc/host.conf` and `/etc/nscd.conf`.  The rest of `/etc/resolv.conf`,
  apart from the values listed above, is trusted as well.
* Data from identity management services such as NIS and Hesiod, or from NSS
  modules for services such as LDAP: both its format on the wire and the data
  it returns.  Malformed DNS messages are an exception (see below).
* Buffer lengths that the kernel reports, for example the return value of a
  successful `read` or `write` system call.
* The standard installation directories chosen at configure time (`--prefix`,
  `--libdir`, `--sysconfdir` and the others).
* Locale data: the compiled locales in the system locale directory and locale
  archive, and the sources (locale definitions, character maps and repertoire
  maps) that `localedef` compiles into them.  `LOCPATH` is ignored in
  `AT_SECURE` processes, so a locale loaded through it only affects processes
  of the user who sets it, and crafted binary locale files are not a security
  issue.
* Message catalogs (`.mo` files) used by `gettext` and related functions.
  Translations are used as `printf` format strings, so a crafted catalog can
  take control of a program regardless of how carefully the library parses it.
  Bugs triggered by crafted catalogs are not security bugs, but a bug that lets
  untrusted input make a program load a catalog from a location that the
  program did not select is a security bug.
* The character set conversion configuration: the `gconv-modules` files, the
  `gconv-modules.cache` file built by `iconvconfig`, and the conversion
  modules.
* The input of the tools that build system data, such as `ldconfig`,
  `localedef`, `iconvconfig` and `zic`.  Like a compiler, these tools trust
  their input, so untrusted input should only be processed in a sandbox.
* For a process that is not `AT_SECURE`, anything that only the user who
  starts it controls, apart from locale names: the environment (for example
  `GLIBC_TUNABLES`, `LD_PRELOAD`, `LD_LIBRARY_PATH`, the `MALLOC_*` variables,
  `LOCPATH` and `GCONV_PATH`), the command line and the resource limits.  This
  user can already run arbitrary code in the process, for example with
  `LD_PRELOAD`.

The library treats these files as trusted whatever their ownership and
permissions, and does not check them.  If the administrator lets untrusted
users modify such a file, or a program points the library at a file or
directory that untrusted users can modify (for example with `bindtextdomain`
or by running `ldconfig` on it), those users gain the control that the file
gives, and this is not a bug in the library.

The DNS stub resolver does not authenticate responses.  An attacker who can
observe or modify the traffic to the configured name servers can supply
arbitrary answers, including Hesiod data, and this is not a bug in the
library.  DNS messages themselves are untrusted whatever the query is for, so
bugs in the processing of malformed DNS messages are security bugs.

### Security Exceptions

It may be especially complicated to determine the security status of bugs in
some subsystems in the GNU C Library.  This subsection describes such
subsystems and the special considerations applicable during security bug
classification in them.

#### Regular expression processing

Regular expression processing comes in two parts, compilation (through regcomp)
and execution (through regexec).

Implementing regular expressions efficiently, in a standard-conforming way, and
without denial-of-service vulnerabilities is very difficult and impossible for
Basic Regular Expressions. Most implementation strategies have issues dealing
with certain classes of patterns.

Consequently, certain issues which can be triggered only with crafted patterns
(either during compilation or execution) are treated as regular bugs and not
security issues.  Examples of such issues include (but are not limited to):

 * Running out of memory through valid use of malloc
 * Quadratic or exponential behaviour resulting in slow execution time
 * Stack overflows due to recursion when processing patterns

Crashes, infinite loops (and not merely exponential behavior), buffer overflows
and overreads, memory leaks and other bugs resulting from the regex
implementation relying on undefined behavior should be treated as security
vulnerabilities.

#### wordexp patterns

`wordexp` inherently has exponential memory consumption in terms of the input
size.  This means that denial of service flaws from crafted patterns are not
security issues (even if they lead to other issues, such as NULL pointer
dereferences).

#### Asynchronous I/O

The GNU C Library tries to implement asynchronous I/O without kernel support,
which means that several operations are not fully standard conforming.  Several
known races can cause crashes and resource leaks.  Such bugs are only treated
as security bugs if applications (as opposed to synthetic test cases) have
security exposures due to these bugs.

#### Asynchronous cancellation

The implementation of asynchronous cancellation is not fully
standard-conforming and has races and leaks.  Again, such bugs are only treated
as security bugs if applications (as opposed to synthetic test cases) have
security exposures due to these bugs.

#### Shared objects, ldd and ldconfig

Loading a shared object runs code from it, such as ELF constructors and IFUNC
resolvers, so the dynamic loader trusts the objects it loads.  Bugs triggered
by crafted executables or shared objects are not security bugs, whether the
objects are loaded at startup (through `DT_NEEDED` dependencies, or through
`LD_PRELOAD` and `LD_AUDIT` in a process that is not `AT_SECURE`) or with
`dlopen`.  A bug that makes an `AT_SECURE` process load an object from a
location that the invoking user controls, for example through
`LD_LIBRARY_PATH`, `LD_PRELOAD` or `$ORIGIN`, is a security bug.

The `ldd` tool is not expected to be used with untrusted executables.

The `ldconfig` tool is run by the administrator, usually as root.  Its
configuration files and the shared objects in the directories it scans are
trusted, including when the `-f`, `-C` or `-r` options point it to another
configuration file, cache or root directory.  Crashes, hangs or excessive
resource use caused by crafted configuration files or crafted shared objects
(for example, malformed ELF notes) are not security bugs.  To run `ldconfig`
on a directory tree that untrusted users can modify, use a sandbox.

#### Post-exploitation countermeasures

Certain features have been added to the library only to make exploitation of
security bugs (mainly for code execution) more difficult.  Examples include
the stack smashing protector, function pointer obfuscation, vtable validation
for stdio stream handles, and various heap consistency checks.  Failure of such
countermeasures to stop exploitation of a different vulnerability is not a
security vulnerability in itself.  By their nature, these countermeasures are
based on heuristics and will never offer complete protection, so the original
vulnerability needs to be fixed anyway.

## Reporting security bugs

The process to report security bugs is documented on the glibc [security
page](https://sourceware.org/glibc/security.html).  In general, most security
bugs may be reported publicly in the [glibc
bugzilla](https://sourceware.org/glibc/bugs.html), but if in doubt, please feel
free to report security issues privately first.

## Triaging security bugs

This section is aimed at developers, not reporters.

Security-relevant bugs should be marked with `security+`, as per the [Bugzilla
security flag
documentation](https://sourceware.org/glibc/wiki/Bugzilla%20Procedures#security),
following the guidelines above.  If you set the `security+` flag, you should
make sure the following information is included in the bug (usually in a bug
comment):

* The first glibc version which includes the vulnerable code.  If the
  vulnerability was introduced before glibc 2.4 (released in 2006), this
  information is not necessary.
* The commit or commits (identified by hash) that fix this vulnerability in the
  master branch, and (for historic security bugs) the first release that
  includes this fix.
* The summary should include the CVE names (if any), in parentheses at the end.
* If there is a single CVE name assigned to this bug, it should be set as an
  alias.

The following links are helpful for finding untriaged bugs:

* [Unprocessed bugs](https://sourceware.org/bugzilla/buglist.cgi?f1=flagtypes.name&o1=notsubstring&product=glibc&query_format=advanced&v1=security)
* [`security?` review requests](https://sourceware.org/bugzilla/buglist.cgi?f1=flagtypes.name&o1=substring&product=glibc&query_format=advanced&v1=security%3f)
* [Open `security+` bugs](https://sourceware.org/bugzilla/buglist.cgi?bug_status=UNCONFIRMED&bug_status=NEW&bug_status=ASSIGNED&bug_status=SUSPENDED&bug_status=WAITING&bug_status=REOPENED&bug_status=VERIFIED&f1=flagtypes.name&o1=substring&product=glibc&query_format=advanced&v1=security%2B)

## Fixing security bugs

For changes to master, the regular [consensus-driven
process](https://sourceware.org/glibc/wiki/Consensus) must be followed.  It
makes sense to obtain consensus in private, to ensure that the patch is likely
in a committable state, before disclosing an embargoed vulnerability.

Security backports to release branches need to follow the
[release process](https://sourceware.org/glibc/wiki/Release#General_policy).

Contact the [website
maintainers](https://sourceware.org/glibc/wiki/MAINTAINERS#Maintainers_for_the_website)
and have them draft a news entry for the website frontpage to direct users to
the bug, the fix, or the mailing list discussions.

## CVE assignment

Security bugs flagged with `security+` should have [CVE
identifiers](https://www.cve.org/About/Overview). Please reach out to the glibc
security team using the documented [security
process](https://sourceware.org/glibc/security.html) and they work on getting a
CVE number.
