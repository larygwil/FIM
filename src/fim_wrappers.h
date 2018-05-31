/* $LastChangedDate: 2017-12-17 02:33:15 +0100 (Sun, 17 Dec 2017) $ */
/*
 fim_wrappers.h : Some wrappers

 (c) 2011-2017 Michele Martone

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef FIM_WRAPPERS_H
#define FIM_WRAPPERS_H
namespace fim
{
/* symbolic wrappers for memory handling calls */
#define fim_calloc(x,y) std::calloc((x),(y)) /* may make this routine aligned in the future */
#define fim_stralloc(x) (fim_char_t*) std::calloc((x),(1)) /* ensures that first char is NUL */
#define fim_malloc(x) std::malloc(x)
#define fim_free(x) std::free(x), x=FIM_NULL
#define fim_memset(x,y,z) std::memset(x,y,z)
#define fim_bzero(x,y) fim_memset(x,0,y) /* bzero has been made legacy by POSIX.2001 and deprecated since POSIX.2004 */
}
#endif /* FIM_WRAPPERS_H */
