ARCH:=x86
OS:=linux

LIBGF:=libmoepgf.a
LIBGFDIR:=src

BENCH:=libmoepgfbench
BENCHDIR:=benchmark

CFLAGS:= -O2 -Wall -Wextra -I../include -Isrc
CC:=gcc
AR:=ar
ARFLAGS:=rcs
LDFLAGS:=

