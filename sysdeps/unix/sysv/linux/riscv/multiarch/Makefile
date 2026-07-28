ifeq ($(subdir),string)
sysdep_routines += \
  memccpy \
  memccpy-generic \
  memccpy-vector \
  memchr \
  memchr-generic \
  memchr-vector \
  memcmp \
  memcmp-generic \
  memcmp-vector \
  memcpy \
  memcpy-generic \
  memcpy-vector \
  memcpy_noalignment \
  memmove \
  memmove-generic \
  memmove-vector \
  memset \
  memset-generic \
  memset-vector \
  stpncpy \
  stpncpy-generic \
  stpncpy-vector \
  strcat \
  strcat-generic \
  strcat-vector \
  strchr \
  strchr-generic \
  strchr-vector \
  strcmp \
  strcmp-generic \
  strcmp-vector \
  strcpy \
  strcpy-generic \
  strcpy-vector \
  strlen \
  strlen-generic \
  strlen-vector \
  strncmp \
  strncmp-generic \
  strncmp-vector \
  strncpy \
  strncpy-generic \
  strncpy-vector \
  strrchr \
  strrchr-generic \
  strrchr-vector \
  # sysdep_routines

# Called during static library initialization, so turn stack-protection
# off for non-shared builds.
CFLAGS-memset-generic.o = $(no-stack-protector)
CFLAGS-memset-generic.op = $(no-stack-protector)
CFLAGS-memcpy-generic.o = $(no-stack-protector)
CFLAGS-memcpy-generic.op = $(no-stack-protector)
CFLAGS-strlen-generic.o = $(no-stack-protector)
CFLAGS-strlen-generic.op = $(no-stack-protector)

CFLAGS-memcpy_noalignment.c += -mno-strict-align
endif
