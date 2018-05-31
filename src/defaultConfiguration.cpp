/* $LastChangedDate: 2017-07-16 16:17:23 +0200 (Sun, 16 Jul 2017) $ */
/*
 defaultConfiguration.cpp :
  The following code contains key bindings and aliases definitions
  which make up a first usable configuration for FIM.
 
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
#if FIM_DEFAULT_CONFIG
#if 1
		alias(FIM_FLA_MAGNIFY,FIM_FLC_MAGNIFY,"magnify the displayed image by the " FIM_VID_MAGNIFY_FACTOR " variable or " FIM_CNS_EX_ARGS_STRING "");
		alias(FIM_FLA_REDUCE ,FIM_FLC_REDUCE,"reduce the displayed image by " FIM_VID_REDUCE_FACTOR " or " FIM_CNS_EX_ARGS_STRING "");
 		alias(FIM_FLA_NEXT,FIM_FLC_NEXT,"go to the next page or picture file");
 		alias(FIM_FLA_PREV,FIM_FLC_PREV,"go to the previous page or picture file");
 		alias(FIM_FLA_NEXT_PAGE,FIM_FLC_NEXT_PAGE,"go to the next page in the file");
 		alias(FIM_FLA_PREV_PAGE,FIM_FLC_PREV_PAGE,"go to the previous page in the file");
 		alias(FIM_FLA_NEXT_FILE,FIM_FLC_NEXT_FILE,"go to the next file in the list");
 		alias(FIM_FLA_PREV_FILE,FIM_FLC_PREV_FILE,"go to the previous file in the list");
#endif
#if 1
 		alias("scale_factor_grow","scale '+*'","multiply the scale factors " FIM_VID_REDUCE_FACTOR " and " FIM_VID_MAGNIFY_FACTOR " by " FIM_VID_SCALE_FACTOR_MULTIPLIER "");
 		alias("scale_factor_shrink","scale '+/'","divide the scale factors " FIM_VID_REDUCE_FACTOR " and " FIM_VID_MAGNIFY_FACTOR " by " FIM_VID_SCALE_FACTOR_MULTIPLIER "");
 		alias("scale_factor_decrease","scale '+-'","subtract " FIM_VID_SCALE_FACTOR_DELTA " to the scale factors " FIM_VID_REDUCE_FACTOR " and " FIM_VID_MAGNIFY_FACTOR "");
 		alias("scale_factor_increase","scale '++'","add " FIM_VID_SCALE_FACTOR_DELTA " to the scale factors " FIM_VID_REDUCE_FACTOR " and " FIM_VID_MAGNIFY_FACTOR "");
 		alias('A',"'" FIM_VID_AUTOTOP "=1-" FIM_VID_AUTOTOP ";'");
#endif
#if 0
 		alias('A',"'autotop=1-autotop;'");
		alias("toggleVerbosity","verbose=1-verbose;display;");
		alias("toggleKeyVerbosity","_verbose_keys=1-_verbose_keys;display;");
#endif
#if 0
		bind('h',FIM_FLT_HELP);
		bind('p',"prev;display;");
		bind('r',FIM_FLT_RELOAD);
		bind('R',FIM_FLT_REDISPLAY);
		bind('m',FIM_FLC_MIRROR);
		bind('f',FIM_FLC_FLIP);
		bind(4283163,"pan 'up'");
		bind(4414235,"pan 'right'");
		bind(4348699,"pan 'down'");
		bind(4479771,"pan 'left'");
		bind(' ',FIM_FLT_SCROLLDOWN);
		bind(2117491483,"next");
		bind(2117425947,"prev");
		bind('i',FIM_FLT_INFO);
#endif
#if 1
		bind('q',FIM_FLT_QUIT);
		bind('n',FIM_FLC_NEXT_FILE);
		bind('p',FIM_FLC_PREV_FILE);
		bind('d',FIM_FLT_DISPLAY);
		bind('+',FIM_FLA_MAGNIFY);
		bind('-',FIM_FLA_REDUCE);
		//bind('k',FIM_FLC_PAN_UP);
		//bind('j',FIM_FLC_PAN_DOWN);
		bind('k',FIM_FLC_SCROLL_UP);
		bind('j',FIM_FLC_SCROLL_DOWN);
		bind('h',FIM_FLC_PAN_LEFT);
		bind('l',FIM_FLC_PAN_RIGHT);
		bind(29,"scale_factor_increase");
		bind(31,"scale_factor_decrease");
		bind(29,"scale_factor_grow");
		bind(31,"scale_factor_shrink");
#endif
#if 1
		setVariable(FIM_VID_CONSOLE_KEY,FIM_SYM_CONSOLE_KEY);
		setVariable(FIM_VID_PUSH_PUSHES_DIRS,1);
#endif
#if 0
		setVariable(FIM_VID_REDUCE_FACTOR,FIM_CNS_SCALEFACTOR);
		setVariable(FIM_VID_MAGNIFY_FACTOR,FIM_CNS_SCALEFACTOR);
		setVariable(FIM_VID_SCALE_FACTOR_DELTA,FIM_CNS_SCALEFACTOR_DELTA);
		setVariable(FIM_VID_VERBOSE_KEYS,-1);
		setVariable(FIM_VID_MAGNIFY_FACTOR,FIM_CNS_SCALEFACTOR);
		setVariable(FIM_VID_SCALE_FACTOR_MULTIPLIER,FIM_CNS_SCALEFACTOR_MULTIPLIER);
#endif
#endif
/*
 * The following code binds key codes to key symbols.
 * In this way the user can write a configuration file
 * and associate actions to key configurations..
 *
 * Note : certain key bindings (especially Control key with arrows or other non letter keys ) are known to be non working.
 * But by modifying this file one may experiment!
 */
#if FIM_DEFAULT_KEY_CONFIG
		/*
		 * Standard key configuration
		 */
		fim_char_t k[4];k[1]='\0';

		/*	97=0x61 to 122=0x7A	*/
		for(fim_char_t i='a';i<='z';++i) *k=i, sym_keys_[k]=i;
		
		/*	65=0x41 to 90=0x5A (shifted keys)	*/
		for(fim_char_t i='A';i<='Z';++i) *k=i, sym_keys_[k]=i;
		
		/*	32=0x20 to 65=0x40	*/
		for(fim_char_t i=' ';i<='@';++i) *k=i, sym_keys_[k]=i;

		/*	91=0x5B to 96=0x60	*/
		for(fim_char_t i='[';i<'a';++i) *k=i, sym_keys_[k]=i;

		/*	123=0x7B to 127=0x7F (DEL)	*/
		for(fim_key_t i='{';i<=126;++i) *k=fim_char_t(i), sym_keys_[k]=(fim_char_t)i;
		// A note : DEL is not captured by the above configuration.
		
		/*
		 * Control keys handling
		 */
		k[0]='C'; k[1]='-'; k[3]='\0';
		//problem : no uppercase with control.sorry.
		for(fim_key_t i='a';i<='j';++i) k[2]=fim_char_t(i), sym_keys_[k]=((i-'a')+1);
		for(fim_key_t i='k';i<='z';++i) k[2]=fim_char_t(i), sym_keys_[k]=((i-'a')+1);
		// for(fim_key_t i=' ';i<='?';++i) k[2]=fim_char_t(i), sym_keys_[k]=( i+1)-'a' ;

		/*
		 * The following bindings are known to be useless or bugful.
		*/
		/*
		for(fim_char_t i='[';i<'a';++i,k[2]=i) sym_keys_[k]=(i+1);
		for(int i='{';i<=127;++i,k[2]=(fim_char_t)i) sym_keys_[k]=(i+1)(fim_char_t)i;
		*/
		
		/*
		 * The following means that when a decimal 4479771 will be read from
		 * the keyboard, it will be interpreted as a Left pressure.
		 */
		sym_keys_[FIM_KBD_LEFT]=4479771;
		// and so on..
		sym_keys_[FIM_KBD_RIGHT]=4414235;
		sym_keys_[FIM_KBD_UP]=4283163;
		sym_keys_[FIM_KBD_DOWN]=4348699;
		sym_keys_[FIM_KBD_PAGEDOWN]=2117491483;
		sym_keys_[FIM_KBD_PAGEUP]=2117425947;
		sym_keys_[FIM_KBD_ENTER]=10;
		sym_keys_[FIM_KBD_BACKSPACE_]=127;
		sym_keys_[FIM_KBD_BACKSPACE]=127;
		sym_keys_[FIM_KBD_DEL]=2117294875;
		sym_keys_[FIM_KBD_INS]=2117229339;
		sym_keys_[FIM_KBD_HOME]=2117163803;
		sym_keys_[FIM_KBD_END] =2117360411;
		sym_keys_[FIM_KBD_TAB] =9;
		sym_keys_[FIM_KBD_ANY] =0;	/* EXPERIMENTAL */
		sym_keys_[FIM_KBD_ESC] =27;

		exitBinding_ = sym_keys_[FIM_KBD_ENTER];	/*  Enter will terminate loops */
//		sym_keys_[" "]=' ';
//		sym_keys_["."]='.';
//		sym_keys_[","]=',';
//
		cc.key_syms_update();
#endif
