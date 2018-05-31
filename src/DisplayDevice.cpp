/* $LastChangedDate: 2017-10-07 17:51:37 +0200 (Sat, 07 Oct 2017) $ */
/*
 DisplayDevice.cpp : virtual device Fim driver file

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

#include "fim.h"
#include "DisplayDevice.h"

#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	DisplayDevice::DisplayDevice(MiniConsole& mc):
	finalized_(false)
	,mc_(mc)
	,f_(FIM_NULL)
	,fontname_(FIM_NULL)
#else /* FIM_WANT_NO_OUTPUT_CONSOLE */
	DisplayDevice::DisplayDevice(void):
	finalized_(false)
	,f_(FIM_NULL)
	,fontname_(FIM_NULL)
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
	,redraw_(FIM_REDRAW_UNNECESSARY)
	{
		const fim_char_t *line;

	    	if (FIM_NULL != (line = fim_getenv(FIM_ENV_FBFONT)))
			fontname_ = line;
	}

	fim_sys_int DisplayDevice::get_input(fim_key_t * c, bool want_poll)
	{
		fim_sys_int r=0;
		*c=0;
#ifdef  FIM_SWITCH_FIXUP
			/* This way the console switches the right way. */
			/* (The following code dates back to the original fbi.c)
			 */

			/*
			 * patch: the following read blocks the program even when switching console
			 */
			//r=read(fim_stdin_,&c,1); if(c==0x1b){read(0,&c,3);c=(0x1b)+(c<<8);}
			/*
			 * so the next coded shoul circumvent this behaviour!
			 */
			{
				fd_set set;
				fim_sys_int fdmax;
				struct timeval  limit;
				fim_sys_int timeout=1,rc,paused=0;
	
			        FD_ZERO(&set);
			        FD_SET(cc.fim_stdin_, &set);
			        fdmax = 1;
#ifdef FBI_HAVE_LIBLIRC
				/*
				 * expansion code :)
				 */
			        if (-1 != lirc)
				{
			            FD_SET(lirc,&set);
			            fdmax = lirc+1;
			        }
#endif /* FBI_HAVE_LIBLIRC */
			        limit.tv_sec = timeout;
			        limit.tv_usec = 0;
			        rc = select(fdmax, &set, FIM_NULL, FIM_NULL,
			                    (0 != timeout && !paused) ? &limit : FIM_NULL);
				if(handle_console_switch())
				{
					r=-1;	/* originally a 'continue' in a loop */
					goto ret;
				}
				
				if (FD_ISSET(cc.fim_stdin_,&set))
					rc = read(cc.fim_stdin_, c, 4);
				r=rc;
				*c=int2msbf(*c);
			}
#else  /* FIM_SWITCH_FIXUP */
			/* This way the console switches the wrong way. */
			r=read(fim_stdin_,&c,4);	//up to four chars should suffice
#endif  /* FIM_SWITCH_FIXUP */
ret:		return r;
	}

	fim_key_t DisplayDevice::catchInteractiveCommand(fim_ts_t seconds)const
	{
		/*	
		 * This call 'steals' circa 1/10 of second..
		 */
		fd_set          set;
		ssize_t rc=0,r;
		fim_key_t c=-1;/* -1 means 'no character pressed */
		struct termios tattr, sattr;
		//we set the terminal in raw mode.
                if (! isatty(cc.fim_stdin_))
		{
                        sleep(seconds);
			goto ret;
		}

		FD_SET(0, &set);
		//fcntl(0,F_GETFL,&saved_fl);
		tcgetattr (0, &sattr);
		//fcntl(0,F_SETFL,O_BLOCK);
		memcpy(&tattr,&sattr,sizeof(struct termios));
		tattr.c_lflag &= ~(ICANON|ECHO);
		tattr.c_cc[VMIN]  = 0;
		tattr.c_cc[VTIME] = 1 * (seconds==0?1:(seconds*10)%256);
		tcsetattr (0, TCSAFLUSH, &tattr);
		//r=read(fim_stdin_,&c,4);
		// FIXME : read(.,.,3) is NOT portable. DANGER
		r=read(cc.fim_stdin_,&c,1); if(r>0&&c==0x1b){rc=read(0,&c,3);if(rc)c=(0x1b)+(c<<8);/* we should do something with rc now */}
		//we restore the previous console attributes
		tcsetattr (0, TCSAFLUSH, &sattr);
		if( r<=0 )
			c=-1;	
ret:		return c; /* read key */
	}

void DisplayDevice::clear_screen_locking(void)
{
	this->lock();
	fim_int ls=cc.getIntVariable(FIM_VID_CONSOLE_ROWS);
	fim_coo_t fh=f_?f_->sheight():1;
	ls=FIM_MIN(ls,height()/fh);
	clear_rect(0, width()-1, 0,fh*ls);
	this->unlock();
}

#ifndef FIM_KEEP_BROKEN_CONSOLE
void DisplayDevice::fb_status_screen_new(const fim_char_t *msg, fim_bool_t draw, fim_flags_t flags)
{
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	fim_err_t r=FIM_ERR_NO_ERROR;
	
	if(flags==0x03)
	{
		/* clear screen sequence */
		mc_.clear();
		goto ret;
	}

	if( flags==0x01 ) { mc_.scroll_down(); goto ret; }
	if( flags==0x02 ) { mc_.scroll_up(); goto ret; }

	r=mc_.add(msg);
	if(r==FIM_ERR_BUFFER_FULL)
	{
		r=mc_.grow();
		if(r==FIM_ERR_GENERIC)
			goto ret;
		r=mc_.add(msg);
		if(r==FIM_ERR_GENERIC)
			goto ret;
	}

	if(!draw )
		goto ret;
	clear_screen_locking();
	mc_.dump();
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
ret:
	return;
}
#endif /* FIM_KEEP_BROKEN_CONSOLE */

fim_err_t DisplayDevice::console_control(fim_cc_t arg)
{
	if(arg==0x01)
		fb_status_screen_new(FIM_NULL,false,arg);
	if(arg==0x02)
		fb_status_screen_new(FIM_NULL,false,arg);
	if(arg==0x03)
		fb_status_screen_new(FIM_NULL,false,arg);
	return FIM_ERR_NO_ERROR;
}

fim_err_t DisplayDevice::format_console(void)
{
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	if(f_)
	{	
		mc_.setRows ((height()/f_->sheight())/2);
		mc_.reformat( width() /f_->swidth()    );
	}
	else
	{
		mc_.setRows ( height()/2 );
		mc_.reformat( width()    );
	}
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
	return FIM_ERR_NO_ERROR;
}

DisplayDevice::~DisplayDevice(void)
{
}

#if FIM_WANT_BENCHMARKS
fim_int DisplayDevice::get_n_qbenchmarks(void)const
{
	return 1;
}

string DisplayDevice::get_bresults_string(fim_int qbi, fim_int qbtimes, fim_fms_t qbttime)const
{
	std::ostringstream oss;
	string msg;

	switch(qbi)
	{
		case 0:
		oss << "fim display check" << " : " << (float)(((fim_fms_t)(qbtimes*2))/((qbttime)*1.e-3)) << " clears/s\n";
		msg=oss.str();
	}
	return msg;
}

void DisplayDevice::quickbench_init(fim_int qbi)
{
	switch(qbi)
	{
		case 0:
		std::cout << "fim display check" << " : " << "please be patient\n";
		break;
	}
}

void DisplayDevice::quickbench_finalize(fim_int qbi)
{
}

void DisplayDevice::quickbench(fim_int qbi)
{
	/*
		a quick draw benchmark and sanity check.
		currently performs only the clear function.
	*/
	fim_coo_t x1 = 0, x2 = width()-1, y1 = 0, y2 = height()-1;
	switch(qbi)
	{
		case 0:
		this->lock();
		// clear_rect(x1, x2, y1, y2);
		fill_rect(x1, x2, y1, y2, FIM_CNS_WHITE);
		this->unlock();
		this->lock();
		fill_rect(x1, x2, y1, y2, FIM_CNS_BLACK);
		this->unlock();
		break;
	}
}
#endif /* FIM_WANT_BENCHMARKS */

	fim_err_t DisplayDevice::resize(fim_coo_t w, fim_coo_t h){return FIM_ERR_NO_ERROR;}
	fim_err_t DisplayDevice::reinit(const fim_char_t *rs){return FIM_ERR_NO_ERROR;}
	fim_err_t DisplayDevice::set_wm_caption(const fim_char_t *msg){return FIM_ERR_UNSUPPORTED;}

	fim_coo_t DisplayDevice::suggested_font_magnification(const fim_coo_t wf, const fim_coo_t hf)const
	{
		// suggests a font magnification until font is wider than 1/wf and taller than 1/hf wrt screen.
		// if a value is non positive, it will be ignored.
		fim_coo_t fmf = 1;

		if(wf>0 && hf<0)
			while ( wf * fmf * this->f_->width  < this->width() )
				++fmf;
		if(wf<0 && hf>0)
			while ( hf * fmf * this->f_->height < this->height() )
				++fmf;
		if(wf>0 && hf>0)
			while ( wf * fmf * this->f_->width  < this->width() &&
				hf * fmf * this->f_->height < this->height() )
				++fmf;
		return fmf;
	}

	fim_err_t DisplayDevice::fs_putc(struct fs_font *f, fim_coo_t x, fim_coo_t y, const fim_char_t c)
	{
		fim_char_t s[2] = {c,FIM_SYM_CHAR_NUL};
		return fs_puts(f, x, y, s);
	}

	void DisplayDevice::fs_multiline_puts(const char *str, fim_int doclear, int vw, int wh)
	{
		int fh=this->f_ ? this->f_->sheight():1; // FIXME : this is not clean
		int fw=this->f_ ? this->f_->swidth():1; // FIXME : this is not clean
		int sl = strlen(str), rw = vw / fw;
		int cpl = this->get_chars_per_line();

		if(doclear && cpl)
		{
			int lc = FIM_INT_FRAC(sl,cpl); /* lines count */
			this->clear_rect(0, vw-1, 0, FIM_MIN(fh*lc,wh-1));
		}

		for( int li = 0 ; sl > rw * li ; ++li )
			if((li+1)*fh<wh) /* FIXME: maybe this check shall better reside in fs_puts() ? */
			this->fs_puts(this->f_, 0, fh*li, str+rw*li);
	}

