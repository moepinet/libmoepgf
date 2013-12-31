/*
 * This file is part of moep80211gf.
 * 
 * Copyright (C) 2014 	Stephan M. Guenther <moepi@moepi.net>
 * Copyright (C) 2014 	Maximilian Riemensberger <riemensberger@tum.de>
 * Copyright (C) 2013 	Alexander Kurtz <alexander@kurtz.be>
 * 
 * moep80211gf is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2 of the License.
 * 
 * moep80211gf is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License * along
 * with moep80211gf.  If not, see <http://www.gnu.org/licenses/>.
 */

#define GF4_INV_TABLE { \
0x0,0x1,0x3,0x2 \
}
#define GF4_POLYNOMIAL_DIV_TABLE { \
{0x0,0x0}, \
{0x1,0x2}, \
{0x2,0x0}, \
{0x3,0x1} \
}
#define GF4_MUL_TABLE { \
{0x0,0x0,0x0,0x0}, \
{0x0,0x1,0x2,0x3}, \
{0x0,0x2,0x3,0x1}, \
{0x0,0x3,0x1,0x2} \
}
