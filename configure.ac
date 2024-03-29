AC_INIT([Moep Galois Field Library], [1.2], [moepi@moepi.net], [libmoepgf], [http://moep80211.net])
#AC_PREREQ([2.69])

AC_PROG_CC
AC_PROG_LN_S

AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects foreign])
AM_SILENT_RULES([yes])

AM_PROG_AR
AM_PROG_CC_C_O	# Needed for some buggy automake versions

AC_SUBST(AR_FLAGS, [cr]) # Fixes ar warning during make

AC_CANONICAL_HOST

lib_cflags=

AS_CASE([$host_cpu],
	[x86_64], [
		arch="x86_64"
		AX_CHECK_COMPILE_FLAG([-msse2],
			[AC_SUBST([SSE2_CFLAGS], ["-msse2"])],
			[AC_MSG_ERROR("Your compiler does not support SSE2")])
		AX_CHECK_COMPILE_FLAG([-mssse3],
			[AC_SUBST([SSSE3_CFLAGS], ["-mssse3"])],
			[AC_MSG_ERROR("Your compiler does not support SSSE3")])
		AX_CHECK_COMPILE_FLAG([-mavx2],
			[AC_SUBST([AVX2_CFLAGS], ["-mavx2"])],
			[AC_MSG_ERROR("Your compiler does not support AVX2")])
		AX_CHECK_COMPILE_FLAG([-mavx512f],
			[AC_SUBST([AVX512_CFLAGS], ["-mavx512f"])],
			[AC_MSG_ERROR("Your compiler does not support AVX512")])
		AX_CHECK_COMPILE_FLAG([-mavx512f -mavx512bw],
			[AC_SUBST([AVX512BW_CFLAGS], ["-mavx512f -mavx512bw"])],
			[AC_MSG_ERROR("Your compiler does not support AVX512BW")])
		AX_CHECK_COMPILE_FLAG([-msse2 -mavx -mavx512bw -mgfni],
			[AC_SUBST([GFNI_CFLAGS], ["-msse2 -mavx -mavx512bw -mgfni"])],
			[AC_MSG_ERROR("Your compiler does not support GFNI")])
	],
	[arm*], [
		arch="arm"
		AX_CHECK_COMPILE_FLAG([-mfpu=neon],
			[AC_SUBST([NEON_CFLAGS], ["-mfpu=neon"])],
			[AC_MSG_ERROR("Your compiler does not support Neon")])
	],
	[powerpc64le], [
		arch="powerpc64le"
	],
	[mips*], [
		arch="mips"
		AX_CHECK_COMPILE_FLAG([-mmsa],
			[AC_SUBST([NEON_CFLAGS], ["-mmsa"])],
			[AC_MSG_ERROR("Your compiler does not support MSA")])
	],
	[riscv*], [
		arch="riscv"
	],
	[AC_MSG_ERROR("Unsupported architecture '$host_cpu'")])

AM_CONDITIONAL([ARCH_X86_64], [test "$arch" = "x86_64"])
AM_CONDITIONAL([ARCH_ARM], [test "$arch" = "arm"])
AM_CONDITIONAL([ARCH_POWERPC64LE], [test "$arch" = "powerpc64le"])
AM_CONDITIONAL([ARCH_MIPS], [test "$arch" = "mips"])
AM_CONDITIONAL([ARCH_RISCV], [test "$arch" = "riscv"])

AX_CHECK_COMPILE_FLAG([-funroll-loops], [lib_cflags="$lib_cflags -funroll-loops"], [])

AC_SUBST([LIB_CFLAGS], ["$lib_cflags"])

AC_SEARCH_LIBS([clock_gettime], [rt])
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_SEARCH_LIBS([pthread_join], [pthread])

LT_INIT

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
