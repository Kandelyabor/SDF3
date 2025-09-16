/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   textdiff.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   Nov 15, 2011
 *
 *  Function        :
 *
 *  History         :
 *      15-11-11    :   Initial version.
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */
#ifndef __BASE_TEXT_DIFF_H__
#define __BASE_TEXT_DIFF_H__

#include "base/string/cstring.h"
CString TextDiff(CString &file_a, CString &file_b, char split = '\n');

#endif
