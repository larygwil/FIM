/* $LastChangedDate: 2015-12-19 01:44:41 +0100 (Sat, 19 Dec 2015) $ */
/*
 readline.cpp : Code dealing with the GNU readline library.

 (c) 2008-2015 Michele Martone

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

#include "CommandConsole.h"
#include <iostream>
#ifdef FIM_USE_READLINE
#include "readline.h"
#endif /* FIM_USE_READLINE */
#ifdef FIM_USE_READLINE
#include "fim.h"

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))

#define FIM_COMPLETE_ONLY_IF_QUOTED  1
#define FIM_COMPLETE_INSERTING_DOUBLE_QUOTE  0
#define FIM_WANT_RL_KEY_DUMPOUT 0

/*
 * This file is severely messed up :).
 * */

static int fim_rl_pc=FIM_SYM_CHAR_NUL;

namespace fim
{
	extern CommandConsole cc;
fim_char_t * fim_readline(const fim_char_t *prompt)
{
	fim_char_t * rc=FIM_NULL;
	fim_rl_pc=FIM_SYM_CHAR_NUL;
	rc=readline(prompt);
	fim_rl_pc=FIM_SYM_CHAR_NUL;
	return rc;
}
}

/*
 * in fim.cpp
 * */
extern fim::string g_fim_output_device;

/* Generator function for command completion.  STATE lets us
 *    know whether to start from scratch; without any state
 *       (i.e. STATE == 0), then we start at the top of the list. */
static fim_char_t * command_generator (const fim_char_t *text,int state)
{
//	static int list_index, len;
//	fim_char_t *name;
	/* If this is a new word to complete, initialize now.  This
	 *      includes saving the length of TEXT for efficiency, and
	 *	initializing the index variable to 0. 
	 */
	return cc.command_generator(text,state,0);

		
//	if (!state) { list_index = 0; len = strlen (text); }

        /* Return the next name which partially matches from the
	 * command list.
	 */

//	while (name = commands[list_index].name)
//	{ list_index++; if (strncmp (name, text, len) == 0) return (dupstr(name)); }
	/* If no names matched, then return FIM_NULL. */
//	return ((fim_char_t *)FIM_NULL);
}

static fim_char_t * varname_generator (const fim_char_t *text,int state)
{
	return cc.command_generator(text,state,4);
}


namespace rl
{
#if FIM_WANT_READLINE_CLEAR_WITH_ESC
	static int fim_want_rl_cl_with_esc;
#endif /* FIM_WANT_READLINE_CLEAR_WITH_ESC */
/* 
 * Attempt to complete on the contents of TEXT.  START and END
 *     bound the region of rl_line_buffer that contains the word to
 *     complete.  TEXT is the word to complete.  We can use the entire
 *     contents of rl_line_buffer in case we want to do some simple
 *     parsing.  Return the array of matches, or FIM_NULL if there aren't any.
 */
static fim_char_t ** fim_completion (const fim_char_t *text, int start,int end)
{
	//FIX ME
	fim_char_t **matches = (fim_char_t **)FIM_NULL;

	if(start==end && end<1)
	{
#if 0
		fim_char_t **__s,*_s;
		_s=dupstr("");
		if(! _s)return FIM_NULL;
		__s=(fim_char_t**)fim_calloc(1,sizeof(fim_char_t*));
		if(!__s)return FIM_NULL;__s[0]=_s;
		//we print all of the commands, with no completion, though.
#endif
		cc.print_commands();
		rl_attempted_completion_over = 1;
		/* this could be set only here :) */
		return FIM_NULL;
	}

        /* If this word is at the start of the line, then it is a command
	*  to complete.  Otherwise it is the name of a file in the current
	*  directory.
	*/
        if (start == 0)
	{
		//std::cout << "completion for word " << start << "\n";
		matches = rl_completion_matches (text, command_generator);
	}
	else 
	{
		if(start>0 && !fim_isquote(rl_line_buffer[start-1]) )
		{
#if FIM_COMPLETE_INSERTING_DOUBLE_QUOTE  
			// FIXME: this is NEW
			if(start==end && fim_isspace(rl_line_buffer[start-1]))
			{
				fim_char_t**sp=(fim_char_t**)malloc(2*sizeof(fim_char_t*));
				sp[0]=dupstr("\"");
				sp[1]=FIM_NULL;
				rl_completion_append_character = '\0';
				fim::cout << "you can type double quoted string (e.g.: \"" FIM_CNS_EXAMPLE_FILENAME "\"), or a variable name (e.g.:" FIM_VID_FILELISTLEN "). some variables need a prefix (one of " FIM_SYM_NAMESPACE_PREFIXES ")\n" ;
				return sp;
			}
#endif /* FIM_COMPLETE_INSERTING_DOUBLE_QUOTE */
			if(start<end)
			{
				matches = rl_completion_matches (text, varname_generator);
				return matches;
			}
#if FIM_COMPLETE_ONLY_IF_QUOTED
			rl_attempted_completion_over = 1;
#endif /* FIM_COMPLETE_ONLY_IF_QUOTED */
		}
		//std::cout << "sorry, no completion for word " << start << "\n";
	}
        return (matches);
}

/*
 * 	this function is called to display the proposed autocompletions
 */
static void completion_display_matches_hook(fim_char_t **matches,int num,int max)
{
	/* FIXME : fix the oddities of this code */
	fim_char_t buffer[FIM_RL_COMPLETION_BUFSIZE];
	int w,f,l;w=0;f=sizeof(buffer)-1;l=0;
	buffer[0]='\0';
	if(!matches)return;
#define FIM_SHOULD_SUGGEST_POSSIBLE_COMPLETIONS 1
#if FIM_SHOULD_SUGGEST_POSSIBLE_COMPLETIONS 
	if(num>1)
		fim::cout << "possible completions for \""<<matches[0]<<"\":\n" ;
#endif /* FIM_SHOULD_SUGGEST_POSSIBLE_COMPLETIONS */
	for(int i=/*0*/1;i<num && matches[i] && f>0;++i)
	{
#if FIM_SHOULD_SUGGEST_POSSIBLE_COMPLETIONS 
		fim::cout << matches[i] << "\n";
#endif /* FIM_SHOULD_SUGGEST_POSSIBLE_COMPLETIONS  */
		w=min(strlen(matches[i])+1,(size_t)f);
		if(f>0){
		strncpy(buffer+l,matches[i],w);
		w=strlen(buffer+l);l+=w;f-=w;}
		if(f>0){strcpy(buffer+l," ");--f;++l;}
		buffer[l]='\0';
//		strcpy(buffer+strlen(buffer),matches[i]);
//		strcpy(buffer+strlen(buffer)," ");
	}

//	std::cout << buffer << "\n" ;
 //     status((fim_byte_t*)"here shall be autocompletions", FIM_NULL);
}

/*
static void redisplay_no_fb(void)
{
	printf("%s",rl_line_buffer);
}
*/

static void redisplay(void)
{	
	cc.set_status_bar(( fim_char_t*)rl_line_buffer,FIM_NULL);
}

/*
 * ?!
 * */
/*
static int redisplay_hook_no_fb(void)
{
	redisplay_no_fb();
	return 0;
}*/

static int fim_post_rl_getc(int c)
{
#if FIM_WANT_READLINE_CLEAR_WITH_ESC
	if(c==FIM_SYM_ESC && fim_want_rl_cl_with_esc)
	{
		if(rl_line_buffer)
			rl_point=0,
			rl_line_buffer[0]=FIM_SYM_PROMPT_NUL;

		c=FIM_SYM_ENTER;
#if FIM_WANT_DOUBLE_ESC_TO_ENTER
		if(fim_want_rl_cl_with_esc==-1)
			fim_want_rl_cl_with_esc=0;
#endif /* FIM_WANT_DOUBLE_ESC_TO_ENTER */
	}
#endif /* FIM_WANT_READLINE_CLEAR_WITH_ESC */
	if(FIM_WANT_RL_KEY_DUMPOUT)cout << "got key: " << (int)(c) << " : " << (c==FIM_SYM_ESC)<<"\n";
	return c;
}

#if defined(FIM_WITH_LIBSDL) || defined(FIM_WITH_AALIB) || defined(FIM_WITH_LIBIMLIB2)
static int fim_rl_sdl_aa_getc_hook(void)
{
	//unsigned int c;
	fim_key_t c;
	c=0;
	
	if(cc.get_displaydevice_input(&c,true)==1)
	{
		c=fim_post_rl_getc(c);
		if(c&(1<<31))
		{
			rl_set_keymap(rl_get_keymap_by_name("emacs-meta"));	/* FIXME : this is a dirty trick : */
			//c&=!(1<<31);		/* FIXME : a dirty trick */
			c&=0xFFFFFF^(1<<31);	/* FIXME : a dirty trick */
			//std::cout << "alt!  : "<< (fim_byte_t)c <<" !\n";
			//rl_stuff_char(c);	/* warning : this may fail */
			rl_stuff_char(c);	/* warning : this may fail */
		}
		else
		{
			rl_set_keymap(rl_get_keymap_by_name("emacs"));		/* FIXME : this is a dirty trick : */
			//std::cout << "char in : "<< (fim_byte_t)c <<" !\n";
			rl_stuff_char(c);	/* warning : this may fail */
		}
		return 1;	
	}
	return 0;	
}

//void fim_rl_prep_dummy(int meta_flag){}
//void fim_rl_deprep_dummy(void){}

int fim_rl_sdl_aa_getc(FILE * fd)
{
	return 0;/* yes, a dummy function instead of getc() */
}
#endif /* defined(FIM_WITH_LIBSDL) || defined(FIM_WITH_AALIB) || defined(FIM_WITH_LIBIMLIB2) */

int fim_rl_getc(FILE * fd)
{
	int c=FIM_SYM_CHAR_NUL;
#if 1
	c=rl_getc(fd);
#if FIM_WANT_DOUBLE_ESC_TO_ENTER
	if(c==FIM_SYM_ESC)
	{
		if(fim_rl_pc==c)
			fim_want_rl_cl_with_esc=-1,
			fim_rl_pc=c;
		else
			fim_rl_pc=c;
			//c=FIM_SYM_CHAR_NUL;
	}
	else
#endif /* FIM_WANT_DOUBLE_ESC_TO_ENTER */
		fim_rl_pc=c;
#else
	/* the following code is not complete yet. it needs interpretation of the input sequence */
	int cc=rl_getc(fd);
	if(cc==FIM_SYM_ESC)
	{
		int tries=0;
		fim_char_t cb[4];
		cb[0]=cb[1]=cb[2]=cb[3]=FIM_SYM_CHAR_NUL;
		c|=cc;
		cb[0]=cc;
		if(FIM_WANT_RL_KEY_DUMPOUT)cout<<"adding: "<<((int)cc)<<"\n";
		for(tries=1;tries<3;++tries)
		if((cc=rl_getc(fd))==FIM_SYM_ESC)
		{
			ungetc(cc,fd);
			for(--tries;tries>1;--tries) ungetc(cb[tries],fd);
			c=cb[0];
			goto read_ok;
		}
		else
		{
			if(cc==FIM_SYM_CHAR_NUL)
			{
				for(--tries;tries>1;--tries) ungetc(cb[tries],fd);
				c=cb[0];
				goto read_ok;
			}
			if(FIM_WANT_RL_KEY_DUMPOUT)cout<<"adding: "<<((int)cc)<<"\n";
			c*=256;
			c|=cc;
			cb[tries]=cc;
			c=*(int*)cb;
		}
	}
	else 
		c=cc;
read_ok:
#endif
	c=fim_post_rl_getc(c);
	return c;
}

int fim_search_rl_startup_hook(void)
{
	const fim_char_t * hs=cc.browser_.last_regexp_.c_str();
	if(hs)
	{
		rl_replace_line(hs,0);
		rl_point=strlen(hs);
	}
	return 0;
}

static int redisplay_hook(void)
{
	redisplay();
	return 0;
}

/*
 * ?!
 * */
/*static int fim_rl_end(int a,int b)
{
	rl_point=rl_end;
	return 0;
}*/

/*
 * ?!
 * */
/*static int fim_set_command_line_text(const fim_char_t*s)
{
	rl_replace_line(s,0);
	return 0;
}*/


/*
 *	initial setup to set the readline library working
 */
void initialize_readline (fim_bool_t with_no_display_device, fim_bool_t wcs)
{
	//FIX ME
	/* Allow conditional parsing of the ~/.inputrc file. */
	rl_readline_name = "fim";	//??
	/* Tell the completer that we want a crack first. */
	rl_attempted_completion_function = fim_completion;
	rl_completion_display_matches_hook=completion_display_matches_hook;
	rl_erase_empty_line=1; // NEW: 20110630 in sdl mode with no echo disabling, prints newlines, if unset
#if FIM_WANT_READLINE_CLEAR_WITH_ESC
	fim_want_rl_cl_with_esc=1;
#endif /* FIM_WANT_READLINE_CLEAR_WITH_ESC */

#define FIM_WANT_COOKIE_STREAM 1 /* FIXME: for now, this is just a dirty hack for the no-stdin case; in the future this shall be completed and replace the rl_stuff_char trick. */
#if FIM_WANT_COOKIE_STREAM
	if(wcs)
       	{
		int isp[2];
		pipe(isp);
		rl_instream = fdopen(isp[0],"r");
		rl_outstream = fopen("/dev/null","w"); /* FIXME: seems like rl_erase_empty_line is not always working :-( */
	}
#endif /* FIM_WANT_COOKIE_STREAM */
	if(with_no_display_device==0)
	{
		rl_catch_signals=0;
		rl_catch_sigwinch=0;
		rl_redisplay_function=redisplay;
	        rl_event_hook=redisplay_hook;
	        rl_pre_input_hook=redisplay_hook;
	}
#if defined(FIM_WITH_LIBSDL) || defined(FIM_WITH_AALIB) || defined(FIM_WITH_LIBIMLIB2)
	//if( g_fim_output_device==FIM_DDN_INN_SDL 
	if(g_fim_output_device.find(FIM_DDN_INN_SDL)==0
		/* uncommenting the following may give problems; but commenting it will break X11-backed aalib input ..  */ 
		|| g_fim_output_device==FIM_DDN_INN_AA
		|| g_fim_output_device==FIM_DDN_INN_IL2
	)
	{
		rl_getc_function=fim_rl_sdl_aa_getc;
		rl_event_hook   =fim_rl_sdl_aa_getc_hook;
//		rl_prep_term_function=fim_rl_prep_dummy;
//		rl_deprep_term_function=fim_rl_deprep_dummy;

		/*
                 * FIXME : The following hack uses SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, all -0x100 ..
                 *         (/usr/include/SDL/SDL_keysym.h)
                 *
                 *         Regard this as a problem.
		 */
 		rl_bind_keyseq("\x11", rl_get_previous_history);	// up
 		rl_bind_keyseq("\x12", rl_get_next_history);		// down
 		rl_bind_keyseq("\x13", rl_forward_char);		// right
 		rl_bind_keyseq("\x14", rl_backward_char);		// left
	}
	#endif
	if(
			g_fim_output_device.find(FIM_DDN_INN_FB)==0 ||
			g_fim_output_device.find(FIM_DDN_INN_AA)==0 ||
			0
	  )
#if FIM_WANT_READLINE_CLEAR_WITH_ESC
	if(
		       	g_fim_output_device==FIM_DDN_INN_AA
		       	|| g_fim_output_device==FIM_DDN_INN_FB
	  )
	{
		fim_want_rl_cl_with_esc=0;
		rl_getc_function=fim_rl_getc;
	}
#endif /* FIM_WANT_READLINE_CLEAR_WITH_ESC */
	//rl_completion_entry_function=FIM_NULL;
	/*
	 * to do:
	 * see rl_filename_quoting_function ..
	 * */
	//rl_inhibit_completion=1;	//if set, TABs are read as normal characters
	rl_filename_quoting_desired=1;
	rl_filename_quote_characters="\"";
	//rl_reset_terminal("linux");
	//rl_reset_terminal("vt100");
	//rl_bind_key(0x09,fim_rl_end);
	//rl_bind_key(0x7F,fim_rl_end);
	//rl_bind_key(-1,fim_rl_end);
	//rl_bind_key('~',fim_rl_end); // ..
	//rl_bind_key('\t',rl_insert);
	//rl_bind_keyseq("g",fim_rl_end);
	//rl_set_prompt("$");

/*	rl_voidfunc_t *rl_redisplay_function=redisplay;
	rl_hook_func_t *rl_event_hook=redisplay_hook;
	rl_hook_func_t *rl_pre_input_hook=redisplay_hook;*/
	//std::cout << "readline initialized\n";
}


}

#endif /* FIM_USE_READLINE */
