/* $LastChangedDate: 2017-07-23 23:23:40 +0200 (Sun, 23 Jul 2017) $ */
/*
 CommandConsole-cmd.cpp : Fim console commands

 (c) 2009-2017 Michele Martone

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
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif /* HAVE_LIBGEN_H */

#define FIM_WANT_SYSTEM_CALL_DEBUG 0
#define FIM_CMD_RET_HELP_MSG(CMD) "See " FIM_FLT_HELP " " CMD " for its invocation.\n"

namespace fim
{
	fim_key_t CommandConsole::kstr_to_key(const fim_char_t * kstr)const
	{
		int l;
		fim_key_t key = FIM_SYM_NULL_KEY;

		if(!kstr)
			goto err;
		l=strlen(kstr);
		if(!l)
			goto err;

#ifdef FIM_WANT_RAW_KEYS_BINDING
		if(l>=2 && isdigit(kstr[0]) && isdigit(kstr[1]))
		{
			/* special syntax for raw keys */
			key=atoi(kstr+1);
		}
#endif /* FIM_WANT_RAW_KEYS_BINDING */
		else
		{
			if(sym_keys_.find(kstr)!= sym_keys_.end())
				key=sym_keys_.find(fim::string(kstr))->second;
		}
err:
		return key;
	}

	fim_cxr CommandConsole::fcmd_bind(const args_t& args)
	{
		/*
		 *	this is the interactive bind command
		 *	the user supplies a string with the key combination, and if valid, its keycode
		 *	is associated to the user supplied action (be it a command, alias, etc..)
		 */
		const fim_char_t* kerr=FIM_FLT_BIND ": invalid key argument (should be one of: k, C-k, K, <Left..> }\n";
		const fim_char_t* kstr=FIM_NULL;
		fim_key_t key=FIM_SYM_NULL_KEY;

		if(args.size()==0)
			return getBindingsList();

		kstr=(args[0].c_str());
		if(!kstr || 0 == strlen(kstr))
		{
#if FIM_WANT_BIND_ALL
			if(args.size()>1)
			{
				for (auto bi=bindings_.begin(); bi != bindings_.end(); ++bi)
					bindings_[bi->first]=args[1];
				return "Re-bound all keys to " + args[1];
			}
			else
#endif
			return kerr;
		}

		key = kstr_to_key(args[0].c_str());

		if(key==FIM_SYM_NULL_KEY)
		{
			return "supplied key "+args[0]+" is invalid.\n";
		}
		if(args.size()==1 && bindings_.find(key)==bindings_.end())
		{
			return "no command bound to keycode "+string(key)+" (keysym "+args[0]+").\n";
		}

		if(args.size()==1)
		{
			//first arg should be a valid key code
			std::ostringstream oss;
			oss << FIM_FLT_BIND" '" << args[0] << "' '" << bindings_[key] << "'\n";
			return oss.str();
		}
		if(args.size()<2)
		       	return kerr;
		if(args[1]==FIM_CNS_EMPTY_STRING)
		       	return unbind(args[0]);

		if(args.size()>=3)
			return bind(key,args[1],args[2]);
		else
			return bind(key,args[1]);
	}

	fim_cxr CommandConsole::fcmd_unbind(const args_t& args)
	{
		/*
		 * 	unbinds the action eventually bound to the first key name specified in args..
		 * 	what about multiple unbindings ?
		 *	maybe you should made surjective the binding_keys mapping..
		 */
		if(args.size()!=1)
			return FIM_FLT_UNBIND ": specify the key to unbind\n";
		return unbind(args[0]);
	}

#if 0
	fim_cxr CommandConsole::fcmd_setenv(const args_t& args)
	{
		/*
		 *	
		 */
		if(args.size()<2) return FIM_CNS_EMPTY_RESULT;
		setenv(args[0].c_str(),args[1].c_str(),1);
		setenv("DISPLAY","",1);
		return FIM_CNS_EMPTY_RESULT;
	}
#endif

	fim_cxr CommandConsole::get_help(const fim_cmd_id& item)const
	{
		if(item.size())
		{
			if((item.at(0))==FIM_CNS_SLASH_CHAR)
			{
				/* regexp-based would be nicer */
				fim::string hstr,cstr,astr,bstr,ptn;

				ptn = item.substr(1,item.size());
				for(size_t i=0;i<commands_.size();++i) 
					if(commands_[i].getHelp().find(ptn) != commands_[i].getHelp().npos)
					{
						cstr += commands_[i].cmd();
						cstr += " ";
					}

				for( aliases_t::const_iterator ai=aliases_.begin();ai!=aliases_.end();++ai)
				{
					if(ai->first.find(ptn) != ai->first.npos)
					{	
						astr +=((*ai).first);
						astr += " ";
					}
					else
					if(ai->second.first.find(ptn) != ai->second.first.npos)
					{	
						astr +=((*ai).first);
						astr += " ";
					}
					else
					if(ai->second.second.find(ptn) != ai->second.second.npos)
					{	
						astr +=((*ai).first);
						astr += " ";
					}
				}

				for(bindings_t::const_iterator  bi=bindings_.begin();bi!=bindings_.end();++bi)
				{
					key_syms_t::const_iterator ikbi=key_syms_.find(((*bi).first));
					bindings_help_t::const_iterator  bhi=bindings_help_.find((*bi).first);

					if(ikbi!=key_syms_.end())
					{
						if(ikbi->second.find(ptn) != ikbi->second.npos)
						{
							bstr += ikbi->second;
						       	bstr += " ";
							// std::cout << "key: " << ikbi->second << "\n";
						}
						else
						if(bi->second.find(ptn) != bi->second.npos)
						{
							bstr += ikbi->second;
						       	bstr += " ";
							// std::cout << "def: " << ikbi->second << "\n";
						}
						if(bhi != bindings_help_.end() )
						if(bhi->second.find(ptn) != bhi->second.npos)
						{
							bstr += ikbi->second;
						       	bstr += " ";
							// std::cout << "dsc: " << ikbi->second << "\n";
						}
					}
				}
				/* FIXME: may extend to autocmd's */

				if(!cstr.empty())
					hstr+="Commands (type 'help' and one quoted command name to get more info):\n " + cstr + "\n";
				if(!astr.empty())
					hstr+="Aliases (type 'alias' and one quoted alias to get more info):\n " + astr + "\n";
				if(!bstr.empty())
					hstr+="Bindings (type 'bind' and one quoted binding to get more info):\n " + bstr + "\n";
				if(!hstr.empty())
					return "The following help items matched \"" + ptn + "\":\n" + hstr;
				else
					return "No item matched \"" + ptn + "\"\n";
					// note that '/' will be caught here as well..
			}

		{
			std::ostringstream oss;
			fim::string sws;
			// consider using fim_shell_arg_escape or a modification of it here.
			sws = fim_help_opt(item.c_str());
			if( sws != FIM_CNS_EMPTY_STRING )
				oss << sws << "\n";
			if( findCommandIdx(item) != FIM_INVALID_IDX )
				oss << "\"" << item << "\" is a command, documented:\n" << commands_[findCommandIdx(item)].getHelp() << "\n";
			if(aliasRecall(fim::string(item))!=FIM_CNS_EMPTY_STRING)
				oss << "\"" << item << "\" is an alias, and was declared as:\n" << get_alias_info(item) << "\n";
			if( getBoundAction(kstr_to_key(item))!=FIM_CNS_EMPTY_STRING)
				oss << "\"" << item << "\" key is bound to command: " << getBoundAction(kstr_to_key(item))<<"\n";
			if(isVariable(item))
				oss << "\"" << item << "\" is a variable, with value:\n" 
					<< getStringVariable(item) << "\nand description:\n" 
					<< fim_var_help_db_query(item) << "\n";
			if( oss.str() != FIM_CNS_EMPTY_STRING )
				return oss.str();
			else
				return item + ": no such command, alias, bound key or variable.\n";
		}
		}
		return "";
	}

	fim_cxr CommandConsole::fcmd_help(const args_t& args)
	{	
		fim_cxr retval;

		if( !args.empty() && args[0].length()>0 )
			retval = get_help(args[0]);

		this->setVariable(FIM_VID_DISPLAY_CONSOLE,1);

		if(retval.empty())
			retval = "" FIM_FLT_HELP " " FIM_CNS_EX_ID_STRING ": provides help for " FIM_CNS_EX_ID_STRING ", if it is a variable, alias, or command. Use " FIM_KBD_TAB " in commandline mode to get a list of commands. Command line mode can be entered with the default key '" FIM_SYM_CONSOLE_KEY_STR "', and left pressing " FIM_KBD_ENTER ".\n";
		return retval;

	}

	fim_cxr CommandConsole::fcmd_quit(const args_t& args)
	{
		/*
		 * now the postcycle execution autocommands are enabled !
		 * */
		show_must_go_on_=0;
		if( args.size() < 1 )
			return_code_=0;
		else
			return_code_=(int)args[0];
		return FIM_CNS_EMPTY_RESULT;
	}

#ifndef FIM_WANT_NOSCRIPTING
	fim_cxr CommandConsole::fcmd_executeFile(const args_t& args)
	{
		for(size_t i=0;i<args.size();++i)
			if( executeFile(args[i].c_str()) != FIM_ERR_NO_ERROR )
				; // might be verbose ...
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_WANT_NOSCRIPTING */
	
	fim_cxr CommandConsole::fcmd_foo(const args_t& args)
	{
		/*
		 * useful function for bogus commands, but autocompletable (like language constructs)
		 * */
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd_echo(const args_t& args)
	{
		return do_echo(args);
	}

	fim::string CommandConsole::do_echo(const args_t& args)const
	{
		/*
		 * a command to echo arguments, for debug and learning purposes
		 */
		if(args.size()==0)
			fim::cout<<FIM_FLT_ECHO" command\n";
		for(size_t i=0;i<args.size();++i)
			fim::cout << (args[i].c_str()) << "\n";
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd__stdout(const args_t& args)
	{
		/*
		 * a command to echo to stdout arguments, for debug and learning purposes
		 */
		if(args.size()==0)
			std::cout<<"echo command\n";
		for(size_t i=0;i<args.size();++i)
			std::cout << (args[i].c_str()) << "\n";
		return FIM_CNS_EMPTY_RESULT;
	}

#ifdef FIM_AUTOCMDS
	fim_cxr CommandConsole::fcmd_autocmd(const args_t& args)
	{
		/*
		 * associates an action to a certain event in certain circumstances
		 */
		//cout << "autocmd '"<<args[0]<<"' '"<<args[1]<<"' '"<<args[2]<<"' added..\n";
		if(args.size()==0)
		{
			/* no args, returns autocmd's list */
			return autocmds_list("","");
		}
		if(args.size()==1)
		{
			/* autocmd Event: list all autocmds for Event */
			return autocmds_list(args[0],"");
		}
		if(args.size()==2)
		{
			/* autocmd Event Pattern: list all autocmds for Event Pattern */
			return autocmds_list(args[0],args[1]);
		}
		if(args.size()==3)
		{
			return autocmd_add(args[0],args[1],args[2]);
		}
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd_autocmd_del(const args_t& args)
	{
		/*
		 * deletes one or more autocommands
		 */
		if(args.size()==0)
		{
			/* no args, returns autocmd's list */
			return autocmd_del("","","");
		}
		if(args.size()==1)
		{
			/* autocmd Event: delete all autocmds for Event */
			return autocmd_del(args[0],"","");
		}
		if(args.size()==2)
		{
			/* autocmd Event Pattern: delete all autocmds for Event Pattern */
			return autocmd_del(args[0],args[1],"");
		}
		if(args.size()==3)
		{
			return autocmd_del(args[0],args[1],args[2]);
		}
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_AUTOCMDS */

	fim_cxr CommandConsole::fcmd_set_in_console(const args_t& args)
	{
		/*
		 * EXPERIMENTAL !!
		 * */
#ifdef FIM_USE_READLINE
		ic_ = 1;
#endif /* FIM_USE_READLINE */
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd_set_interactive_mode(const args_t& args)
	{
#ifdef FIM_USE_READLINE
		ic_=-1;set_status_bar("",FIM_NULL);
#endif /* FIM_USE_READLINE */
		/*
		 *
		 * */
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd_sys_popen(const args_t& args)
	{
		/*
		 *
		 * */
		for(size_t i=0;i<args.size();++i)
		{
			FILE* fd=popen(args[i].c_str(),"r");
			/*
			 * example:
			 *
			 * int fd=(int)popen("/bin/echo quit","r");
			 */
			executeStdFileDescriptor(fd);
			pclose(fd);
		}
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_err_t CommandConsole::fpush(FILE *tfd)
	{
			/* todo: read errno in case of error and print some report.. */
	
			/* pipes are not seekable: this is incompatible with Fim's file handling mechanism */
			/*
			while( (rc=read(0,buf,FIM_PIPE_BUFSIZE))>0 ) fwrite(buf,rc,1,tfd);
			rewind(tfd);
			*/
			/*
			 * Note that it would be much nicer to do this in another way,
			 * but it would require to rewrite much of the file loading stuff
			 * (which is quite fbi's untouched stuff right now)
			 * */
			ImagePtr stream_image=FIM_NULL;
			if(!tfd)
				return FIM_ERR_GENERIC;
			try{ stream_image=ImagePtr ( new Image(FIM_STDIN_IMAGE_NAME,fim_fread_tmpfile(tfd)) ); }
			catch (FimException e){/* write me */}
#ifdef FIM_READ_STDIN_IMAGE
			if(stream_image)
			{
#if FIM_USE_CXX11
				browser_.set_default_image(ImagePtr(std::move(stream_image)));
#else /* FIM_USE_CXX11 */
				browser_.set_default_image(stream_image);
#endif /* FIM_USE_CXX11 */
				if(!cc.browser_.cache_.setAndCacheStdinCachedImage(stream_image))
					std::cerr << FIM_EMSG_CACHING_STDIN;
				browser_.push_path(FIM_STDIN_IMAGE_NAME);
			}
#else /* FIM_READ_STDIN_IMAGE */
			/* FIXME: this point should be better not reached */
#endif /* FIM_READ_STDIN_IMAGE */
			return FIM_ERR_NO_ERROR;
			//pclose(tfd);
	}

#ifdef FIM_PIPE_IMAGE_READ
	/*
	 * FBI/FIM FILE PROBING MECHANISMS ARE NOT THOUGHT WITH PIPES IN MIND!
	 * THEREFORE WE MUST FIND A SMARTER TRICK TO IMPLEMENT THIS
	 * */
	fim_cxr CommandConsole::fcmd_pread(const args_t& args)
	{
		/*
		 * we read a whole image file from pipe
		 * */
		size_t i;
		FILE* tfd;
		/* fim_char_t buf[FIM_PIPE_BUFSIZE]; int rc=0; */
		for(i=0;i<args.size();++i)
		if( (tfd=popen(args[i].c_str(),"r")) != FIM_NULL )
		{	
			fpush(tfd);
		}
		else
		{
			/*
			 * error handling
			 * */
		}
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_PIPE_IMAGE_READ */

	fim_cxr CommandConsole::fcmd_cd(const args_t& args)
	{
		/*
		 * change working directory
		 * */
		for(size_t i=0;i<args.size();++i)
		{
			fim::string dir=args[i];
			if(dir=="-")
				dir=oldcwd_;
			oldcwd_=fim_getcwd();
#ifdef HAVE_LIBGEN_H
			if(!is_dir(dir))
				dir=fim_dirname(dir);
#endif /* HAVE_LIBGEN_H */
			if( chdir(dir.c_str()) )
			       	return (fim::string("cd error: ")+fim::string(strerror(errno)));
		}
		setVariable(FIM_VID_PWD,fim_getcwd());
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd_basename(const args_t& args)
	{
		fim::string res;
		for(size_t i=0;i<args.size();++i)
			res+=fim_basename_of(args[i]);
		return res;
	}

	fim_cxr CommandConsole::fcmd_pwd(const args_t& args)
	{
		/*
		 * print working directory
		 * */
		fim::string cwd = fim_getcwd();
		return cwd;
	}

#ifndef FIM_NO_SYSTEM
	fim_cxr CommandConsole::fcmd_system(const args_t& args)
	{
		/*
		 * executes the shell commands given in the arguments,
		 * one by one, and returns the (collated) standard output
		 * */
#if FIM_WANT_SYSTEM_CALL_DEBUG
		fim::string is=FIM_CNS_EMPTY_STRING;
#endif /* FIM_WANT_SYSTEM_CALL_DEBUG */
#if FIM_WANT_SINGLE_SYSTEM_INVOCATION
		/* 20110302 FIXME: inefficient */
		fim::string cc=FIM_CNS_EMPTY_STRING;
		for(size_t i=0;i<args.size();++i)
		{
			// FIXME: escaping the command (first argument) actually requires more than this
#define FIM_WANT_SIMPLE_SHELL_ESCAPING 1
#if FIM_WANT_SIMPLE_SHELL_ESCAPING
			cc+=fim_shell_arg_escape(args[i]);
#else /* FIM_WANT_SIMPLE_SHELL_ESCAPING */
			cc+=args[i];
#endif /* FIM_WANT_SIMPLE_SHELL_ESCAPING */
			cc+=" ";
#if FIM_WANT_SYSTEM_CALL_DEBUG
			is+=args[i];
			is+=" ";
#endif /* FIM_WANT_SYSTEM_CALL_DEBUG */
		}
#if FIM_WANT_SYSTEM_CALL_DEBUG
		std::cerr << "received string: " << is << FIM_SYM_ENDL;	
		std::cerr << "about to call popen on string: " << cc << FIM_SYM_ENDL;	
#endif /* FIM_WANT_SYSTEM_CALL_DEBUG */
		if(args.size())
		{
			FILE* fd=popen(cc.c_str(),"r");
		//	cout << readStdFileDescriptor(fd);
			setVariable(FIM_VID_LAST_SYSTEM_OUTPUT,readStdFileDescriptor(fd));
			cout << getStringVariable(FIM_VID_LAST_SYSTEM_OUTPUT);
		       	pclose(fd);
		}
#else /* FIM_WANT_SINGLE_SYSTEM_INVOCATION */
		for(size_t i=0;i<args.size();++i)
		{
			FILE* fd=popen(args[i].c_str(),"r");
			/*
			 * popen example:
			 *
			 * int fd=(int)popen("/bin/echo quit","r");
			 */
			//cout << readStdFileDescriptor(fd);
			setVariable(FIM_VID_LAST_SYSTEM_OUTPUT,readStdFileDescriptor(fd));
			pclose(fd);
		}
#endif /* FIM_WANT_SINGLE_SYSTEM_INVOCATION */
#if 0
		for(size_t i=0;i<args.size();++i)
		{
			std::system(args[i].c_str());
		}
#endif
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_NO_SYSTEM */
	
	fim_cxr CommandConsole::fcmd_status(const args_t& args)
	{
		/*
		 * the status bar is updated with the given arguments collated.
		 * */
		fim::string s;
		for(size_t i=0;i<args.size();++i)
			s+=args[i].c_str();
		browser_.display_status(s.c_str());
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd_unalias(const args_t& args)
	{
		/*
		 * removes the actions assigned to the specified aliases,
		 */
		if(args.size()<1)
			return FIM_FLT_UNALIAS ": please specify an alias to remove or all (-a)!\n";

		if(args[0]==string("-a"))
		{
			/* the lexer/parser has limitation and it takes -a as a numerical expression if not between double quotes ("-a") */
			aliases_.clear();
			return FIM_CNS_EMPTY_RESULT;
		}

		for(size_t i=0;i<args.size();++i)
		if(aliases_[args[i]].first!=FIM_CNS_EMPTY_STRING)
		{
			aliases_.erase(args[i]);
			return FIM_CNS_EMPTY_RESULT;
			/* fim::string(FIM_FLT_UNALIAS ": \"")+args[i]+fim::string("\" successfully unaliased.\n"); */
		}
		else
		       	return fim::string(FIM_FLT_UNALIAS ": \"")+args[i]+fim::string("\" there is not such alias.\n");
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd_dump_key_codes(const args_t& args)
	{
		return do_dump_key_codes(DefRefMode);
	}

	fim::string CommandConsole::do_dump_key_codes(FimDocRefMode refmode)const
	{
		/*
		 * dump all keysyms
		 * */
		std::ostringstream oss;
		sym_keys_t::const_iterator ki;

		if ( refmode == DefRefMode )
			oss<<"There are " << fim::string((int)(sym_keys_.size())) << " bindings (dumping them here, unescaped).\n";

		for( ki=sym_keys_.begin();ki!=sym_keys_.end();++ki)
		{
			if ( refmode == DefRefMode )
				oss << ((*ki).first);
			else
			{
				oss << "\"";
				if( ((*ki).first).compare(0,1,"\\") == 0 )
					oss << '\\';
				oss << ((*ki).first) << "\"";
			}
			if ( refmode == DefRefMode )
				oss << " -> " << fim::string((int)(((*ki).second)));
			oss << "  ";
		}
		oss << "\n";
		return oss.str();
	}

#ifdef FIM_RECORDING
	fim::string CommandConsole::dump_record_buffer(const args_t& args)
	{
		return do_dump_record_buffer(args,false);
	}

	fim::string CommandConsole::do_dump_record_buffer(const args_t& args, bool headeronly)const
	{
		std::ostringstream oss;

		if(headeronly)
			oss << recorded_actions_.size() << " commands.\n";
		else
		for(size_t i=0;i<recorded_actions_.size();++i)
			oss << FIM_FLT_USLEEP " '" << recorded_actions_[i].second << "';\n"
			       	<< recorded_actions_[i].first << "\n";
		return oss.str();
	}

	fim::string CommandConsole::execute_record_buffer(const args_t& args)
	{
		/*
		 * all of the commands in the record buffer are re-executed.
		 * */
		if(recordMode_==Normal)
		{
			cout << "Will execute " << do_dump_record_buffer(args_t(),true);
			recordMode_=Playing;
			execute_internal(dump_record_buffer(args).c_str(),FIM_X_NULL);
			recordMode_=Normal;
		}
		/* for unknown reasons, the following code gives problems: image resizes don't work..
		 * but the present (above) doesn't support interruptions ...
		 * */
/*		fim::string res;
		for(size_t i=0;i<recorded_actions_.size();++i)
		{
			res=recorded_actions_[i].first+(fim::string)recorded_actions_[i].second;
			execute_internal(res.c_str(),FIM_X_QUIET);
		}*/
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd_eval(const args_t& args)
	{
		/*
		 * all of the commands given as arguments are executed.
		 * */
		for(size_t i=0;i<args.size();++i)
		{
			if(execute_internal(args[i].c_str(),FIM_X_NULL))
				return fim::string("problems executing ")+args[i]+fim::string("\n");
		}
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_RECORDING */

	fim::string CommandConsole::repeat_last(const args_t& args)
	{
		/*
		 * WARNING: there are a few points concerning this command:
		 * - This command should NOT be registered as last_command, nor any alias 
		 *   triggering it. But this solution would require heavy parsing and very
		 *   complicated machinery and information propagation... 
		 * - A solution would be confining the repeat_last only to interactive commands,
		 *   but this would be a lot sorrowful too, and requires the non-registration 
		 *   of the 'repeat_last;' issuing..
		 * - So, since the recording is made AFTER the command was executed, we set
		 *   a dont_record_last_action_ flag after each detection of repeat_last, so we do not 
		 *   record the containing string.
		 */
		execute_internal(last_action_.c_str(),FIM_X_NULL);
		dont_record_last_action_=true;	//the issuing action will not be recorded
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd_recording(const args_t& args)
	{
		if(args.size()<1)
		{
			if(recordMode_==Normal)
				return "Neither recording nor re-executing. Specify 'start' to start recording commands and then 'stop' to terminate, and 'dump' to see recorded commands.\n";
			if(recordMode_==Recording)
				return "Recording commands. Specify 'stop' to stop recording and then 'play' to execute them.\n";
			if(recordMode_==Playing)
				return "Executing recorded commands.\n";
		}
		else
		{
			if(args[0]=="start")
				return start_recording();
			if(args[0]=="stop")
				return stop_recording();
			if(args[0]=="dump")
				return dump_record_buffer(args_t(args.begin()+1,args.end()));
			if(args[0]=="execute" || args[0]=="play")
				return execute_record_buffer(args_t(args.begin()+1,args.end()));
			if(args[0]=="repeat_last")
				return repeat_last(args_t(args.begin()+1,args.end()));
		}
nop:
		return FIM_CMD_RET_HELP_MSG(FIM_FLT_RECORDING);
	}

	fim::string CommandConsole::start_recording(void)
	{
		std::ostringstream oss;

		if(recordMode_==Normal)
		{
			recorded_actions_.clear();
			recordMode_=Recording;
			record_action(std::string()); // initialization
			oss << "Starting recording.\n";
		}
		return oss.str();
	}

	fim::string CommandConsole::stop_recording(void)
	{
		std::ostringstream oss;

		if(recordMode_==Recording)
		{
			oss << "Recorded " << do_dump_record_buffer(args_t(),true);
			recordMode_=Normal;
		}
		return oss.str();
	}

	fim_cxr CommandConsole::fcmd_set(const args_t& args)
	{
		/*
		 * with no arguments, prints out the variable names.
		 * with one identifier as argument, prints out its value.
		 * with two arguments, sets the first argument's value.
		 * */
		if( ! args.size())
			return get_variables_list();
		if(1==args.size())
			return getStringVariable(args[0]);
		/*
		 * warning!
		 * */
		if(2==args.size())
			return setVariable(args[0],args [1]);
		else
			return FIM_CMD_HELP_SET;
	}

#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	fim::string CommandConsole::scroll_up(const args_t& args)
	{
		if(!displaydevice_)
	       	{ }
		else
			displaydevice_->console_control(0x01);
		return FIM_CNS_EMPTY_RESULT;
	}

	fim::string CommandConsole::scroll_down(const args_t& args)
	{
		if(!displaydevice_)
	       	{ }
		else
			displaydevice_->console_control(0x02);
		return FIM_CNS_EMPTY_RESULT;
	}

	fim_cxr CommandConsole::fcmd_clear(const args_t& args)
	{
		displaydevice_->console_control(0x03);//experimental
		return FIM_CNS_EMPTY_RESULT;
	}
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */

	fim_cxr CommandConsole::fcmd_do_getenv(const args_t& args)
	{
		string help="usage: " FIM_FLT_GETENV " " FIM_CNS_EX_ID_STRING " will create a fim variable named " FIM_CNS_EX_ID_STRING " with value $" FIM_CNS_EX_ID_STRING " (if nonempty), from the current shell."
#ifndef HAVE_GETENV
		" (note that getenv call was not available at build time, so it won't work)\n"
#endif /* HAVE_GETENV */
		;
		if( ! args.size())
			return help;
#ifdef HAVE_GETENV
		if(1==args.size())
		{
			if(fim_getenv(args[0].c_str()))
				return setVariable( fim::string("ENV_")+args[0], fim_getenv(args[0].c_str()) );
			else
				return FIM_CNS_EMPTY_RESULT;
		}
		else
			return help;
#else /* HAVE_GETENV */
		return help;
#endif /* HAVE_GETENV */
	}

#if FIM_USE_CXX11
	/* */
#else /* FIM_USE_CXX11 */
	static void fim_cc_fetch(const std::pair<string,fim_char_t> & dfp){cc.id_.fetch(dfp.first,dfp.second);}
#endif /* FIM_USE_CXX11 */

	fim_cxr CommandConsole::fcmd_desc(const args_t& args)
	{
#if FIM_WANT_PIC_CMTS
		fim_char_t sc = '\t';
		int aoc = fim_args_opt_count(args,'-');

#if FIM_WANT_PIC_CMTS_RELOAD
		if ( args.size()-aoc > 0 && args[aoc] == "reload" )
		{
#if FIM_USE_CXX11
			for (auto & dfp : cc.browser_.dfl_) 
				cc.id_.fetch(dfp.first,dfp.second);
#else /* FIM_USE_CXX11 */
			std::for_each(cc.browser_.dfl_.begin(),cc.browser_.dfl_.end(),fim_cc_fetch);
#endif /* FIM_USE_CXX11 */
			browser_.cache_.desc_update();
		}
#endif /* FIM_WANT_PIC_CMTS_RELOAD */

		if(2 > args.size()-aoc )
			goto err;

		if ( args[aoc] == "load" )
		{
			if(2 < args.size()-aoc)
				sc = *args[2+aoc].c_str();
			this->id_.fetch(args[1+aoc],sc);
			browser_.cache_.desc_update();
		}
		else
		if ( args[aoc] == "save" )
		{
			bool saveall = fim_args_opt_have(args,"-all");
			bool wappend = fim_args_opt_have(args,"-append");
			bool nooverw = fim_args_opt_have(args,"-nooverw");
			string sdn = args[aoc+1];

			if(2 < args.size()-aoc)
				sc = *args[2+aoc].c_str();

			if(nooverw && is_file_nonempty(sdn))
				goto err;
			browser_.dump_desc(args[1+aoc].c_str(),sc,saveall,wappend);
		}
		else
			goto err;
err:
#endif /* FIM_WANT_PIC_CMTS */
		return FIM_CNS_EMPTY_RESULT;
	}
} /* namespace fim */
