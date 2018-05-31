/* $LastChangedDate: 2017-07-25 00:49:49 +0200 (Tue, 25 Jul 2017) $ */
/*
 Command.cpp :

 (c) 2007-2017 Michele Martone

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
#include "fim.h"
#include "Command.h"
namespace fim
{
	const fim_cmd_id & Command::cmd(void)const{return cmd_;}
	const fim::string & Command::getHelp(void)const{return help_;}
#if FIM_USE_CXX11
	fim::string Command::execute(const args_t&args)
	{
		assert(cmf_);
		return (cmf_)(args);
	}
#else /* FIM_USE_CXX11 */
	fim::string Command::execute(const args_t&args)
	{
		assert(browser_ && browserf_);
		return (browser_->*browserf_)(args); // Browser or CommandConsole or FimWindow
	}
	Command::Command(fim_cmd_id cmd, fim::string help, Browser *b, fim::string(Browser::*bf)(const args_t&)) :cmd_(cmd),help_(help),browserf_(bf),browser_(b) {}
#ifdef FIM_WINDOWS
	Command::Command(fim_cmd_id cmd, fim::string help, FimWindow *w, fim::string(FimWindow::*cf)(const args_t&)) :cmd_(cmd),help_(help),windowf_(cf),window_(w) {}
#endif /* FIM_WINDOWS */
	Command::Command(fim_cmd_id cmd, fim::string help, CommandConsole *c,fim::string(CommandConsole::*cf)(const args_t&)) :cmd_(cmd),help_(help),consolef_(cf),commandconsole_(c) {}
#endif /* FIM_USE_CXX11 */
} /* namespace fim */
