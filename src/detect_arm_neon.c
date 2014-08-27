/*
 * Copyright 2014	Stephan M. Guenther <moepi@moepi.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * See COPYING for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <signal.h>
#include <setjmp.h>

#include <moepgf/moepgf.h>

static sigjmp_buf jmpbuf;

static void
sigill_intrinsic_handler(int sig)
{
	siglongjmp(jmpbuf, 1);
}

uint32_t
detect_arm_neon()
{
	struct sigaction new_action, old_action;
	uint32_t hwcaps = 0;

	new_action.sa_handler = sigill_intrinsic_handler;
	new_action.sa_flags = SA_RESTART;
	sigemptyset(&new_action.sa_mask);
	sigaction(SIGILL, &new_action, &old_action);

	if (!sigsetjmp(jmpbuf, 1)) {
		asm volatile (
			"vand.u8 d0, d1, d0\n"
		);
		hwcaps |= (1 << MOEPGF_HWCAPS_SIMD_NEON);
	}

	sigaction(SIGILL, &old_action, NULL);

	return hwcaps;
}

