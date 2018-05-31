/* $LastChangedDate: 2018-01-21 16:53:30 +0100 (Sun, 21 Jan 2018) $ */
/*
 CommandConsole.cpp : Fim console dispatcher

 (c) 2007-2018 Michele Martone

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
#include <sys/time.h>
#if FIM_EXPERIMENTAL_FONT_CMD
#include <dirent.h> // readdir
#endif /* FIM_EXPERIMENTAL_FONT_CMD */
#include <errno.h>

#ifdef FIM_USE_READLINE
#include "readline.h"
#endif /* FIM_USE_READLINE */

#include <sys/ioctl.h>

#include <signal.h>
#include <fstream>

#if HAVE_GET_CURRENT_DIR_NAME
#else /* HAVE_GET_CURRENT_DIR_NAME */
#if _BSD_SOURCE || _XOPEN_SOURCE >= 500
#include <unistd.h>		/* getcwd, as replacement for get_current_dir_name */ /* STDIN_FILENO */
#endif /* _BSD_SOURCE || _XOPEN_SOURCE >= 500 */
#endif /* HAVE_GET_CURRENT_DIR_NAME */

#if FIM_WANT_RAW_KEYS_BINDING
#define FIM_CNS_RAW_KEYS_MESG "If " FIM_CNS_EX_KSY_STRING " is at least two characters long and begins with 0 (zero), the integer number after the 0 will be treated as a raw keycode to bind the specified " FIM_CNS_EX_KSY_STRING " to." FIM_CNS_CMDSEP "Use the '" FIM_VID_VERBOSE_KEYS "' variable to discover (display device dependent) raw keys." 
#else /* FIM_WANT_RAW_KEYS_BINDING */
#define FIM_CNS_RAW_KEYS_MESG 
#endif /* FIM_WANT_RAW_KEYS_BINDING */

#define FIM_KEY_OFFSET '0'
#define FIM_UNLIMITED_INFO_STRINGS 1 /* Description line can be of any length */

extern fim_sys_int yyparse();
#if FIM_WANT_BACKGROUND_LOAD
#include <mutex>
#endif /* FIM_WANT_BACKGROUND_LOAD */
#define FIM_USE_STD_CHRONO FIM_USE_CXX14
#if FIM_WANT_BACKGROUND_LOAD || FIM_USE_STD_CHRONO 
#include <thread>
#include <chrono>
#endif /* */

#define FIM_CNS_IFELSE "if(expression){action;}['else'{action;}]"

namespace fim
{
#if ( FIM_FONT_MAGNIFY_FACTOR <= 0 )
    	extern fim_int fim_fmf_; /* FIXME */
#endif /* FIM_FONT_MAGNIFY_FACTOR */

	static  bool nochars(const fim_char_t *s)
	{
		/*
		 * true if the string is null or empty, false otherwise
		 */
		if(s==FIM_NULL)
			return true;
		while(*s && isspace(*s))
			++s;
		return *s=='\0'?true:false;
	}

	int CommandConsole::findCommandIdx(fim_cmd_id cmd)const
	{
		/*
		 * check whether cmd is a valid internal (registered) Fim command and returns index
		 */
		for(size_t i=0;i<commands_.size();++i) 
			if( commands_[i].cmd()==cmd)
				return i;
		return FIM_INVALID_IDX;
	}
/*
	Command &CommandConsole::findCommand(fim_cmd_id cmd)const
	{
		int idx=findCommandIdx(cmd);

		if(idx!=FIM_INVALID_IDX)
			return &(commands_[idx]);
		return FIM_NULL;
	}
*/
	fim::string CommandConsole::bind(const fim_key_t c, const fim_cls binding, const fim::string hstr)
	{
		/*
		 * binds keycode c to the action specified in binding
		 */
		fim::string rs("keycode ");
		string ksym(key_syms_[c]);

		if( ksym != FIM_CNS_EMPTY_STRING )
			ksym = " (keysym \"" + ksym + "\")";

		rs+=string(static_cast<fim_int>(c));
		rs+=ksym;
		if(bindings_.find(c) != bindings_.end())
			rs+=" successfully reassigned to \"";
		else
			rs+=" successfully assigned to \"";
		bindings_[c]=binding; /* this is the operation; what follows is only debug info */
		rs+=bindings_[c];
		rs+="\"\n";
		if(hstr!="")
			bindings_help_[c]=hstr;
		return rs;
	}

	fim::string CommandConsole::getBindingsList(void)const
	{
		/*
		 * collates all registered action bindings_ together in a single string
		 * */
		std::ostringstream oss;
		bindings_t::const_iterator bi;

		for( bi=bindings_.begin();bi!=bindings_.end();++bi)
		{
			oss << FIM_FLT_BIND" \"";
			key_syms_t::const_iterator ikbi=key_syms_.find(((*bi).first));
			if(ikbi!=key_syms_.end())
			       	oss << ikbi->second;
			oss << "\" \"" << ((*bi).second);
			if( bindings_help_.find((*bi).first) != bindings_help_.end() )
				oss << "\" # " << bindings_help_.find((*bi).first) -> second << "\n";
			else
				oss << "\"\n";
		}
		return oss.str();
	}

	fim::string CommandConsole::unbind(const fim::string& kfstr)
	{
		/*
		 * 	unbinds the action eventually bound to the first key name specified in args..
		 */
		fim_key_t key=FIM_SYM_NULL_KEY;
#ifdef FIM_WANT_RAW_KEYS_BINDING
		const fim_char_t* const kstr=kfstr.c_str();

		if(strlen(kstr)>=2 && isdigit(kstr[0]) && isdigit(kstr[1]))
		{
			key=atoi(kstr+1);
		}
		else
#endif /* FIM_WANT_RAW_KEYS_BINDING */
		{
			sym_keys_t::const_iterator kbi=sym_keys_.find(kfstr);
			if(kbi!=sym_keys_.end())
				key=sym_keys_[kfstr];
		}
		return unbind(key);
	}

	fim_key_t CommandConsole::find_keycode_for_bound_cmd(fim_cls binding)const
	{
		/*
		 * looks for a binding to 'cmd' and returns a string description for its bound key 
		 */
		bindings_t::const_iterator bi;
		aliases_t::const_iterator ai;
		fim_key_t key=FIM_SYM_NULL_KEY;

		for( bi=bindings_.begin();bi!=bindings_.end();++bi)
		{
			if(bi->second==binding)
			{
				key = bi->first;	
				goto ret;
			}
		}

		for( ai=aliases_.begin();ai!=aliases_.end();++ai)
		{
			if(ai->second.first==binding)
			{
				key = ai->first;	
				goto ret;
			}
		}
ret:		return key;
	}

	fim::string CommandConsole::find_key_for_bound_cmd(fim_cls cmd)const
	{
		fim_key_t key = find_keycode_for_bound_cmd(cmd);

		if( key != FIM_SYM_NULL_KEY )
		{
			key_syms_t::const_iterator ki = key_syms_.find(key);
			if( ki != key_syms_.end() )
				return ki->second;
		}

		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::unbind(fim_key_t c)
	{
		/*
		 * unbinds the action eventually bound to the key combination code c
		 */
		std::ostringstream oss;
		bindings_t::const_iterator bi=bindings_.find(c);

		if( bi != bindings_.end() )
		{
#if FIM_USE_CXX11
			bindings_.erase(bi);
#else /* FIM_USE_CXX11 */
			bindings_.erase(c);
#endif /* FIM_USE_CXX11 */
			oss << c << ": successfully unbound.\n";
			bindings_help_.erase(c); // if key c existent, erases associated help msg.
		}
		else
			oss << c << ": there is not such binding.\n";
		return oss.str();
	}

	fim::string CommandConsole::aliasRecall(fim_cmd_id cmd)const
	{
		// returns the expanded alias specified by cmd
		aliases_t::const_iterator ai=aliases_.find(cmd);

		if(ai!=aliases_.end())
		       	return ai->second.first;
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::getAliasesList(void)const
	{
		// collates all expanded aliases
		fim::string aliases_expanded;
		aliases_t::const_iterator ai;

		for( ai=aliases_.begin();ai!=aliases_.end();++ai)
			aliases_expanded+=get_alias_info((*ai).first);
		return aliases_expanded;
	}

	fim::string CommandConsole::get_alias_info(const fim::string aname)const
	{
		std::ostringstream oss;
		oss << FIM_FLT_ALIAS" \"" << aname << "\" \"";

		aliases_t::const_iterator ai=aliases_.find(aname);
		if(ai!=aliases_.end())
			oss << ai->second.first;
		oss << "\"";
		if(ai!=aliases_.end())
		if(ai->second.second!=FIM_CNS_EMPTY_STRING)
			oss << " # " << ai->second.second;
		oss << fim::string("\n");
		return oss.str();
	}

	fim_cxr CommandConsole::fcmd_alias(const args_t& args)
	{
		// create an alias
		fim::string cmdlist,desc;
		std::ostringstream r;

		if(args.size()==0)
		{
			return getAliasesList();
		}
		if(args.size()<2)
		{
			return get_alias_info(args[0]);
		}
		if(args.size()>=2)
			cmdlist+=args[1];
		if(args.size()>=3)
			desc   +=args[2];
		if(aliases_[args[0]].first!=FIM_CNS_EMPTY_STRING)
		{
			aliases_[args[0]]=std::pair<fim_cmd_id,fim::string>(cmdlist,desc);
			r << FIM_FLT_ALIAS " " << args[0] << " successfully replaced.\n";
		}
		else
		{
			aliases_[args[0]].first=cmdlist;
			aliases_[args[0]].second=desc;
			r << FIM_FLT_ALIAS " " << args[0]<< " successfully added.\n";
		}
		return r.str();
	}

	fim::string CommandConsole::dummy(const args_t & args)
	{
		return "dummy function for test purposes\n";
	}

	CommandConsole::CommandConsole(void):
	Namespace(this),
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
#ifndef FIM_KEEP_BROKEN_CONSOLE
	mc_(*this,FIM_NULL),
#endif /* FIM_KEEP_BROKEN_CONSOLE */
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
	fontserver_(),
	show_must_go_on_(1),
	return_code_(0)
	,mangle_tcattr_(false)
	,browser_(*this)
	,cycles_(0)
#ifdef FIM_RECORDING
	,recordMode_(Normal)
	,dont_record_last_action_(false)
#endif /* FIM_RECORDING */
	,fim_stdin_(STDIN_FILENO)
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	,dummydisplaydevice_(this->mc_)
#else /* FIM_WANT_NO_OUTPUT_CONSOLE */
	,dummydisplaydevice_()
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
	,displaydevice_(FIM_NULL)
	,oldcwd_(fim_getcwd())
	{
		mc_.setDisplayDevice(&dummydisplaydevice_);
		addCommand(Command(FIM_FLT_ALIAS,FIM_CMD_HELP_ALIAS,this,&CommandConsole::fcmd_foo));
		addCommand(Command(FIM_FLT_ALIGN,FIM_CMD_HELP_ALIGN,&browser_,&Browser::fcmd_align));
#ifdef FIM_AUTOCMDS
		addCommand(Command(FIM_FLT_AUTOCMD,FIM_CMD_HELP_AUTOCMD,this,&CommandConsole::fcmd_autocmd));
		addCommand(Command(FIM_FLT_AUTOCMD_DEL,FIM_CMD_HELP_AUTOCMD_DEL,this,&CommandConsole::fcmd_autocmd_del));
#endif /* FIM_AUTOCMDS */
		addCommand(Command(FIM_FLT_BASENAME,FIM_CMD_HELP_BASENAME,this,&CommandConsole::fcmd_basename));
		addCommand(Command(FIM_FLT_BIND,FIM_CMD_HELP_BIND,this,&CommandConsole::fcmd_bind));
		addCommand(Command(FIM_FLT_CD,FIM_CMD_HELP_CD,this,&CommandConsole::fcmd_cd));
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
		addCommand(Command(FIM_FLT_CLEAR,FIM_CMD_HELP_CLEAR,this,&CommandConsole::fcmd_clear));
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
		addCommand(Command(FIM_FLT_COMMANDS,FIM_CMD_HELP_COMMANDS,this,&CommandConsole::fcmd_commands_list));
		addCommand(Command(FIM_FLT_COLOR,FIM_CMD_HELP_COLOR,&browser_,&Browser::fcmd_color));
		addCommand(Command(FIM_FLT_DESC,FIM_FLT_HELP_DESC,this,&CommandConsole::fcmd_desc));
		addCommand(Command(FIM_FLT_DISPLAY,FIM_FLT_HELP_DISPLAY,this,&CommandConsole::fcmd_display));
		addCommand(Command(FIM_FLT_DUMP_KEY_CODES,FIM_CMD_HELP_DUMP_KEY_CODES,this,&CommandConsole::fcmd_dump_key_codes));
		addCommand(Command(FIM_FLT_ECHO,FIM_CMD_HELP_ECHO,this,&CommandConsole::fcmd_echo));
		addCommand(Command(FIM_FLT_ELSE,FIM_CMD_HELP_ELSE,this,&CommandConsole::fcmd_foo));
#ifdef FIM_RECORDING
		addCommand(Command(FIM_FLT_EVAL,FIM_CMD_HELP_EVAL,this,&CommandConsole::fcmd_eval));
#endif /* FIM_RECORDING */
#ifndef FIM_WANT_NOSCRIPTING
		addCommand(Command(FIM_FLT_EXEC,FIM_CMD_HELP_EXEC,this,&CommandConsole::fcmd_executeFile));
#endif /* FIM_WANT_NOSCRIPTING */
#if FIM_EXPERIMENTAL_FONT_CMD
		addCommand(Command(FIM_FLT_FONT,FIM_CMD_HELP_FONT,this,&CommandConsole::fcmd_font));
#endif /* FIM_EXPERIMENTAL_FONT_CMD */
		addCommand(Command(FIM_FLT_GETENV,FIM_CMD_HELP_GETENV,this,&CommandConsole::fcmd_do_getenv));
		addCommand(Command(FIM_FLT_GOTO,FIM_CMD_HELP_GOTO,&browser_,&Browser::fcmd_goto));
		addCommand(Command(FIM_FLT_HELP,FIM_CMD_HELP_HELP,this,&CommandConsole::fcmd_help));
		addCommand(Command(FIM_FLT_IF,FIM_CMD_HELP_IFELSE,this,&CommandConsole::fcmd_foo));
		addCommand(Command(FIM_FLT_INFO,FIM_CMD_HELP_INFO,&browser_,&Browser::fcmd_info));
#if FIM_WANT_PIC_LBFL
		addCommand(Command(FIM_FLT_LIMIT,fim::string(FIM_FLT_LIMIT " "
#if FIM_WANT_PIC_LVDN
		" {'-list'|'-listall'} 'variable'|"
#endif /* FIM_WANT_PIC_LVDN */
		"['-further'|'-merge'|'-subtract'] [{expression} |{variable} {value}]: A browsable file list filtering function (like limiting in the \'mutt\' program). Uses information loaded via --" FIM_OSW_LOAD_IMG_DSC_FILE "."
#if FIM_WANT_PIC_LVDN
		FIM_CNS_CMDSEP " If invoked with '-list'/'-listall' only, will list the current description variable ids."
		FIM_CNS_CMDSEP " If invoked with '-list'/'-listall' 'id', will list set values for the variable 'id'."
#endif /* FIM_WANT_PIC_LVDN */
		FIM_CNS_CMDSEP " If '-further' is present, will start with the current list; if not, with the full list."
		FIM_CNS_CMDSEP " If '-merge' is present, new matches will be merged in the existing list and sorted."
		FIM_CNS_CMDSEP " If '-subtract' is present, sort and filter out matches."
		FIM_CNS_CMDSEP " If {variable} and {values} are provided, limit to files having property {variable} set to {value}."
#if FIM_WANT_FILENAME_MARK_AND_DUMP
		FIM_CNS_CMDSEP " If {expression} is one exclamation point ('!'), will limit to the currently marked files only."
#endif /* FIM_WANT_FILENAME_MARK_AND_DUMP */
#if FIM_WANT_LIMIT_DUPBN
		FIM_CNS_CMDSEP " If {expression} is '~!' will limit to files with unique basename."
		FIM_CNS_CMDSEP " if '~=', to files with duplicate basename."
		FIM_CNS_CMDSEP " if '~^', to the first of the files with duplicate basename."
		FIM_CNS_CMDSEP " if '~$', to the last of the files with duplicate basename."
#endif /* FIM_WANT_LIMIT_DUPBN */
		FIM_CNS_CMDSEP " On '~i' [MINIDX][-][MAXIDX], (each a number eventually followed by a multiplier 'K') will limit on filenames in position MINIDX to MAXIDX."
#if FIM_WANT_FLIST_STAT 
		FIM_CNS_CMDSEP " On '~z' will limit to files having the current file's size."
		FIM_CNS_CMDSEP " on '~z' [MINSIZE][-][MAXSIZE], (each a number eventually followed by a multiplier among 'k','K','m','M') will limit on filesize within these limits."
		FIM_CNS_CMDSEP " on '~d' will limit to files having the current file's date +- one day."
		FIM_CNS_CMDSEP " on '~d' [MINTIME][-][MAXTIME], (each the count of seconds since the Epoch (First of Jan. of 1970) or a date as DD/MM/YYYY) will limit on file time (struct stat's 'st_mtime', in seconds) within this interval."
#endif /* FIM_WANT_FLIST_STAT */
		FIM_CNS_CMDSEP " For other values of {expression}, limit to files whose description string matches {expression}."
		FIM_CNS_CMDSEP " Invoked with no arguments, the original browsable files list is restored." ),&browser_,&Browser::fcmd_limit));
#endif /* FIM_WANT_PIC_LBFL */
		addCommand(Command(FIM_FLT_LIST,FIM_CMD_HELP_LIST,&browser_,&Browser::fcmd_list));
		addCommand(Command(FIM_FLT_LOAD,FIM_CMD_HELP_LOAD,&browser_,&Browser::fcmd_load));
		addCommand(Command(FIM_FLT_PAN,FIM_CMD_HELP_PAN,&browser_,&Browser::fcmd_pan));
		addCommand(Command(FIM_FLT_POPEN,FIM_CMD_HELP_POPEN,this,&CommandConsole::fcmd_sys_popen));
#ifdef FIM_PIPE_IMAGE_READ
		addCommand(Command(FIM_FLT_PREAD,FIM_CMD_HELP_PREAD,this,&CommandConsole::fcmd_pread));
#endif /* FIM_PIPE_IMAGE_READ */
		addCommand(Command(FIM_FLT_PREFETCH,FIM_CMD_HELP_PREFETCH,&browser_,&Browser::fcmd_prefetch));
		addCommand(Command(FIM_FLT_PWD,FIM_CMD_HELP_PWD,this,&CommandConsole::fcmd_pwd));
		addCommand(Command(FIM_FLT_QUIT,FIM_CMD_HELP_QUIT,this,&CommandConsole::fcmd_quit));
#ifdef FIM_RECORDING
		addCommand(Command(FIM_FLT_RECORDING,FIM_CMD_HELP_RECORDING,this,&CommandConsole::fcmd_recording));
#endif /* FIM_RECORDING */
		addCommand(Command(FIM_FLT_REDISPLAY,FIM_CMD_HELP_REDISPLAY  ,this,&CommandConsole::fcmd_redisplay));
		addCommand(Command(FIM_FLT_RELOAD,FIM_CMD_HELP_RELOAD,&browser_,&Browser::fcmd_reload));
		addCommand(Command(FIM_FLT_ROTATE,FIM_CMD_HELP_ROTATE,&browser_,&Browser::fcmd_rotate));
		addCommand(Command(FIM_FLT_SCALE,FIM_CMD_HELP_SCALE,&browser_,&Browser::fcmd_scale));
		addCommand(Command(FIM_FLT_SCROLL,FIM_CMD_HELP_SCROLL,&browser_,&Browser::fcmd_scroll));
		addCommand(Command(FIM_FLT_SET,FIM_CMD_HELP_SET,this,&CommandConsole::fcmd_set));
		addCommand(Command(FIM_FLT_SET_CONSOLE_MODE,FIM_CMD_HELP_SET_CONSOLE_MODE,this,&CommandConsole::fcmd_set_in_console));
		addCommand(Command(FIM_FLT_SET_INTERACTIVE_MODE,FIM_CMD_HELP_SET_INTERACTIVE_MODE,this,&CommandConsole::fcmd_set_interactive_mode));
		addCommand(Command(FIM_FLT_SLEEP,FIM_CMD_HELP_SLEEP,this,&CommandConsole::fcmd_foo));
		addCommand(Command(FIM_FLT_STATUS,FIM_CMD_HELP_STATUS,this,&CommandConsole::fcmd_status));
		addCommand(Command(FIM_FLT_STDOUT,FIM_CMD_HELP_STDOUT ,this,&CommandConsole::fcmd__stdout));
#ifndef FIM_NO_SYSTEM
		addCommand(Command(FIM_FLT_SYSTEM,FIM_CMD_HELP_SYSTEM,this,&CommandConsole::fcmd_system));
#endif /* FIM_NO_SYSTEM */
		addCommand(Command(FIM_FLT_VARIABLES,FIM_CMD_HELP_VARIABLES,this,&CommandConsole::fcmd_variables_list));
		addCommand(Command(FIM_FLT_UNALIAS,FIM_CMD_HELP_UNALIAS,this,&CommandConsole::fcmd_unalias));
		addCommand(Command(FIM_FLT_UNBIND,FIM_CMD_HELP_UNBIND,this,&CommandConsole::fcmd_unbind));
		addCommand(Command(FIM_FLT_WHILE,FIM_CMD_HELP_WHILE,this,&CommandConsole::fcmd_foo));/* may introduce a special "help grammar" command */
#ifdef FIM_WINDOWS
		/* this is a stub for the manual generation (actually, the FimWindow object gets built later) */
		addCommand(Command(FIM_FLT_WINDOW,FIM_CMD_HELP_WINDOW,this,&CommandConsole::fcmd_foo));
#endif /* FIM_WINDOWS */
		execDefaultConfiguration();
		fcmd_cd(args_t());
		setVariable(FIM_VID_VERSION,FIM_REVISION_NUMBER);
		setVariable(FIM_VID_STEPS,FIM_CNS_STEPS_DEFAULT);
		setVariable(FIM_VID_TERM, fim_getenv(FIM_CNS_TERM_VAR));
		setVariable(FIM_VID_LOAD_DEFAULT_ETC_FIMRC,1);
		setVariable(FIM_VID_DEFAULT_ETC_FIMRC,FIM_CNS_SYS_RC_FILEPATH);
		setVariable(FIM_VID_PRELOAD_CHECKS,1);
		*prompt_=*(prompt_+1)=FIM_SYM_CHAR_NUL;
	}

	fim_err_t CommandConsole::execDefaultConfiguration(void)
	{
		#include "defaultConfiguration.cpp"
		return FIM_ERR_NO_ERROR; 
	}

        bool CommandConsole::is_file(fim::string nf)const
        {
		/*
		 * consider using access() as an alternative.
		 */
#if HAVE_SYS_STAT_H
                struct stat stat_s;

                if(-1==stat(nf.c_str(),&stat_s))
			goto err;
                if( S_ISDIR(stat_s.st_mode))
			goto err;
#endif /* HAVE_SYS_STAT_H */
                return true;
err:
                /* if the file doesn't exist, return false */
		return false;
        }

	fim_err_t CommandConsole::addCommand(Command c)
	{
		int idx=findCommandIdx(c.cmd());

		if(idx!=FIM_INVALID_IDX)
			commands_[idx]=c;
		else
			commands_.push_back(c);
		return FIM_ERR_NO_ERROR; 
	}

	fim::string CommandConsole::alias(const fim_cmd_id& a, const fim_cmd_id& c, const fim_cmd_id& d)
	{
		/*
		 * an internal alias member function
		 */
#if FIM_USE_CXX11
		return fcmd_alias({a,c,d});
#else /* FIM_USE_CXX11 */
		args_t args;
		args.push_back(a);
		args.push_back(c);
		args.push_back(d);
		return fcmd_alias(args);
#endif /* FIM_USE_CXX11 */
	}

	fim_char_t * CommandConsole::command_generator (const fim_char_t *text,int state,int mask)const
	{
		// inefficient. should cache results and skip repeated searches.
		args_t completions;
		aliases_t::const_iterator ai;
		variables_t::const_iterator vi;
		static size_t list_index=0;
		fim_char_t nschar='\0';

		if(state==0)
			list_index=0;
		while(isdigit(*text))
			text++;	//initial  repeat match
		if(!*text)
			return FIM_NULL;
		if(string(text).re_match(FIM_SYM_NAMESPACE_REGEX)==true)
		{
			mask=4,
			nschar=text[0],
			text+=2;
		}
		const fim_cmd_id cmd(text);
		if(mask==0 || (mask&1))
		for(size_t i=0;i<commands_.size();++i)
		{
			if(commands_[i].cmd().find(cmd)==0)
			completions.push_back(commands_[i].cmd());
		}
		if(mask==0 || (mask&2))
		for( ai=aliases_.begin();ai!=aliases_.end();++ai)
		{	
			if((ai->first).find(cmd)==0){
			completions.push_back(ai->first);}
		}
		if(mask==0 || (mask&4))
		{
			if(nschar==FIM_SYM_NULL_NAMESPACE_CHAR || nschar==FIM_SYM_NAMESPACE_GLOBAL_CHAR)
			for( vi=variables_.begin();vi!=variables_.end();++vi)
			{
				if((vi->first).find(cmd)==0)
					completions.push_back((*vi).first);
			}
			if(browser_.c_getImage())
			if(nschar==FIM_SYM_NULL_NAMESPACE_CHAR || nschar==FIM_SYM_NAMESPACE_IMAGE_CHAR)
				browser_.c_getImage()->find_matching_list(cmd,completions,true);
			if(nschar==FIM_SYM_NULL_NAMESPACE_CHAR || nschar==FIM_SYM_NAMESPACE_BROWSER_CHAR)
				browser_.find_matching_list(cmd,completions,true);
#ifdef FIM_WINDOWS
			if(current_window().current_viewportp())
			if(nschar==FIM_SYM_NULL_NAMESPACE_CHAR || nschar==FIM_SYM_NAMESPACE_VIEWPORT_CHAR)
				current_window().current_viewportp()->find_matching_list(cmd,completions,true);
			if(nschar==FIM_SYM_NULL_NAMESPACE_CHAR || nschar==FIM_SYM_NAMESPACE_WINDOW_CHAR)
				current_window().find_matching_list(cmd,completions,true);
#endif
		}
#ifndef FIM_COMMAND_AUTOCOMPLETION
		sort(completions.begin(),completions.end());
#endif /* FIM_COMMAND_AUTOCOMPLETION */
		
		if(list_index<completions.size())
			//readline frees this string.
			return dupstr(completions[list_index++].c_str());
		return FIM_NULL;
	}

#define istrncpy(x,y,z) {strncpy(x,y,z-1);x[z-1]='\0';}
#define ferror(s) {/*fatal error*/FIM_FPRINTF(stderr, "%s,%d:%s(please submit this error as a bug!)\n",__FILE__,__LINE__,s);}/* temporarily, for security reason: no exceptions launched */
//#define ferror(s) {/*fatal error*/FIM_FPRINTF(stderr, "%s,%d:%s(please submit this error as a bug!)\n",__FILE__,__LINE__,s);throw FIM_E_TRAGIC;}

	fim::string CommandConsole::getBoundAction(const fim_key_t c)const
	{
		bindings_t::const_iterator bi=bindings_.find(c);

		if(bi!=bindings_.end()) 
			return bi->second;
		else
		       	return FIM_CNS_EMPTY_RESULT;
	}

	bool CommandConsole::executeBinding(const fim_key_t c)
	{
		/*
		 *	Executes a command, without logging / recording.
		 *	If binding inexistent, ignores silently error and return false.
		 */
		bindings_t::const_iterator bi=bindings_.find(c);
		fim_err_t status=FIM_ERR_NO_ERROR;
#ifdef FIM_ITERATED_COMMANDS
		static fim_int it_buf=-1; /* FIXME: make this it_buf_ instead. */

		if( c>='0' && c <='9' && (bi==bindings_.end() || bi->second==FIM_CNS_EMPTY_STRING))//a number, not bound
		{
			if(it_buf>0)
			{
				fim_int nit_buf = it_buf;
				it_buf*=10;
				it_buf+=c - FIM_KEY_OFFSET;
				if( it_buf < nit_buf )
					it_buf = nit_buf;
			}
			else
			       	it_buf=c - FIM_KEY_OFFSET;
			goto ret;
		}
		if(c==FIM_SYM_NULL_KEY) // this branch happens to be entered in SDL mode. investigate way to avoid this check.
			goto ret;
#endif /* FIM_ITERATED_COMMANDS */

		if(bi!=bindings_.end() && bi->second!=FIM_CNS_EMPTY_STRING)
		{
			FIM_AUTOCMD_EXEC_PRE(FIM_ACM_PREINTERACTIVECOMMAND,current());
#ifdef FIM_ITERATED_COMMANDS
			if(it_buf>1)
			{
				fim_int mit = getIntVariable(FIM_VID_MAX_ITERATED_COMMANDS);
				fim::string nc;

				if(mit>0 && it_buf > mit)
				{
					cout << "Command repeat parameter of " << it_buf << " exceeds the maximum allowed value of " << mit << ". You can adjust " FIM_VID_MAX_ITERATED_COMMANDS " to raise this limit.\n";
					it_buf = FIM_MIN(mit,it_buf);
				}
				nc=it_buf;
				if(it_buf>1)
					nc+="{"+getBoundAction(c)+"}";
					/* adding ; before } can cause problems as long as ;; is not supported by the parser*/
				else
					nc = getBoundAction(c);
					
				cout << "About to execute " << nc << " .\n";
				status=execute_internal(nc.c_str(),FIM_X_HISTORY);
				it_buf=-1;
			}
			else
#endif /* FIM_ITERATED_COMMANDS */
				status=execute_internal(getBoundAction(c).c_str(),FIM_X_NULL);
			if( show_must_go_on_ )
				FIM_AUTOCMD_EXEC_POST(FIM_ACM_POSTINTERACTIVECOMMAND);
		}

		if(status)
		{
			std::cerr << "error performing execute()\n";
			//show_must_go_on_=0;	/* we terminate interactive execution */
		}
		else
			return true;
ret:
		return false;
	}

	fim_err_t CommandConsole::execute_internal(const fim_char_t *ss, fim_xflags_t xflags)
	{
		try{
		/*
		 *	execute a string containing a fim script.
		 *	second argument specifies whether to add to history.
		 *
		 *	note: the pipe here opened shall be closed in the yyparse()
		 *	call, by the YY_INPUT macro (defined by me in lex.lex)
		 */
		fim_bool_t add_history_=(xflags&FIM_X_HISTORY)?true:false;
		/* fim_bool_t suppress_output_=(xflags&FIM_X_QUIET)?true:false; */
		fim_char_t *s=dupstr(ss);//this malloc is free
		int iret=0;
		int r =0;

		if(s==FIM_NULL)
		{
			std::cerr << "allocation problem!\n";
			//if(s==FIM_NULL){ferror("null command");return;}
			//assert(s);
			//this shouldn't happen
			//this->quit(0);
			return FIM_ERR_GENERIC;
		}
		if(errno)
		{
			//fim_perror("before pipe(fim_pipedesc)");
			//goto ret;
			fim_perror(FIM_NULL);// we need to clear errno
		}
		//we open a pipe with the lexer/parser
		r = pipe(fim_pipedesc);
		if(r!=0)
		{
			//strerror(errno);
			std::cerr << "error piping with the command interpreter ( pipe() gave "<< r<< " )\n";
			std::cerr << "the command was:\"" << ss << "\"\n";
			std::cerr << "we had: "<< aliases_.size()<< " aliases_\n";
//			std::exit(-1);
//			ferror("pipe error\n");
//   			cleanup();
			if(errno)
			{
				fim_perror("in pipe(fim_pipedesc)");
				goto ret;
			}
			return FIM_ERR_GENERIC;
		}
		//we write there our script or commands
		r=write(fim_pipedesc[1],s,strlen(s));
		if(errno)
		{
			fim_perror("in write(fim_pipedesc[1])");
			goto ret;
		}
		//we are done!
		if((size_t)r!=strlen(s))
		{
			ferror("write error");
    			cleanup();
			return FIM_ERR_GENERIC;
		} 
		for(fim_char_t *p=s;*p;++p)
			if(*p=='\n')
				*p=' ';
		iret=close(fim_pipedesc[1]);
		if(iret || errno)
		{
			fim_perror("in close(fim_pipedesc[1])");
			goto ret;
		}
		try
		{
			iret=yyparse();
		}
		catch	(FimException e)
		{
			if( e == FIM_E_TRAGIC || e == FIM_E_NO_MEM )
			       	this->quit( FIM_E_NO_MEM );
			else
			       	;	/* ]:-)> */
		}
		close(fim_pipedesc[0]);
		if(iret!=0 || errno!=0)
		{
			if(getIntVariable(FIM_VID_VERBOSE_ERRORS)==1)
			{
				// FIXME; the pipe descriptor is used in a bad way.
				std::cout << "When parsing: " << FIM_MSG_CONSOLE_LONG_LINE   << s << FIM_MSG_CONSOLE_LONG_LINE  << "\n";
				fim_perror("in yyparse()");
			}
			else
				fim_perror(FIM_NULL);
			// ignoring yyparse's errno: it may originate from any command!
			//goto ret;
		}

#ifdef FIM_USE_READLINE
		if(add_history_)
			if(nochars(s)==false)
				add_history(s);
#endif /* FIM_USE_READLINE */
ret:
			fim_free(s);
		}
		catch	(FimException e)
		{
			if( e == FIM_E_TRAGIC || true )
			       	return this->quit( FIM_E_TRAGIC );
		}
		return FIM_ERR_NO_ERROR;
	}

        fim::string CommandConsole::execute(fim_cmd_id cmd, args_t args)
	{
		/*
		 * Single tokenized commands with arguments.
		 */
		int cidx=FIM_INVALID_IDX;
		/* first determine whether cmd is an alias */
		fim::string ocmd=aliasRecall(cmd);

		if(ocmd!=FIM_CNS_EMPTY_STRING)
		{
			//an alias should be expanded. arguments are appended.
			std::ostringstream oss;
			cmd=ocmd;
			oss << ocmd;
			// pipe to the parser
			fim_sys_int r = pipe(fim_pipedesc),sl;
			if(r!=0)
			{ferror("pipe error\n");exit(-1);}
#ifndef			FIM_ALIASES_WITHOUT_ARGUMENTS
			for(size_t i=0;i<args.size();++i)
				oss << " \"" << args[i] << fim::string("\""); 
#endif			/* FIM_ALIASES_WITHOUT_ARGUMENTS */
			sl=strlen(oss.str().c_str());
			r=write(fim_pipedesc[1],oss.str().c_str(),sl);
			if(r!=sl)
			{ferror("pipe write error");exit(-1);} 
			
			/*
			 * now the yyparse macro YY_INPUT itself handles the closing of the pipe.
			 *
			 * in this way nested commands could not cause harm, because the pipe
			 * is terminated BEFORE executing the command, and reusing fim_pipedesc
			 * is harmless.
			 *
			 * before occurred multiple pipe creations, on the same descriptor buffer,
			 * resulting in a loss of the original descriptors on openings..
			 */
			close(fim_pipedesc[1]);
			fim_perror(FIM_NULL);//FIXME: shall use only one yyparse-calling function!
			yyparse();
			fim_perror(FIM_NULL);//FIXME: shall use only one yyparse-calling function!
			close(fim_pipedesc[0]);
			goto ok;
		}
		if(cmd==FIM_FLT_USLEEP)
		{
#if FIM_USE_STD_CHRONO
			using namespace std::chrono_literals;
			std::chrono::microseconds useconds = 1us;

			if(args.size()>0)
				useconds*=atoi(args[0].c_str());
			std::this_thread::sleep_for(useconds);
#else /* FIM_USE_STD_CHRONO */
			fim_tus_t useconds=1;
			
			if(args.size()>0)
				useconds=atoi(args[0].c_str());
			usleep(useconds);
#endif /* FIM_USE_STD_CHRONO */
			goto ok;
		}
		else
		if(cmd==FIM_FLT_SLEEP)
		{
			fim_ts_t seconds=1;

			if(args.size()>0)
				seconds=atoi(args[0].c_str());
#if 0
			sleep(seconds);
#else
			/* we want an interruptible sleep.  */
			//while(seconds>0 && catchLoopBreakingCommand(seconds--))sleep(1);
			catchLoopBreakingCommand(seconds);
#endif
			goto ok;
		}
		else
		if(cmd==FIM_FLT_ALIAS)
		{
			//assignment of an alias
			args_t aargs;

			for(size_t i=0;i<args.size();++i)
				aargs.push_back(args[i]);
			cout << this->fcmd_alias(aargs) << "\n";
			goto ok;
		}
		else
		{
			cidx=findCommandIdx(cmd);

#ifdef FIM_COMMAND_AUTOCOMPLETION
			if(getIntVariable(FIM_VID_CMD_EXPANSION)==1)
			if(cidx==FIM_INVALID_IDX)
			{
				fim_char_t *match = this->command_generator(cmd.c_str(),0,0);

				if(match)
				{
					//cout << "but found:`"<<match<<"...\n";
					cidx=findCommandIdx(match);
					fim_free(match);
				}
			}
#endif /* FIM_COMMAND_AUTOCOMPLETION */
			if(cidx==FIM_INVALID_IDX)
			{
				cout << "sorry, no such command:`"<<cmd.c_str()<<"'\n";
				goto ok;
			}
			else
			{
				if(getVariable(FIM_VID_DBG_COMMANDS).find('c') >= 0)
					std::cout << FIM_CNS_DBG_CMDS_PFX << "executing: " << cmd << " " << args << "\n";
				
				fim::string lco = commands_[cidx].execute(args);
				cout << lco;
				setVariable(FIM_VID_LAST_CMD_OUTPUT,lco);
				goto ok;
			}
		}
		return "If you see this string, please report it to the program maintainer.\n";
ok:
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_int CommandConsole::catchLoopBreakingCommand(fim_ts_t seconds)
	{
		/*	
		 *	Interactive loops breaking logic.
		 *	Allows user to press any key during loop.
		 *	Loop will continue its execution, unless pressed key is exitBinding_.
		 *	If not, and if the key is bound to some action, this action is executed.
		 *	returns 1 if loop has to be broken.
		 */
		fim_key_t c;

		if ( exitBinding_ == 0 )
		       	goto err;	/* any key triggers an exit */

		c = displaydevice_->catchInteractiveCommand(seconds);
		while(c!=-1)
		{
			/* while characters read */
			//if( c == -1 ) return 0;	/* no chars read */
			sym_keys_t::const_iterator ki;

			if((ki=sym_keys_.find(FIM_KBD_ESC))!=sym_keys_.end() && c==ki->second)
				goto err;
			if((ki=sym_keys_.find(FIM_KBD_COLON))!=sym_keys_.end() && c==ki->second)
				goto err;
			if( c != exitBinding_ )  /* characters read */
			{
				executeBinding(c); /* any interactive action */
				if(!show_must_go_on_)
					goto err;
				c = displaydevice_->catchInteractiveCommand(1);
			}
			if(c==exitBinding_)
			       	goto err;
		}
		return 0;	/* no chars read  */
err:
		return 1;	/* loop shall be broken */
	}
		
#ifdef	FIM_USE_GPM
	static int gh(Gpm_Event *event, void *clientdata)
	{
		std::cout << "GPM event captured.\n";
		exit(0);
		return 'n';
		//return 0;
	}
#endif	/* FIM_USE_GPM */

	fim_perr_t CommandConsole::executionCycle(void)
	{
		/*
		 * the cycle with fetches the instruction stream.
		 * */
#ifdef	FIM_USE_GPM
		Gpm_PushRoi(0,0,1023,768,GPM_DOWN|GPM_UP|GPM_DRAG|GPM_ENTER|GPM_LEAVE,gh,FIM_NULL);
#endif	/* FIM_USE_GPM */
		fim::string initial = browser_.current();
		FIM_AUTOCMD_EXEC(FIM_ACM_PREEXECUTIONCYCLE,initial);
		FIM_AUTOCMD_EXEC(FIM_ACM_PREEXECUTIONCYCLEARGS,initial);
		*prompt_=FIM_SYM_PROMPT_NUL;

#if FIM_WANT_BACKGROUND_LOAD
		background_push();
#endif /* FIM_WANT_BACKGROUND_LOAD */

#if FIM_WANT_CMDLINE_KEYPRESS
		for(size_t i=0;i<clkpv_.size();++i)
		if( const char * c_str = clkpv_[i].c_str() )
		{
			for(;isdigit(*c_str);++c_str)
				executeBinding( *c_str );
			if( *c_str )
				executeBinding( kstr_to_key( c_str ) );
		}
#endif /* FIM_WANT_CMDLINE_KEYPRESS */
	 	while(show_must_go_on_)
		{
			cycles_++;
#if 0
			/* dead code */
			// FIXME: document this
			fd_set          set;
			struct timeval  limit;
			FD_SET(0, &set);
			limit.tv_sec = -1;
			limit.tv_usec = 0;
#endif

#ifdef FIM_USE_READLINE
			if(ic_==1)
			{
				ic_=1;
				fim_char_t *rl = fim_readline(FIM_KBD_COLON);
				*prompt_=FIM_SYM_PROMPT_CHAR;
				if(rl==FIM_NULL)
				{
					goto rlnull;
				}
				else
				if(*rl!=FIM_SYM_CHAR_NUL)
				{
					/*
					 * This code gets executed when the user is about to exit console mode, 
					 * having she pressed the 'Enter' key and expecting result.
					 * */
					FIM_AUTOCMD_EXEC_PRE(FIM_ACM_PREINTERACTIVECOMMAND,current());
#ifdef FIM_RECORDING
					if(recordMode_==Recording)
						record_action(fim::string(rl));
#endif /* FIM_RECORDING */
					execute_internal(rl,FIM_X_HISTORY);	//execution of the command line with history
					ic_=(ic_==-1)?0:1; //a command could change the mode !
					if( show_must_go_on_ )
						FIM_AUTOCMD_EXEC_POST(FIM_ACM_POSTINTERACTIVECOMMAND);
#ifdef FIM_RECORDING
					memorize_last(rl);
#endif /* FIM_RECORDING */
				}

				if(rl && *rl==FIM_SYM_CHAR_NUL)
				{
					/* happens when no command is issued and Enter key is pressed */
					ic_=0;
					*(prompt_)=FIM_SYM_PROMPT_NUL;
					set_status_bar(FIM_CNS_EMPTY_STRING,FIM_NULL);
				}
				if(rl)
					fim_free(rl);
			}
			else
#endif /* FIM_USE_READLINE */
			{
#ifdef	FIM_USE_GPM
				Gpm_Event *EVENT = FIM_NULL;
#endif	/* FIM_USE_GPM */
				fim_key_t c;
				fim_sys_int r;
				fim_char_t buf[FIM_VERBOSE_KEYS_BUFSIZE];

				*prompt_ = FIM_SYM_PROMPT_NUL;
				c=0;
				r=displaydevice_->get_input(&c);
#ifdef	FIM_USE_GPM
				if(Gpm_GetEvent(EVENT)==1)
					quit();
				else
					cout << "...";
#endif	/* FIM_USE_GPM */
				if(r>0)
				{
					if(getIntVariable(FIM_VID_VERBOSE_KEYS)==1)
					{
						/*
						 * <0x20 ? print ^ 0x40+..
						 * */
						sprintf(buf,"got: 0x%x (%d)\n",c,c);
						cout << buf ;
					}
					if(getVariable(FIM_VID_DBG_COMMANDS).find('k') >= 0)
					{
						std::cout << FIM_CNS_DBG_CMDS_PFX << "keysym: '"<<c<<"' "
							<< fim_key_escape(key_syms_[c]) << "\n";
					}
#ifndef FIM_USE_READLINE
					if(c==(fim_key_t)getIntVariable(FIM_VID_CONSOLE_KEY) || 
						c == FIM_SYM_FW_SEARCH_KEY || c == FIM_SYM_BW_SEARCH_KEY )
						set_status_bar("compiled with no readline support!\n",FIM_NULL);
#else /* FIM_USE_READLINE */
					if(c==(fim_key_t)getIntVariable(FIM_VID_CONSOLE_KEY))
					{
						ic_=1;
						*prompt_ = FIM_SYM_PROMPT_CHAR;
					}
					else
					if( c == FIM_SYM_FW_SEARCH_KEY || c == FIM_SYM_BW_SEARCH_KEY )
					{
						/* a hack to handle vim-style regexp searches */
						fim_sys_int tmp=rl_filename_completion_desired;
						rl_hook_func_t *osh=rl_startup_hook;
						rl_startup_hook=rl::fim_search_rl_startup_hook;
						fim_char_t *rl = FIM_NULL;
						const fim_char_t *rlp = FIM_CNS_SLASH_STRING;
						*prompt_=FIM_SYM_PROMPT_SLASH;
						if(c == FIM_SYM_BW_SEARCH_KEY)
							rlp=FIM_CNS_QU_MA_STRING,
							*prompt_='?';
						rl=fim_readline(rlp); // !!
						ic_=1;
						rl_inhibit_completion=1;
						rl_startup_hook=osh;
						// no readline ? no interactive searches !
						*prompt_=FIM_SYM_PROMPT_NUL;
						rl_inhibit_completion=tmp;
						ic_=0;
						if(rl==FIM_NULL)
						{
							goto rlnull;
						}
						else
						if(*rl != FIM_SYM_CHAR_NUL)
						{
							args_t args;
							std::ostringstream rls;

							if(c == FIM_SYM_BW_SEARCH_KEY)
								rls << "-";
							rls << "/" << rl << "/";
							args.push_back(rls.str());
							execute(FIM_FLT_GOTO,args);
						}
					}
					else
#endif /* FIM_USE_READLINE */
					{

						if( this->executeBinding(c) )
						{
#ifdef FIM_RECORDING
							if(recordMode_==Recording)
							       	record_action(getBoundAction(c));
							memorize_last(getBoundAction(c));
#endif /* FIM_RECORDING */
						}
					}
				}
				else
				{
					// framebuffer console switching is handled here
					if(r==-1)
						redisplay(), // fbi used to redraw elsewhere
						browser_.display_status("");
				}
			}
			continue;
#ifdef FIM_USE_READLINE
rlnull:
			{
				ic_=0;
				*prompt_=FIM_SYM_CHAR_NUL;
				*prompt_=*(prompt_+1)=FIM_SYM_CHAR_NUL;
				const fim_char_t * const msg = " Warning: readline failed ! Probably no stdin is available, right ? Unfortunately fim is not yet ready for this case.\n";
				cout << msg;
				set_status_bar(msg,FIM_NULL);
			}
#endif /* FIM_USE_READLINE */
		}
#if FIM_WANT_BACKGROUND_LOAD
		blt_.join();
#endif /* FIM_WANT_BACKGROUND_LOAD */
		FIM_AUTOCMD_EXEC(FIM_ACM_POSTEXECUTIONCYCLE,initial);
		return quit(return_code_);
	}

	FIM_NORETURN void CommandConsole::exit(fim_perr_t i)const
	{
		/*
		 *	Exit the program as a whole.
		 *      If various object destructors are set to destroy device
		 *	contexts, it should do no harm to the console.
		 *      (it will call statically declared object's destructors )
		 */
		std::exit(i);
	}

	fim_perr_t CommandConsole::quit(fim_perr_t i)
	{
		/*
		 * To be called to exit from the program safely.
		 * it is used mainly for safe exit after severe errors.
		 */
		show_must_go_on_=0;
    		cleanup();
		return i;
	}

#if FIM_WANT_FILENAME_MARK_AND_DUMP
	fim::string CommandConsole::marked_files_list(void)const
	{
		std::ostringstream oss;
		for(std::set<fim::string>::iterator i=marked_files_.begin();i!=marked_files_.end();++i)
			oss << *i << "\n";
		return oss.str();
	}

	fim::string CommandConsole::marked_files_clear(void)
	{
		marked_files_.erase(marked_files_.begin(),marked_files_.end());
#if FIM_USE_CXX11
		return {};
#else /* FIM_USE_CXX11 */
		return fim::string();
#endif /* FIM_USE_CXX11 */
	}
#endif /* FIM_WANT_FILENAME_MARK_AND_DUMP */

	CommandConsole::~CommandConsole(void)
	{
		fim::string sof=getStringVariable(FIM_VID_SCRIPTOUT_FILE);

#if FIM_WANT_FILENAME_MARK_AND_DUMP
		if(!marked_files_.empty())
		{
			std::cerr << "The following files were marked by the user:\n";
			std::cerr << "\n";
			std::cout << marked_files_list();
		}
#endif /* FIM_WANT_FILENAME_MARK_AND_DUMP */
		if(sof!=FIM_CNS_EMPTY_STRING)
		{
        		if(is_file(sof))
			{
				std::cerr << "Warning: the "<<sof<<" file exists and will not be overwritten!\n";
			}
			else
			{
				std::ofstream out(sof.c_str());

				if(!out)
				{
					std::cerr << "Warning: The "<<sof<<" file could not be opened for writing!\n";
					std::cerr << "check output directory permissions and retry!\n";
				}
				else
				{
					out << dump_record_buffer(args_t()) << "\n";
					out.close();
				}
			}
		}

	#ifdef FIM_WITH_AALIB
		if(aad_ && !displaydevice_)
			delete aad_;	/* aad_ is an alias */
		else
	#endif /* FIM_WITH_AALIB */
		{
			if(displaydevice_ && displaydevice_ != &dummydisplaydevice_)
				delete displaydevice_;
		}

#ifdef FIM_WINDOWS
		if(window_)
			delete window_;
#else /* FIM_WINDOWS */
		if(viewport_)
		       	delete viewport_;
#endif /* FIM_WINDOWS */
	}

	fim::string CommandConsole::readStdFileDescriptor(FILE* fd, int*rp)
	{
		fim_sys_int r;
		fim_char_t buf[FIM_STREAM_BUFSIZE];	// note: might make this user configurable
		fim::string cmds;

		if(fd==FIM_NULL)
		{
			cmds = FIM_ERR_GENERIC;
			goto ret;
		}

		while((r=fread(buf,1,sizeof(buf)-1,fd))>0)
		{
			buf[r]='\0';
			cmds+=buf;
			/*		if(displaydevice_->catchInteractiveCommand(0)!=-1) goto ret; */
		}
		if(rp)
			*rp=r;
	ret:
		return cmds;
	}
		
	fim_err_t CommandConsole::executeStdFileDescriptor(FILE* fd)
	{
		fim_sys_int r;
		fim_err_t errv = FIM_ERR_NO_ERROR;
		fim::string cmds = CommandConsole::readStdFileDescriptor(fd,&r);

		if(r==-1)
		{
			errv = FIM_ERR_GENERIC;
			goto ret;
		}

		execute_internal(cmds.c_str(),FIM_X_QUIET);
ret:
		return errv;
	}

	fim_err_t CommandConsole::executeFile(const fim_char_t *s)
	{
		return execute_internal(slurp_file(s).c_str(),FIM_X_QUIET);
	}

	fim_var_t CommandConsole::getVariableType(const fim_var_id& varname)const
	{
		variables_t::const_iterator vi=variables_.find(varname);

		if(vi!=variables_.end())
			return vi->second.getType();
		else
		       	return FimTypeWrong;
	}

	bool CommandConsole::isVariable(const fim_var_id& varname)const
	{
		return getStringVariable(varname).size();
	}

	fim_bool_t CommandConsole::drawOutput(const fim_char_t *s)const
	{
		/*
		 * whether the console should draw or not itself upon the arrival of textual output
		 * */
		//std::cout << s << ": " << (this->inConsole() )<< ( (s&&*s) ) << "\n";
		fim_bool_t sd=(	(	this->inConsole()	/* in the command line */
				&& (s&&*s) 		/* actually some text to add */
			) 
			|| this->getIntVariable(FIM_VID_DISPLAY_CONSOLE)	/* or user requested for showing console */
			);
		return sd;
	}

	fim::string CommandConsole::get_aliases_list(void)const
	{
		/*
		 * returns the list of set action aliases
		 */
		std::ostringstream oss;
		aliases_t::const_iterator ai;

		for( ai=aliases_.begin();ai!=aliases_.end();++ai)
			oss << ((*ai).first) << " ";
		return oss.str();
	}

	fim::string CommandConsole::get_commands_list(void)const
	{
		std::ostringstream oss;

		for(size_t i=0;i<commands_.size();++i)
		{
			if(i)
				oss << " ";
			oss << (commands_.at(i).cmd());
		}
		return oss.str();
	}

	fim::string CommandConsole::get_variables_list(void)const
	{
		std::ostringstream acl;
		std::string sep=" ";
		variables_t::const_iterator vi;

		for( vi=variables_.begin();vi!=variables_.end();++vi)
			acl << ((*vi).first) << " ";

#ifdef FIM_NAMESPACES
		acl << browser_.get_variables_list() << sep;
		if(browser_.c_getImage())
			acl << browser_.c_getImage()->get_variables_list() << sep;
#endif /* FIM_NAMESPACES */
#ifdef FIM_WINDOWS
		if(window_)
			acl << window_->get_variables_list() << sep;
		if(current_viewport())
			acl << current_viewport()->get_variables_list() << sep;
#endif /* FIM_WINDOWS */
		return acl.str();
	}

#ifdef FIM_AUTOCMDS
	fim::string CommandConsole::autocmds_list(const fim::string event, const fim::string pattern)const
	{
		std::ostringstream acl;
		autocmds_t::const_iterator ai;

		if(event==FIM_CNS_EMPTY_STRING && pattern==FIM_CNS_EMPTY_STRING)
		//for each autocommand event registered
		for( ai=autocmds_.begin();ai!=autocmds_.end();++ai )
		//for each file pattern registered, display the list..
		for(	autocmds_p_t::const_iterator api=((*ai)).second.begin();
				api!=((*ai)).second.end();++api )
		//.. display the list of autocommands...
		for(	args_t::const_iterator aui=((*api)).second.begin();
				aui!=((*api)).second.end();++aui )
			acl << FIM_FLT_AUTOCMD" \""  << (*ai).first << "\" \"" << (*api).first << "\" \"" << (*aui)  << "\"\n";
		else
		if(pattern==FIM_CNS_EMPTY_STRING)
		{
			autocmds_t::const_iterator ai=autocmds_.find(event);
			//for each autocommand event registered
			//for each file pattern registered, display the list..
			if(ai!=autocmds_.end())
			for(	autocmds_p_t::const_iterator api=(*ai).second.begin();
					api!=(*ai).second.end();++api )
			//.. display the list of autocommands...
			for(	args_t::const_iterator aui=((*api)).second.begin();
					aui!=((*api)).second.end();++aui )
				acl << FIM_FLT_AUTOCMD" \"" << (*ai).first << "\" \"" << (*api).first << "\" \"" << (*aui) << "\"\n";
		}
		else
		{
			autocmds_t::const_iterator ai=autocmds_.find(event);

			//for each autocommand event registered
			//for each file pattern registered, display the list..
			if(ai!=autocmds_.end())
			{
				autocmds_p_t::const_iterator api=(*ai).second.find(pattern);

				//.. display the list of autocommands...
				if(api!=(*ai).second.end())
				{
					for(	args_t::const_iterator aui=((*api)).second.begin();
					aui!=((*api)).second.end();++aui )
						acl << FIM_FLT_AUTOCMD" \"" << (*ai).first << "\" \"" << (*api).first << "\" \"" << (*aui) << "\"\n"; 
				}
			}
		}
		
		if(acl.str()==FIM_CNS_EMPTY_STRING)
			acl << "no autocommands loaded\n";
		return acl.str();
	}

	fim::string CommandConsole::autocmd_del(const fim::string event, const fim::string pattern, const fim::string action)
	{
		autocmds_t::iterator ai;
		size_t n = 0;

		if(event==FIM_CNS_EMPTY_STRING && pattern==FIM_CNS_EMPTY_STRING  && action == FIM_CNS_EMPTY_STRING  )
		{
			/* deletion of all autocmd's */
			n = autocmds_.size();
			for( ai=autocmds_.begin();ai!=autocmds_.end();++ai )
				autocmds_.erase(ai);
		}
		else
		if(action==FIM_CNS_EMPTY_STRING   && pattern==FIM_CNS_EMPTY_STRING    )
		{
			/* deletion of all autocmd's for given event */
			ai=autocmds_.find(event);
			if(ai==autocmds_.end())
				return FIM_CNS_EMPTY_RESULT;
			n = (*ai).second.size();
			for(	autocmds_p_t::iterator api=((*ai)).second.begin();
				api!=((*ai)).second.end();++api )
				(*ai).second.erase(api);
		}
		else
		if(action==FIM_CNS_EMPTY_STRING)
		{
			/* deletion of all autocmd's for given event and pattern */
			ai=autocmds_.find(event);
			if(ai==autocmds_.end())
				return FIM_CNS_EMPTY_RESULT;
			autocmds_p_t::iterator api=((*ai)).second.find(pattern);
			n = (*api).second.size();
			for(	args_t::iterator aui=((*api)).second.begin();
					aui!=((*api)).second.end();++aui )
						(*api).second.erase(aui);
		}
		if(n)
			return n+" autocmd's removed\n";
		else
			return "no autocmd's removed\n";
	}

	fim::string CommandConsole::autocmd_add(const fim::string& event,const fim::string& pat,const fim_cmd_id& cmd)
	{
		/* valid vs invalid events? check and warning .. */
		if(cmd==FIM_CNS_EMPTY_STRING)
		{
			cout << "can't add empty autocommand\n";
			goto ok;
		}
		for(size_t i=0;i<autocmds_[event][pat].size();++i)
		if((autocmds_[event][pat][i])==cmd)
		{
			cout << "autocommand "<<cmd<<" already specified for event \""<<event<<"\" and pattern \""<<pat<<"\"\n";
			goto ok;
		}
		autocmds_[event][pat].push_back(cmd);
ok:
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::pre_autocmd_add(const fim_cmd_id& cmd)
	{
		if(getVariable(FIM_VID_DBG_COMMANDS).find('a') >= 0)
			std::cout << FIM_CNS_DBG_CMDS_PFX << "adding autocmd " << cmd << ":\n";
	    	return autocmd_add(FIM_ACM_POSTHFIMRC,"",cmd);
	}

	fim::string CommandConsole::pre_autocmd_exec(void)
	{
	    	return FIM_AUTOCMD_EXEC(FIM_ACM_POSTHFIMRC,"");
	}

	fim::string CommandConsole::autocmd_exec(const fim::string& event, const fim_fn_t& fname)
	{
		autocmds_p_t::const_iterator api;
		/*
		 * we want to prevent from looping autocommands; this rudimentary mechanism shall avoid them.
		 */
		autocmds_loop_frame_t frame(event,fname);

		if(! autocmd_in_stack( frame ))
		{
			autocmd_push_stack( frame );
			for( api=autocmds_[event].begin();api!=autocmds_[event].end();++api )
 				autocmd_exec(event,(*api).first,fname);
			autocmd_pop_stack( frame );
		}
		else
		{
			cout << "WARNING: there is a loop for "
			     << "(event:" << event << ",filename:" << fname << ")";
		}
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::autocmd_exec(const fim::string& event, const fim::string& pat, const fim_fn_t& fname)
	{
//		cout << "autocmd_exec_cmd...\n";
//		cout << "autocmd_exec_cmd. for pat '" << fname <<  "'\n";

		if(getIntVariable(FIM_VID_DBG_AUTOCMD_TRACE_STACK)!=0)
			autocmd_trace_stack();
			
		if(regexp_match(fname.c_str(),pat.c_str(),getIntVariable(FIM_VID_IGNORECASE)))
		{
			for (size_t i=0;i<autocmds_[event][pat].size();++i)
			{
				autocmds_frame_t frame(autocmds_loop_frame_t(event,fname),(autocmds_[event][pat][i]).c_str());
				if(getVariable(FIM_VID_DBG_COMMANDS).find('a') >= 0)
					std::cout << FIM_CNS_DBG_CMDS_PFX << "autocmd: '"<<event<<"'->'"<<fim_shell_arg_escape(autocmds_[event][pat][i])<<"'\n";
				autocmds_stack.push_back(frame);
				execute_internal((autocmds_[event][pat][i]).c_str(),FIM_X_QUIET);
				autocmds_stack.pop_back();
			}
		}
		return FIM_CNS_EMPTY_RESULT;
	}

	void CommandConsole::autocmd_push_stack(const autocmds_loop_frame_t& frame)
	{
		autocmds_loop_stack.push_back(frame);
	}

	void CommandConsole::autocmd_pop_stack(const autocmds_loop_frame_t& frame)
	{
		autocmds_loop_stack.pop_back();
	}
	
	void CommandConsole::autocmd_trace_stack(void)
	{
		/*
		 * this is mainly a debug function: it will write to stdout
		 * the current autocommands stack trace.
		 * set the FIM_VID_DBG_AUTOCMD_TRACE_STACK variable
		 */
		size_t indent=0,i;

		if(autocmds_stack.end()==autocmds_stack.begin())
			std::cout << "<>\n";
		for(
			autocmds_stack_t::const_iterator citer=autocmds_stack.begin();
			citer!=autocmds_stack.end();++citer,++indent )
			{
				for(i=0;i<indent+1;++i) std::cout << " ";
				std::cout
					<< citer->first.first << " "
					<< citer->first.second << " "
					<< citer->second << "\n";
			}
	}
	
	fim_bool_t CommandConsole::autocmd_in_stack(const autocmds_loop_frame_t& frame)const
	{
		/*
		 * prevents a second autocommand triggered against the same file to execute
		 */
		return  find(autocmds_loop_stack.begin(),autocmds_loop_stack.end(),frame)!=autocmds_loop_stack.end();
	}
#endif /* FIM_AUTOCMDS */
	
	bool CommandConsole::regexp_match(const fim_char_t*s, const fim_char_t*r, int rsic)const
	{
		/*
		 *	given a string s, and a Posix regular expression r, this
		 *	member function returns true if there is match. false otherwise.
		 */
		regex_t regex;		//should be static!!!
		const fim_size_t nmatch=1;	// we are satisfied with the first match, aren't we ?
		regmatch_t pmatch[nmatch];

		/*
		 * we allow for the default match, in case of null regexp
		 */
		if(!r || !strlen(r))
			return true;

		/* fixup code for a mysterious bug
		 */
		if(*r=='*')
			return false;

		//if(regcomp(&regex,"^ \\+$", 0 | REG_EXTENDED | REG_ICASE )==-1)
		if(regcomp(&regex,r, 0 | REG_EXTENDED | (rsic==0?0:REG_ICASE) )!=0)
		{
			/* error calling regcomp (invalid regexp?)! (should we warn the user ?) */
			//cout << "error calling regcomp (invalid regexp?)!" << "\n";
			return false;
		}
		else
		{
//			cout << "done calling regcomp!" << "\n";
		}
		//if(regexec(&regex,s+0,nmatch,pmatch,0)==0)
		if(regexec(&regex,s+0,nmatch,pmatch,0)!=REG_NOMATCH)
		{
//			cout << "'"<< s << "' matches with '" << r << "'\n";
/*			cout << "match: " << "\n";
			cout << "\"";
			for(int m=pmatch[0].rm_so;m<pmatch[0].rm_eo;++m)
				cout << s[0+m];
			cout << "\"\n";*/
			regfree(&regex);
			return true;
		}
		else
		{
			/*	no match	*/
		}
		regfree(&regex);
		return false;
		//return true;
	}

	bool CommandConsole::redisplay(void)
	{
		bool needed_redisplay=false;

		if(window_)
			needed_redisplay = window_->recursive_redisplay();
		return needed_redisplay;
	}

#ifdef FIM_RECORDING
	void CommandConsole::record_action(const fim_cmd_id& cmd)
	{
		static time_t pt=0;
		time_t ct,dt;
	        struct timeval tv;

		if(cmd==FIM_CNS_EMPTY_STRING)
		{
			pt=0;
			return;
		}

		if(gettimeofday(&tv, FIM_NULL)!=0)
			// error case.
			return;

		ct=tv.tv_usec/1000+tv.tv_sec*1000;

	        if(!pt)
			pt=ct;

		dt=(ct-pt)*1000;
		pt=ct;
		recorded_actions_.push_back(recorded_action_t(sanitize_action(cmd),dt));
	}
#endif /* FIM_RECORDING */

#if FIM_WANT_FILENAME_MARK_AND_DUMP
	bool CommandConsole::isMarkedFile(fim_fn_t fname)const
	{
		if(marked_files_.find(fname)==marked_files_.end())
			return false;
		else
			return true;
	}

	void CommandConsole::markCurrentFile(bool mark)
	{
		markFile(browser_.current(), mark);
	}

	fim_int CommandConsole::markFile(const fim_fn_t& file, bool mark, bool aloud)
	{
		/*
		 * the current file will be added to the list of filenames
		 * which will be printed upon the program termination.
		 * */
		fim_int ret = 0; /* number of impacted files */

		if(file!=FIM_STDIN_IMAGE_NAME)
		{
			marked_files_t::iterator mfi=marked_files_.find(file);
			if(mfi==marked_files_.end())
			{
				if(mark)
				{
					++ret;
					marked_files_.insert(file);
					if(aloud)
						cout<<"Marked file \""<<file<<"\"\n";
				}
				else
					if(aloud)
						cout<<"File \""<<file<<"\" was not marked\n";
			}
			else
			{
				if( ! mark)
				{
					++ret;
					marked_files_.erase(mfi);
					if(aloud)
						cout<<"Unmarked file \""<<file<<"\"\n";
				}
				else
					if(aloud)
						cout<<"File \""<<file<<"\" was already marked\n";
			}
		}
		return ret;
	}
#endif /* FIM_WANT_FILENAME_MARK_AND_DUMP */

	void CommandConsole::printHelpMessage(const fim_char_t *pn)const
	{
		std::cout<<" Usage: "<<pn<<" [OPTIONS] [FILES]\n";
	}

#ifdef FIM_RECORDING
	fim::string CommandConsole::memorize_last(const fim_cmd_id& cmd)
	{
		if(dont_record_last_action_==false)
			last_action_=cmd;
		dont_record_last_action_=false;	//from now on we can memorize again
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::sanitize_action(const fim_cmd_id& cmd)const
	{
		/*
		 * make cmd dumpable
		 */
		if(cmd.c_str()[strlen(cmd.c_str())-1]!=FIM_SYM_SEMICOLON)
			return cmd+fim::string(FIM_SYM_SEMICOLON_STRING);
		return cmd;
	}
#endif /* FIM_RECORDING */
	void CommandConsole::appendPostInitCommand(const fim::string& cmd)
	{
		/* Passed via -c
		 * Executed right before a normal execution of Fim
		 * but after the configuration file loading.
		 * */
		postInitCommand_+=sanitize_action(cmd);
	}

	void CommandConsole::appendPreConfigCommand(const fim::string& cmd)
	{
		/* Passed via -C */
		preConfigCommand_+=sanitize_action(cmd);
	}

	void CommandConsole::appendPostExecutionCommand(const fim::string& cmd)
	{
		/* Passed via -W or -F */
		postExecutionCommand_+=sanitize_action(cmd);
	}
	
	bool CommandConsole::appendedPostInitCommand(void)const
	{
		return !postInitCommand_.empty();
	}

	bool CommandConsole::appendedPreConfigCommand(void)const
	{
		return !preConfigCommand_.empty();
	}

	Viewport* CommandConsole::current_viewport(void)const
	{
#ifdef FIM_WINDOWS
		return current_window().current_viewportp();
#else /* FIM_WINDOWS */
		return viewport_;
#endif /* FIM_WINDOWS */
	}

#ifdef FIM_WINDOWS
	const FimWindow& CommandConsole::current_window(void)const
	{
		return *window_;
	}
#endif /* FIM_WINDOWS */

	bool CommandConsole::push(const fim_fn_t nf, fim_flags_t pf)
	{
		return browser_.push_path(nf,pf);
	}

	bool CommandConsole::push(const char * nf, fim_flags_t pf)
	{
#if FIM_WANT_BACKGROUND_LOAD
		if( pf & FIM_FLAG_PUSH_ONE )
			fnpv_.push_back(nf);
#endif /* FIM_WANT_BACKGROUND_LOAD */
		return browser_.push_path(nf,pf);
	}

#if FIM_WANT_BACKGROUND_LOAD
	bool CommandConsole::background_push(void)
	{
		/* Note: There is no true concurrency control (e.g. lock in accessing browser_.flist_) at the moment. */
		blt_ = std::thread
	( [this](void)
	{
		setVariable(FIM_VID_LOADING_IN_BACKGROUND,1);
		for( auto fnpi : this->fnpv_ )
			this->browser_.push_path(fnpi,FIM_FLAG_PUSH_REC|FIM_FLAG_PUSH_BACKGROUND,&this->show_must_go_on_);
		this->fnpv_.erase(this->fnpv_.begin(),this->fnpv_.end());
		this->fnpv_.shrink_to_fit(); /* no use for this now */
		setVariable(FIM_VID_LOADING_IN_BACKGROUND,0);
  	}
	);
		return true;
	}
#endif /* FIM_WANT_BACKGROUND_LOAD */

#ifndef FIM_WANT_NOSCRIPTING
	bool CommandConsole::push_scriptfile(const fim_fn_t ns)
	{
		/*
		 * pushes a script up in the pre-execution scriptfile list
		 * */
	    	scripts_.push_back(ns);
		return true; /* for now a fare return code */
	}

	bool CommandConsole::with_scriptfile(void)const
	{
		return scripts_.size() !=0 ;
	}
#endif /* FIM_WANT_NOSCRIPTING */

	/*
	 *	Setting the terminal in raw mode means:
	 *	 - setting the line discipline
	 *	 - setting the read rate
	 *	 - disabling the echo
	 */
	void CommandConsole::tty_raw(void)
	{
		struct termios tattr;
		//we set the terminal in raw mode.
		    
		fcntl(0,F_GETFL,saved_fl_);
		tcgetattr (0, &saved_attributes_);
		    
		//fcntl(0,F_SETFL,O_BLOCK);
		memcpy(&tattr,&saved_attributes_,sizeof(struct termios));
		tattr.c_lflag &= ~(ICANON|ECHO);
		tattr.c_cc[VMIN] = 1;
		tattr.c_cc[VTIME] = 0;
		tcsetattr (0, TCSAFLUSH, &tattr);
	}
	
	void CommandConsole::tty_restore(void)
	{	
		//POSIX.1 compliant:
		//"a SIGIO signal is sent whenever input or output becomes possible on that file descriptor"
		fcntl(0,F_SETFL,saved_fl_);
		//the Terminal Console State Attributes will be set right NOW
		tcsetattr (0, TCSANOW, &saved_attributes_);
	}

	fim_err_t CommandConsole::load_or_save_history(bool load_or_save)
	{
#if FIM_WANT_HISTORY
#ifndef FIM_NOHISTORY
  #ifndef FIM_WANT_NOSCRIPTING
    #ifdef FIM_USE_READLINE
		bool do_load = (  load_or_save  && getIntVariable(FIM_VID_LOAD_FIM_HISTORY)==1 );
		bool do_save = ((!load_or_save) && getIntVariable(FIM_VID_SAVE_FIM_HISTORY)==1 );

		if( do_load || do_save )
		{
			fim_char_t hfile[FIM_PATH_MAX];
			const fim_char_t *const e = fim_getenv(FIM_CNS_HOME_VAR);

			if(e && strlen(e)<FIM_PATH_MAX-14)//strlen(FIM_CNS_HIST_FILENAME)+2
			{
				strcpy(hfile,e);
				strcat(hfile,"/" FIM_CNS_HIST_FILENAME);

				if( do_load )
					read_history(hfile);
#if HAVE_SYS_STAT_H
				else
				{
					bool need_chmod=!is_file(hfile);		// will try to chmod if already non existent
					write_history(hfile);
					if(need_chmod)
						chmod(hfile,S_IRUSR|S_IWUSR);	// we write the first .fim_history in mode -rw------- (600)
				}
#endif /* HAVE_SYS_STAT_H */
			}
		}

		return FIM_ERR_NO_ERROR;
    #endif /* FIM_USE_READLINE */
  #endif /* FIM_WANT_NOSCRIPTING  */
#endif /* FIM_NOHISTORY */
#endif /* FIM_WANT_HISTORY */
		return FIM_ERR_GENERIC;
	}

	void CommandConsole::cleanup(void)
	{
		/*
		 * the display device should exit cleanly to avoid cluttering the console
		 * ... or the window system
		 * used by: fb_catch_exit_signals(): should this matter ?
		 * */

		if(mangle_tcattr_)
			tty_restore();	
		if(displaydevice_)
		       	displaydevice_->finalize();
		load_or_save_history(false);
	}

	/*
	 * insert desc text into the textual console; eventually display it
	 */
	void CommandConsole::status_screen(const fim_char_t *desc)
	{
		if(!displaydevice_)
			return;

		displaydevice_->fb_status_screen_new((fim_char_t*)desc,drawOutput(desc),0);
	}

	void CommandConsole::set_status_bar(fim::string desc, const fim_char_t *info)
	{
		set_status_bar(desc.c_str(), info);
	}
	
	bool CommandConsole::set_wm_caption(const fim_char_t *str)
	{
		bool wcs = true;
#if FIM_WANT_CAPTION_CONTROL
		fim_err_t rc=FIM_ERR_NO_ERROR;
		string wcss = getStringVariable(FIM_VID_WANT_CAPTION_STATUS);

		if( wcss.c_str() && *wcss.c_str() && browser_.c_getImage())
		{
			fim::string clb = this->getInfoCustom(wcss.c_str());

			rc = displaydevice_->set_wm_caption(clb.c_str());
			wcs = false; /* window caption + status */
		}
		else
			if( str && *str )
				rc = displaydevice_->set_wm_caption(str);

		if(rc==FIM_ERR_UNSUPPORTED)
			wcs = false; /* revert */
#endif /* FIM_WANT_CAPTION_CONTROL */
		return wcs;
	}

	fim_err_t CommandConsole::update_font_size(void)
	{
#if ( FIM_FONT_MAGNIFY_FACTOR <= 0 )
		{
			fim_int fim_fmf__ = getIntVariable(FIM_VID_FBFMF);

			if( displaydevice_ && displaydevice_->f_ )
			{
				fim_int fim_afs__ = getIntVariable(FIM_VID_FBAFS);

				if( fim_afs__ >= 0 )
				{
					if( fim_afs__ == 0 )
						fim_afs__ = 50;
					fim_fmf__ = displaydevice_->suggested_font_magnification(fim_afs__,fim_afs__);
				}
			}

			if( fim_fmf__ < FIM_FONT_MAGNIFY_FACTOR_MIN )
				fim_fmf__ = FIM_FONT_MAGNIFY_FACTOR_DEFAULT;
			setVariable(FIM_VID_FBFMF,fim_fmf__);
			fim_fmf_ = FIM_MIN(fim_fmf__, FIM_FONT_MAGNIFY_FACTOR_MAX);
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
			/* FIXME: instead, call something like SDLDevice::post_wmresize(void) */
			mc_.setGlobalVariable(FIM_VID_CONSOLE_ROWS,(displaydevice_->get_chars_per_column()/2));
			mc_.reformat( displaydevice_->get_chars_per_line() );
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
			if( displaydevice_ )
				displaydevice_->format_console();
		}
#else /* FIM_FONT_MAGNIFY_FACTOR */
		setVariable(FIM_VID_FBFMF,FIM_FONT_MAGNIFY_FACTOR);
#endif /* FIM_FONT_MAGNIFY_FACTOR */
		return FIM_ERR_NO_ERROR;
	}

	void CommandConsole::set_status_bar(const fim_char_t *desc, const fim_char_t *info)
	{
		/*
		 *	Set the 'status bar' of the program.
		 *	- desc will be placed on the left corner
		 *	- info on the right
		 *	Pointers are meant to be freed by the caller.
		 */
		int chars, ilen;
		fim_char_t *str = FIM_NULL;
		fim::string hk=FIM_CNS_EMPTY_STRING;	/* help key string */
		int hkl=0;		/* help key string length */
		FIM_CONSTEXPR int mhkl=5,eisl=9;
		const fim_char_t *const hp=" - Help";
		int hpl=fim_strlen(hp);
		prompt_[1]=FIM_SYM_CHAR_NUL;
		fim_bool_t wcs = isSetVar(FIM_VID_WANT_CAPTION_STATUS);
	
		if( ! displaydevice_   )
		       	goto ret;

		hk=this->find_key_for_bound_cmd(FIM_FLT_HELP); // slow
		hkl=fim_strlen(hk.c_str());
		/* FIXME: can we guarantee a bound on its length in some way ? */
		if(hkl>mhkl)
			{hk=FIM_CNS_EMPTY_STRING;hkl=0;/* fix */}
		else
		{
			if(hkl>0)
				{hk+=hp;hkl=hpl;/* help message append*/}
			else
				{hpl=0;/* no help key ? no message, then */}
		}
	
		update_font_size();

		chars = displaydevice_->get_chars_per_line();
		if(chars<1)
			goto ret;

		str = fim_stralloc(chars+1);
		if(!str)
			goto ret;

		if (desc && info)
		{
			/* non interactive print */
			ilen = fim_strlen(info);
			if(chars-eisl-ilen-hkl>0)
			{
#if 0
				// sprintf(str, "%s%-*.*s [ %s ] %s",prompt_,
				sprintf(str, "%s%-*.*s %s %s",prompt_,
				chars-eisl-ilen-hkl, chars-eisl-ilen-hkl, desc, info, hk.c_str());//here above there is the need of 14+ilen chars
#else
				snprintf(str, chars-1, "%s%-*.*s %s %s",prompt_,
				chars-eisl-ilen-hkl, chars-eisl-ilen-hkl, desc, info, hk.c_str());//here above there is the need of 14+ilen chars
#endif
			}
			else
			{
				if(chars>5)
				{
					if(chars>10)
					{
				       		snprintf(str, chars-3, "%s%s", prompt_, info);
				       		strcat(str, "...");
					}
					else
				       		sprintf(str, "<-!->");
				}
				else
				{
				       	if(chars>0)
				 	      	sprintf(str, "!");
			       	}
			}
		}
#ifdef FIM_USE_READLINE
		else
		if(chars>=6+hkl && desc) /* would be a nonsense */
		{
			/* interactive print */
			static int statusline_cursor=0;
			int offset=0,coffset=0;

			statusline_cursor=rl_point;	/* rl_point is readline stuff */
			ilen = fim_strlen(desc);
			chars-=6+hpl+(*prompt_==FIM_SYM_CHAR_NUL?0:1);	/* displayable, non-service chars  */
			if(!chars)
				goto done;
			/* 11 is strlen(" | H - Help")*/
			offset =(statusline_cursor/(chars))*(chars);
			coffset=(*prompt_!=FIM_SYM_CHAR_NUL)+(statusline_cursor%(chars));
		
			sprintf(str, "%s%-*.*s | %s",prompt_, chars, chars, desc+offset, hk.c_str());
			str[coffset]='_';
		}
#endif /* FIM_USE_READLINE */

#if FIM_WANT_CAPTION_CONTROL
		if(wcs)
			wcs = set_wm_caption(str);
		if(!wcs)
#endif /* FIM_WANT_CAPTION_CONTROL */
			displaydevice_->status_line(str); /* one may check the return value.... */
done:
		fim_free(str);
ret:
		return;
	}

	fim_bool_t CommandConsole::inConsole(void)const
	{
#ifdef FIM_USE_READLINE
		return ic_==1;
#else /* FIM_USE_READLINE */
		return false;
#endif /* FIM_USE_READLINE */
	}

	fim_err_t CommandConsole::resize(fim_coo_t w, fim_coo_t h, fim_bool_t wsl)
	{
		if(!displaydevice_)
			return FIM_ERR_GENERIC;
		if( wsl )
			h += displaydevice_->status_line_height();
		if(FIM_ERR_NO_ERROR!=displaydevice_->resize(w,h))
			return FIM_ERR_GENERIC;

		w=displaydevice_->width();
		h=displaydevice_->height();

#ifdef FIM_WINDOWS
		if(window_)
			window_->update(Rect(0,0,w,h));
#endif
		displaydevice_->format_console();

		if(current_viewport())
			current_viewport()->update_meta(true);

		this->fcmd_redisplay(args_t());

		if(getGlobalIntVariable(FIM_VID_DISPLAY_BUSY))
		{
			std::ostringstream oss;
			oss << "resized window to " << w << " x " << h;
			cc.set_status_bar(oss.str().c_str(),FIM_NULL);
		}

		return FIM_ERR_NO_ERROR;
	}

	fim_err_t CommandConsole::display_reinit(const fim_char_t *rs)
	{
		if(!displaydevice_)
			goto err;
		return displaydevice_->reinit(rs);
err:
		return FIM_ERR_GENERIC;
	}

	fim_bool_t CommandConsole::key_syms_update(void)
	{
		sym_keys_t::const_iterator ki;

		for( ki=sym_keys_.begin();ki!=sym_keys_.end();++ki)
			key_syms_[(((*ki).second))]=((*ki).first);
		return true;
	}

	size_t CommandConsole::byte_size(void)const
	{
		size_t bs = 0;
		bs += browser_.byte_size();
		/* NOTE: lots is missing here */
#if FIM_WANT_PIC_CMTS
		bs += id_.byte_size();
#endif /* FIM_WANT_PIC_CMTS */
		return bs;
	}

#if FIM_EXPERIMENTAL_FONT_CMD
	fim_cxr CommandConsole::fcmd_font(const args_t& args)
	{
		// Experimental. Note that as of now, this leaks memory of the first loaded font.
		typedef std::pair<std::string,struct fs_font *> fim_ffp_t; /* file font pair */
		fim_ffp_t fr;
		static std::vector<fim_ffp_t> fc;
		static int fidx = 0;
		size_t fcnt = fc.size();
		const fim_int vl = 0; // font loading verbosity level (font loading will exit() if not 0)
		std::ostringstream rs;

		if(args.size()>=0)
		{
			fim::string cid;

			if(args.size()==0)
		       		cid = "info";
			else
		       		cid = args[0];

			if( cid == "next" || cid == "prev" )
			       	if( fcnt == 0 )
					cid = "scan"; // scan first time

			if( cid == "scan" )
			{
				fim::string nf;
				std::vector<fim_ffp_t > lfc;
				DIR *dir = FIM_NULL;
				struct dirent *de = FIM_NULL;
				int nfiles=0;

				if(args.size()>1)
			        	nf = args[1];
				else
			        	nf = FIM_LINUX_CONSOLEFONTS_DIR;
				if(nf.size() && nf[nf.size()-1]!=FIM_CNS_DIRSEP_CHAR)
					nf += FIM_CNS_DIRSEP_STRING;

				if( !is_dir( nf ))
					goto nop;
				if ( ! ( dir = opendir(nf.c_str() ) ))
					goto ret;

				set_status_bar(" scanning for fonts...",FIM_NULL);

				while( ( de = readdir(dir) ) != FIM_NULL )
				{
					if( de->d_name[0] == '.' &&  de->d_name[1] == '.' && !de->d_name[2] )
						continue;
					if( de->d_name[0] == '.' && !de->d_name[1] )
						continue;
					nfiles++;
	       				fr.first = nf;
	       				fr.first += de->d_name;
					fr.second = FIM_NULL;
					FontServer::fb_text_init1(fr.first.c_str(),&fr.second,vl);	// FIXME: move this outta here
					if(fr.second)
						lfc.push_back(fr);
				}
				closedir(dir);
#if FIM_USE_CXX11
				for(auto & fs : fc)
					fim_free_fs_font(fs.second), fs.second=FIM_NULL;
#else /* FIM_USE_CXX11 */
				for(	std::vector<fim_ffp_t>::iterator fi=fc.begin(); fi!=fc.end();++fi )
					fim_free_fs_font(fi->second), fi->second=FIM_NULL;
#endif /* FIM_USE_CXX11 */
				fc.erase(fc.begin(),fc.end());
				fc=lfc;

				rs << "font: " << "preloaded " << fc.size() << " fonts out of " << nfiles << " files scanned.";
				set_status_bar(rs.str().c_str(),FIM_NULL);
				rs << "\n";
			}

			if( cid == "load" && args.size()>1 )
			{
				fr.first=args[1];
				fr.second=FIM_NULL;
				FontServer::fb_text_init1(fr.first.c_str(),&fr.second,vl);	// FIXME: move this outta here
				if(fr.second)
					goto lofo;
				else
					rs << "failed loading font file " << fr.first;
			}

			if( cid == "info" )
				rs << getStringVariable(FIM_VID_FBFONT) << "\n";

			if( cid == "next" || cid == "prev" )
			{
				if( fcnt>0 )
				{
					if( cid == "next" )
						fidx=(fidx+1)%fcnt;
					if( cid == "prev" )
						fidx=(fidx+fcnt-1)%fcnt;
					rs << "font: loaded " << fidx << "/" << fcnt << ": ";
					fr=fc[fidx];
					goto lofo;
				}
				else
					rs << "first 'scan' a directory for fonts.\n";
			}
			goto nop;
lofo:
			if( displaydevice_ && displaydevice_->f_ )
			if(fr.second)
			{
				rs << fr.first.c_str() << "\n";
				setVariable(FIM_VID_FBFONT,fr.first);
				displaydevice_->f_ = fr.second;
				update_font_size();
			}
		}
nop:
ret:
		return rs.str();
	}
#endif /* FIM_EXPERIMENTAL_FONT_CMD */

	fim_cxr CommandConsole::fcmd_variables_list(const args_t& args)
	{
		return get_variables_list();
	}

	fim_cxr CommandConsole::fcmd_commands_list(const args_t& args)
	{
		return get_commands_list();
	}

	fim::string CommandConsole::current(void)const
	{
	       	return browser_.current();
	}

	fim_sys_int CommandConsole::get_displaydevice_input(fim_key_t * c, bool want_poll)
	{
		return displaydevice_->get_input(c, want_poll);
	}

#if FIM_WANT_PIC_CMTS
	bool CommandConsole::push_from_id(void)
	{
		for(	ImgDscs::const_iterator ifn=id_.begin();
				ifn!=id_.end();++ifn )
			this->push(ifn->first);
		return true;
	}
#endif /* FIM_WANT_PIC_CMTS */

	fim_cxr CommandConsole::fcmd_display(const args_t& args)
	{
		if( browser_.c_getImage() )
		{
			FIM_AUTOCMD_EXEC_PRE(FIM_ACM_PREDISPLAY,current());
			if( args.size()>0 && args[0] == "reinit" )
			{
				string arg = args.size()>1?args[1]:"";
				this->display_reinit(arg.c_str());
			}
			if( args.size()>0 && args[0] == "resize" )
			{
				fim_coo_t nww = browser_.c_getImage()->width();
				fim_coo_t nwh = browser_.c_getImage()->height();
				fim_bool_t wsl = (getGlobalIntVariable(FIM_VID_DISPLAY_BUSY)) ?  true : false;;

				if( args.size()>2 )
					nww = args[1],
					nwh = args[2],
					wsl = false;
				this->resize(nww,nwh,wsl);
			}
			if(browser_.c_getImage() && (getGlobalIntVariable(FIM_VID_OVERRIDE_DISPLAY)!=1))
			{
				if( this->redisplay() )
					browser_.display_status(current().c_str());
			}
			FIM_AUTOCMD_EXEC_POST(FIM_ACM_POSTDISPLAY);
		}
		else
		{
		       	cout << "no image to display, sorry!";
			this->set_status_bar("no image loaded.", "*");
		}
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd_redisplay(const args_t& args)
	{
		if(browser_.c_getImage())
		{
			FIM_AUTOCMD_EXEC_PRE(FIM_ACM_PREREDISPLAY,current());
			if(browser_.c_getImage())
			{
				current_viewport()->recenter();
				if( redisplay() )
					browser_.display_status(current().c_str());
			}
			FIM_AUTOCMD_EXEC_POST(FIM_ACM_POSTREDISPLAY);
		}
		return FIM_CNS_EMPTY_RESULT;
	}

	void CommandConsole::switch_if_needed(void)
	{
#if FIM_WANT_CONSOLE_SWITCH_WHILE_LOADING
		displaydevice_->switch_if_needed();
#endif /* FIM_WANT_CONSOLE_SWITCH_WHILE_LOADING */
	}

fim::string CommandConsole::getInfoCustom(const fim_char_t * ifsp)const
{
	// see FIM_VID_INFO_FMT_STR FIM_VID_COMMENT_OI 	FIM_VID_COMMENT_OI_FMT
	fim::string linebuffer;
	fim_char_t pagesinfobuffer[FIM_STATUSLINE_BUF_SIZE];
	fim_char_t imagemode[4];
#if FIM_WANT_CUSTOM_INFO_STATUS_BAR
	fim::string ifs;
#endif /* FIM_WANT_CUSTOM_INFO_STATUS_BAR */
	const Image * image = browser_.c_getImage();

	image->get_irs(imagemode);

	if(image->n_pages()>1)
		snprintf(pagesinfobuffer,sizeof(pagesinfobuffer)," [%d/%d]",image->get_page(),image->n_pages());
	else
		*pagesinfobuffer='\0';
		
#if FIM_WANT_CUSTOM_INFO_STATUS_BAR
	if( ifsp )
	{
		const char*fp=ifsp;
		const char*sp=ifsp;

		while(*sp && *sp!='%')
			++sp;
		linebuffer.append(ifsp,sp-ifsp);
		while(*sp=='%' && isprint(sp[1]))
		{
#if FIM_UNLIMITED_INFO_STRINGS
			string app;
#endif /* FIM_UNLIMITED_INFO_STRINGS */
			fim_char_t clb[FIM_IMG_DESC_ITEM_MAX_LEN];
			fim_char_t *clbp = clb;
			int rbc = sizeof(clb)/sizeof(clb[0]);
			clb[0]=FIM_SYM_CHAR_NUL;

			++sp;
			switch(*sp)
			{
				// "%p %wx%h %i/%l %F %M"
				case('p'):
					snprintf(clbp, rbc, "%.0f",image->get_scale()*100);
				break;
				case('w'):
					snprintf(clbp, rbc, "%d",(int)image->width());
				break;
				case('h'):
					snprintf(clbp, rbc, "%d",(int)image->height());
				break;
				case('i'):
					snprintf(clbp, rbc, "%d",(int)(browser_.current_n()+1));
				break;
				case('k'):
				{
					string cv = image->getStringVariable(FIM_VID_COMMENT);
					if( cv.c_str() && *cv.c_str() )
#if FIM_UNLIMITED_INFO_STRINGS
						app+='[', app+=cv, app+=']';
#else /* FIM_UNLIMITED_INFO_STRINGS */
						snprintf(clbp, rbc, "[%s] ",cv.c_str()); /* TODO: might generalize/change this */
#endif /* FIM_UNLIMITED_INFO_STRINGS */
				}
				break;
				case('l'):
					snprintf(clbp, rbc, "%d",(int)browser_.n_files());
				break;
				case('L'):
					snprintf(clbp, rbc, "%s",imagemode);
				break;
				case('P'):
					snprintf(clbp, rbc, "%s",pagesinfobuffer);
				break;
				case('F'):
					fim_snprintf_XB(clbp, rbc,image->get_file_size());
				break;
				case('M'):
					fim_snprintf_XB(clbp, rbc,(int)image->get_pixelmap_byte_size());
				break;
				case('n'):
#if FIM_UNLIMITED_INFO_STRINGS
					app += image->getStringVariable(FIM_VID_FILENAME).c_str();
#else /* FIM_UNLIMITED_INFO_STRINGS */
					snprintf(clbp, rbc, "%s",image->getStringVariable(FIM_VID_FILENAME).c_str());
#endif /* FIM_UNLIMITED_INFO_STRINGS */
				break;
				case('N'):
#if FIM_UNLIMITED_INFO_STRINGS
					app += fim_basename_of(image->getStringVariable(FIM_VID_FILENAME));
#else /* FIM_UNLIMITED_INFO_STRINGS */
					snprintf(clbp, rbc, "%s",fim_basename_of(image->getStringVariable(FIM_VID_FILENAME)));
#endif /* FIM_UNLIMITED_INFO_STRINGS */
				break;
				case('T'):
					fim_snprintf_XB(clbp, rbc,cc.byte_size());
				break;
				case('R'):
					fim_snprintf_XB(clbp, rbc, fim_maxrss());
				break;
#if FIM_WANT_MIPMAPS
				case('m'):
					fim_snprintf_XB(clbp, rbc,image->mm_byte_size());
				break;
#endif /* FIM_WANT_MIPMAPS */
				case('C'):
				{
					fim_char_t buf[2*FIM_PRINTFNUM_BUFSIZE];
					fim_snprintf_XB(buf, sizeof(buf),browser_.cache_.img_byte_size());
					snprintf(clbp, rbc, "#%d:%s",(int)browser_.cache_.cached_elements(),buf);
				}
				break;
				case('c'):
					current_viewport()->snprintf_centering_info(clbp, rbc);
				break;
				case('v'):
					snprintf(clbp, rbc, "%s",FIM_CNS_FIM_APPTITLE);
				break;
				case('%'):
					snprintf(clbp, rbc, "%c",'%');
				break;
#if FIM_EXPERIMENTAL_VAR_EXPANDOS 
				case('?'): /* "%?forward_comment?_filename?back_comment?" */
#if 1
				if(strlen(sp+1)>=4)
				{
					char *fcp = FIM_NULL, *vip = FIM_NULL;
					if( 2 == sscanf(sp,"?%m[A-Z_a-z]?%m[^?]?",&vip,&fcp) )
					if(fcp && vip)
					{
						char *fcpp = fcp;

						if(*vip && image->isSetVar(vip) && *fcp )
						{
							char *vipp = FIM_NULL;
strdo:
							vipp = fcpp;
							while(*fcpp && *fcpp != '%')
								++fcpp;
#if FIM_UNLIMITED_INFO_STRINGS
							app += std::string(vipp,fcpp);
#else /* FIM_UNLIMITED_INFO_STRINGS */
							snprintf(clbp, FIM_MIN(rbc,fcpp-vipp+1), "%s", vipp );
							rbc -= strlen(clbp);
							clbp += strlen(clbp);
#endif /* FIM_UNLIMITED_INFO_STRINGS */

							if(!*fcpp)
								goto strdone;
							++fcpp;
							vipp = fcpp;
							if(*fcpp==':')
							{
								++fcpp;
								while(*fcpp && *fcpp!=':' && ( isalpha(*fcpp) || isdigit(*fcpp) || *fcpp=='_' ))
									++fcpp;
								string vn = string(vipp).substr(1,fcpp-vipp-1);
								string vv = image->getStringVariable(vn);
								if(*fcpp==':')
#if FIM_UNLIMITED_INFO_STRINGS
									app += vv.c_str(),
#else /* FIM_UNLIMITED_INFO_STRINGS */
									snprintf(clbp, rbc, "%s",vv.c_str()),
#endif /* FIM_UNLIMITED_INFO_STRINGS */
									++fcpp;
								else
#if FIM_UNLIMITED_INFO_STRINGS
									app += "<?>";
#else /* FIM_UNLIMITED_INFO_STRINGS */
									snprintf(clbp, rbc, "%s","<?>");
#endif /* FIM_UNLIMITED_INFO_STRINGS */
							}
							else
#if FIM_UNLIMITED_INFO_STRINGS
								app += "<?>";
#else /* FIM_UNLIMITED_INFO_STRINGS */
								snprintf(clbp, rbc, "%s","<?>");
							rbc -= strlen(clbp);
							clbp += strlen(clbp);
#endif /* FIM_UNLIMITED_INFO_STRINGS */
							goto strdo;
						}
strdone:
						sp += strlen(fcp)+strlen(vip)+2;
					}
					if(fcp)std::free(fcp);
					if(vip)std::free(vip);
				}
#else
				if(strlen(sp+1)>=3)
				{
					char *fcp = FIM_NULL, *vip = FIM_NULL, *bcp = FIM_NULL;
					if( 3 == sscanf(sp,"?%a[^?%]?%a[A-Z_a-z]?%a[^?%]?",&fcp,&vip,&bcp) )
					if(fcp && bcp && vip)
					{
						if(*vip && image->isSetVar(vip))
							snprintf(clbp, rbc, "%s%s%s",fcp,image->getStringVariable(vip).c_str(),bcp);
						sp += strlen(fcp)+strlen(vip)+strlen(bcp)+3;
					}
					if(fcp)std::free(fcp);
					if(bcp)std::free(bcp);
					if(vip)std::free(vip);
				}
#endif
				break;
#endif /* FIM_EXPERIMENTAL_VAR_EXPANDOS */
				// default:
				/* rejecting char; may display an error message here */
			}
			++sp;
			fp=sp;
			while(*sp!='%' && sp[0])
				++sp;
			rbc -= strlen(clbp); clbp += strlen(clbp);
#if FIM_UNLIMITED_INFO_STRINGS
			if(!app.empty())
			{
				if(sp-fp>1)
					app = std::string(fp,sp) + app;
				linebuffer+=app;
			}
			else
#endif /* FIM_UNLIMITED_INFO_STRINGS */
			{
				snprintf(clbp, FIM_MIN(sp-fp+1,rbc), "%s",fp);
				rbc -= strlen(clbp); clbp += strlen(clbp);
				linebuffer+=clb;
			}
		}
		//std::cout << "Custom format string chosen: "<< ifsp << ", resulting in: "<< clb <<"\n";
		goto labeldone;
	}
#endif /* FIM_WANT_CUSTOM_INFO_STATUS_BAR */
labeldone:
	return linebuffer;
}
} /* namespace fim */
