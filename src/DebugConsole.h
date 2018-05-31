/* $LastChangedDate: 2017-07-22 19:04:09 +0200 (Sat, 22 Jul 2017) $ */
/*
 DebugConsole.h : Fim virtual console display.

 (c) 2008-2017 Michele Martone

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
#ifndef FIM_CONSOLE_H
#define FIM_CONSOLE_H


#include <vector>
#include "fim.h"
#include "Var.h"
#include "Namespace.h"

#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	class DisplayDevice;
namespace fim
{
	class MiniConsole FIM_FINAL
#ifdef FIM_NAMESPACES
	:public Namespace
#endif /* FIM_NAMESPACES */
	{
		fim_char_t *buffer_;	// the raw console buffer
		fim_char_t **line_;	// the (displayed) line_ pointers array
		fim_char_t *bp_;	// pointer to the top of the buffer
		int  bsize_;	// the buffer size
		int  lsize_;	// the lines array size
		int  ccol_;	// the currently pointed column
		int  cline_;	// the line_ on the top of the buffer
		int  lwidth_;	// line width
		int  rows_;	// rows count
		int  scroll_;	// last line displayed (up to rows_)
		const CommandConsole& cc_;
		DisplayDevice *displaydevice_;
		public:
		MiniConsole(CommandConsole& cc, DisplayDevice *dd, int lw=48, int r=12); /* get rid of these default numerical constants! */
		virtual ~MiniConsole(void){}
		fim_err_t dump(void);	// non const due to user variables reaction
		fim_err_t grow(void);
		fim_err_t setRows(int nr);
		fim_err_t add(const fim_char_t * cso);
		fim_err_t add(const fim_byte_t* cso);
		fim_err_t reformat(int newlsize);
		fim_err_t do_dump(int amount)const;
		fim_err_t clear(void);
		fim_err_t scroll_down(void);
		fim_err_t scroll_up(void);
		virtual size_t byte_size(void)const;
#if FIM_USE_CXX11
		public:
		/* deleted member functions cannot be called not even by be'friend'ed clases */
		MiniConsole& operator= (const MiniConsole&rhs) = delete;
		MiniConsole(const MiniConsole& rhs) = delete;
#else /* FIM_USE_CXX11 */
		/* private member functions can be called even by be'friend'ed clases */
		private:
		MiniConsole& operator= (const MiniConsole&rhs){return *this;/* a nilpotent assignment */}
		MiniConsole(const MiniConsole& rhs) :
			Namespace(rhs),
			buffer_(FIM_NULL),
			line_(FIM_NULL),
			bp_(FIM_NULL),
			bsize_(0),
			lsize_(0),
			ccol_(0),
			cline_(0),
			lwidth_(0),
			rows_(0),
			scroll_(0),
			cc_(rhs.cc_),
			displaydevice_(rhs.displaydevice_)
			{/* this constructor should not be used */}
#endif /* FIM_USE_CXX11 */
		private:
		int line_length(int li);
		fim_err_t do_dump(int f, int l)const;
		fim_err_t do_dump(void)const;

		fim_err_t grow_lines(int glines);
		fim_err_t grow_buffer(int gbuffer);
		fim_err_t grow(int glines, int gbuffer);
		public:
		void setDisplayDevice(DisplayDevice *displaydevice){displaydevice_=displaydevice;}
	};
}
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */

#endif /* FIM_CONSOLE_H */

