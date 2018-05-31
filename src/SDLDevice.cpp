/* $LastChangedDate: 2018-01-28 18:13:26 +0100 (Sun, 28 Jan 2018) $ */
/*
 SDLDevice.cpp : sdllib device Fim driver file

 (c) 2008-2018 Michele Martone
 based on code (c) 1998-2006 Gerd Knorr <kraxel@bytesex.org>

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

/*
 * NOTES : The SDL support is INCOMPLETE:
 *
 *  - input problems when coupled with readline
 */
#include "fim.h"

#ifdef FIM_WITH_LIBSDL

#include "SDLDevice.h"
#define FIM_SDL_FLAGS /*SDL_FULLSCREEN|*/SDL_HWSURFACE

#define FIM_WANT_HARDCODED_ICON 1
#define FIM_SDL_ICONPATH ""
#define FIM_FRAC(VAL,N,D) (((VAL)*(N))/(D))

namespace fim
{
	extern fim_int fim_fmf_; /* FIXME */
	extern CommandConsole cc;
}

#define FIM_SDL_MINWIDTH 2
#define FIM_SDL_MINHEIGHT 2

#define FIM_SDL_ALLOW_QUIT 1
#define FIM_SDL_WANT_KEYREPEAT 1
#define FIM_SDL_WANT_RESIZE 1
#define FIM_SDL_DEBUG 1
#undef FIM_SDL_DEBUG
#define FIM_WANT_MOUSE_PAN 1
#define FIM_WANT_SDL_CLICK_MOUSE_SUPPORT 1 && FIM_WANT_SDL_PROOF_OF_CONCEPT_MOUSE_SUPPORT
#define FIM_WANT_SDL_CLICK_MENU FIM_WANT_SDL_CLICK_MOUSE_SUPPORT && 1

#ifdef FIM_SDL_DEBUG
#define FIM_SDL_INPUT_DEBUG(C,MSG)  \
/* i miss sooo much printf() :'( */ \
std::cout << (size_t)getmilliseconds() << " : "<<MSG<<" : "; \
std::cout.setf ( std::ios::hex, std::ios::basefield ); \
std::cout.setf ( std::ios::showbase ); \
std::cout << *(fim_key_t*)(C) <<"\n"; \
std::cout.unsetf ( std::ios::showbase ); \
std::cout.unsetf ( std::ios::hex );
#else /* FIM_SDL_DEBUG */
#define FIM_SDL_INPUT_DEBUG(C,MSG) {}
#endif /* FIM_SDL_DEBUG */
#define FIM_WANT_POSITION_DISPLAYED FIM_WANT_MOUSE_PAN && 0

typedef int fim_sdl_int;

#if FIM_WANT_SDL_PROOF_OF_CONCEPT_MOUSE_SUPPORT
static fim_int fim_draw_help_map_; // This is static since it gets modified in a static function. This shall change.
static fim_char_t key_char_grid[10] = "'pP+a-=nN"; // by columns left to right, up to down. note that this assignment sets NUL.
static void toggle_draw_help_map(void)
{
       	fim_draw_help_map_=(fim_draw_help_map_+1)%3;
	if(Viewport* cv = cc.current_viewport())
		cv->redisplay(); // will indirectly trigger draw_help_map()
}
static fim_int fim_draw_help_map_tmp_=0; // 
static void flip_draw_help_map_tmp(bool flip)
{
#if FIM_WANT_SDL_CLICK_MENU
	if(!flip)
		if(fim_draw_help_map_tmp_==0)
			return; // nothing to do
	fim_draw_help_map_tmp_=1-fim_draw_help_map_tmp_;
	if(fim_draw_help_map_tmp_)
	if(Viewport* cv = cc.current_viewport())
		cv->redisplay(); // will indirectly trigger draw_help_map()
#endif /* FIM_WANT_SDL_CLICK_MENU */
}
#endif /* FIM_WANT_SDL_PROOF_OF_CONCEPT_MOUSE_SUPPORT */

fim_err_t SDLDevice::parse_optstring(const fim_char_t *os)
{
	bool want_windowed=want_windowed_;
	bool want_mouse_display=want_mouse_display_;
	bool want_resize=want_resize_;
	fim_coo_t current_w=current_w_;
	fim_coo_t current_h=current_h_;

	if(os)
	{
		while(*os&&!isdigit(*os))
		{
			bool tv=true;
			if(isupper(*os))
				tv=false;
			switch(tolower(*os)){
				case 'w': want_windowed=tv; break;
				case 'm': want_mouse_display=tv; break;
				case 'r': want_resize=tv; break;
				default: std::cerr<<"unrecognized specifier character \""<<*os<<"\"\n";goto err;
			}
			++os;
		}
		if(*os)
		{
			if(2==sscanf(os,"%d:%d",&current_w,&current_h))

			{
			//	std::cout << w << " : "<< h<<"\n";
				current_w=FIM_MAX(current_w,0);
				current_h=FIM_MAX(current_h,0);
				if(!allowed_resolution(current_w,current_h))
					goto err;
			}
			else
			{
				current_w=current_h=0;
				std::cerr << "user specification of resolution (\""<<os<<"\") wrong: it shall be in \"width:height\" format! \n";
				// TODO: a better invaling string message needed here
			}
		}
	}
	// commit
	want_windowed_=want_windowed;
	want_mouse_display_=want_mouse_display;
#if FIM_SDL_WANT_RESIZE 
	want_resize_=want_resize;
#else /* FIM_SDL_WANT_RESIZE */
	want_resize_=false;
#endif /* FIM_SDL_WANT_RESIZE */
	current_w_=current_w;
	current_h_=current_h;
	return FIM_ERR_NO_ERROR;
err:
	return FIM_ERR_GENERIC;
}

#if FIM_WANT_POSITION_DISPLAYED
static int gx,gy;
#endif /* FIM_WANT_POSITION_DISPLAYED */

#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	SDLDevice::SDLDevice(MiniConsole& mc_, fim::string opts):DisplayDevice(mc_),
#else /* FIM_WANT_NO_OUTPUT_CONSOLE */
	SDLDevice::SDLDevice(
			fim::string opts
			):DisplayDevice(),
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
	screen_(FIM_NULL),
	vi_(FIM_NULL),
	current_w_(0), current_h_(0),
	Bpp_(FIM_CNS_BPP_INVALID),
	bpp_(FIM_CNS_BPP_INVALID),
	opts_(opts),
	want_windowed_(FIM_SDL_FLAGS & SDL_FULLSCREEN ? false : true),
	want_mouse_display_(true),
	want_resize_(true)
	{
		FontServer::fb_text_init1(fontname_,&f_);	// FIXME : move this outta here
#if FIM_WANT_SDL_PROOF_OF_CONCEPT_MOUSE_SUPPORT
		fim_draw_help_map_=0;
#endif /* FIM_WANT_SDL_PROOF_OF_CONCEPT_MOUSE_SUPPORT */
		keypress_ = 0;
#if FIM_WANT_SDL_OPTIONS_STRING 
		const fim_char_t*const os=opts_.c_str();
		parse_optstring(os);
#endif /* FIM_WANT_SDL_OPTIONS_STRING */
		fim_bzero(&bvi_,sizeof(bvi_));
		//current_w_=current_h_=0;
	}

	fim_err_t SDLDevice::clear_rect_(
		void* dst,	// destination array 
		fim_coo_t oroff,fim_coo_t ocoff,	// row  and column  offset of the first output pixel
		fim_coo_t orows,fim_coo_t ocols,	// rows and columns drawable in the output buffer
		fim_coo_t ocskip		// output columns to skip for each line
	)
	{
		/* output screen variables */
//		fim_coo_t 
//			oi,// output image row index
//			oj;// output image columns index

		//fim_coo_t lor,loc;
    		
		if( oroff <0 )
		       	return -8;
		if( ocoff <0 )
		       	return -9;
		if( orows <=0 )
		       	return -10;
		if( ocols <=0 )
		       	return -11;
		if( ocskip<0 )
		       	return -12;

		if( oroff>orows )
		       	return -8-10*100;
		if( ocoff>ocols )
		       	return -9-11*100;
		if( ocskip<ocols )
		       	return -12-11*100;

		/*
		 * orows and ocols is the total number of rows and columns in the output window.
		 * no more than orows-oroff rows and ocols-ocoff columns will be rendered, however
		 * */

		//lor = orows-1;
		//loc = ocols-1;
		
		return  FIM_ERR_GENERIC;
	}

	fim_err_t SDLDevice::draw_help_map(void)
	{
		if(fim_draw_help_map_==0)
		{
			// nothing to draw
		}
		if(fim_draw_help_map_==2)
		{
			Viewport* cv = cc.current_viewport();
			fim_coo_t xw = cv->viewport_width();
			fim_coo_t yh = cv->viewport_height();
			fim_coo_t xt = xw/3;
			fim_coo_t yt = yh/3;
			fim_coo_t xtl = xt/16;
			fim_coo_t ytl = yt/16;
			fim_coo_t eth = 1; // extra thickness
			fim_color_t ic = FIM_CNS_WHITE;
			fim_color_t oc = FIM_CNS_BLACK;
			// test
			//fill_rect(xw/2, xw/2, 0, yh-1, ic, oc); // test middle vertical 
			//fill_rect(0, xw-1, yh/2, yh/2, ic, oc); // test middle horizontal
			// horizontal
			fill_rect(0, xtl, 1*yt-eth, 1*yt+eth, ic, oc); // left top
			fill_rect(0, xtl, 2*yt-eth, 2*yt+eth, ic, oc); // left bottom
			fill_rect(xw-xtl, xw-1, 1*yt-eth, 1*yt+eth, ic, oc); // right top
			fill_rect(xw-xtl, xw-1, 2*yt-eth, 2*yt+eth, ic, oc); // right bottom
			// vertical
			fill_rect(1*xt-eth, 1*xt+eth, 0*ytl, 1*ytl, ic, oc); // left top
			fill_rect(2*xt-eth, 2*xt+eth, 0*ytl, 1*ytl, ic, oc); // right top
			fill_rect(1*xt-eth, 1*xt+eth, yh-ytl, yh-1, ic, oc); // left bottom
			fill_rect(2*xt-eth, 2*xt+eth, yh-ytl, yh-1, ic, oc); // right bottom

#if FIM_WANT_POSITION_DISPLAYED
			fim_coo_t yp = gy;
			fim_coo_t xp = gx;

			if(yp>=eth && yp<yh-eth)
				fill_rect(0, xw, yp-eth, yp+eth, ic, oc); // h

			if(xp>=eth && xp<xw-eth)
				fill_rect(  xp-eth,   xp+eth, 0, yh-1, ic, oc); // v
#endif /* FIM_WANT_POSITION_DISPLAYED */
		}
		if(fim_draw_help_map_==1 || fim_draw_help_map_tmp_)
		{
			Viewport* cv = cc.current_viewport();
			fim_coo_t xw = cv->viewport_width();
			fim_coo_t yh = cv->viewport_height();
			fim_coo_t xt = xw/6;
			fim_coo_t yt = yh/6;
			fim_coo_t eth = 1;
			fim_coo_t fd = FIM_MAX(f_->swidth(),f_->sheight());

			if(xw > 6*fd && yh > 6*fd)
			{
				fim_color_t ic = FIM_CNS_WHITE;
				fim_color_t oc = FIM_CNS_BLACK;
				fill_rect(0*2*xt, xw, 1*2*yt-eth, 1*2*yt+eth, ic, oc);
				fill_rect(0*2*xt, xw, 2*2*yt-eth, 2*2*yt+eth, ic, oc);
				fill_rect(1*2*xt-eth, 1*2*xt+eth, 0, yh-1, ic, oc);
				fill_rect(2*2*xt-eth, 2*2*xt+eth, 0, yh-1, ic, oc);
				// left column
				fs_putc(f_, 1*xt, 1*yt, key_char_grid[0]);
				fs_putc(f_, 1*xt, 3*yt, key_char_grid[1]);
				fs_putc(f_, 1*xt, 5*yt, key_char_grid[2]);
				// middle column
				fs_putc(f_, 3*xt, 1*yt, key_char_grid[3]);
				fs_putc(f_, 3*xt, 3*yt, key_char_grid[4]);
				fs_putc(f_, 3*xt, 5*yt, key_char_grid[5]);
				// right column
				fs_putc(f_, 5*xt, 1*yt, key_char_grid[6]);
				fs_putc(f_, 5*xt, 3*yt, key_char_grid[7]);
				fs_putc(f_, 5*xt, 5*yt, key_char_grid[8]);
			}
		}
		return FIM_ERR_NO_ERROR;
	}

	fim_err_t SDLDevice::display(
		//const struct ida_image *img, // source image structure
		const void *ida_image_img, // source image structure
		//void* rgb,// source rgb array
		fim_coo_t iroff,fim_coo_t icoff, // row and column offset of the first input pixel
		fim_coo_t irows,fim_coo_t icols,// rows and columns in the input image
		fim_coo_t icskip,	// input columns to skip for each line
		fim_coo_t oroff,fim_coo_t ocoff,// row and column offset of the first output pixel
		fim_coo_t orows,fim_coo_t ocols,// rows and columns to draw in output buffer
		fim_coo_t ocskip,// output columns to skip for each line
		fim_flags_t flags// some flags
	)
	{
		
		/*
		 * TODO : generalize this routine and put in common.cpp
		 * */
		/*
		 * FIXME : centering mechanisms missing here; an intermediate function
		 * shareable with FramebufferDevice would be nice, if implemented in AADevice.
		 * */
		//was : void
		fim_byte_t* rgb = ida_image_img?((const struct ida_image*)ida_image_img)->data:FIM_NULL;// source rgb array

		if ( !rgb ) return -1;
	
		if( iroff <0 ) return -2;
		if( icoff <0 ) return -3;
		if( irows <=0 ) return -4;
		if( icols <=0 ) return -5;
		if( icskip<0 ) return -6;
		if( oroff <0 ) return -7;
		if( ocoff <0 ) return -8;
		if( orows <=0 ) return -9;
		if( ocols <=0 ) return -10;
		if( ocskip<0 ) return -11;
		if( flags <0 ) return -12;

		if( iroff>irows ) return -2-3*100 ;
		if( icoff>icols ) return -3-5*100;
//		if( oroff>orows ) return -7-9*100;//EXP
//		if( ocoff>ocols ) return -8-10*100;//EXP
		if( oroff>height() ) return -7-9*100;//EXP
		if( ocoff>width()) return -8-10*100;//EXP

		if( icskip<icols ) return -6-5*100;
		if( ocskip<ocols ) return -11-10*100;
	
		orows  = FIM_MIN( orows, height());
		ocols  = FIM_MIN( ocols,  width()); 
		ocskip = width(); 	//FIXME maybe this is not enough and should be commented or rewritten!

		if( orows  > height() ) return -9 -99*100;
		if( ocols  >  width() ) return -10-99*100;
		if( ocskip <  width() ) return -11-99*100;
		if( icskip<icols ) return -6-5*100;

		ocskip = width();// output columns to skip for each line

		if(icols<ocols) { ocoff+=(ocols-icols-1)/2; ocols-=(ocols-icols-1)/2; }
		if(irows<orows) { oroff+=(orows-irows-1)/2; orows-=(orows-irows-1)/2; }

		fim_coo_t ytimesw;

		if(SDL_MUSTLOCK(screen_))
		{
			if(SDL_LockSurface(screen_) < 0) return FIM_ERR_GENERIC;
		}

		fim_coo_t idr,idc,lor,loc;

		idr = iroff-oroff;
		idc = icoff-ocoff;

		lor = (FIM_MIN(orows-1,irows-1-iroff+oroff));
		loc = (FIM_MIN(ocols-1,icols-1-icoff+ocoff));

		fim_coo_t ii,ij;
		fim_coo_t oi,oj;
		fim_flags_t mirror=flags&FIM_FLAG_MIRROR, flip=flags&FIM_FLAG_FLIP;//STILL UNUSED : FIXME
		fim_byte_t * srcp;
		const int buginsdlorsomewhere=(icols > 16*1024 || irows > 16*1024); /* FIXME: occurring with SDL-1.2.15; to ascertain! (actually problems occur with an 21874x940 pixelmap ) */

		// FIXME : temporary
//		clear_rect(  ocoff, ocoff+ocols, oroff, oroff+orows); 
//		clear_rect(  0, ocols, 0, orows); 
		clear_rect(  0, width()-1, 0, height()-1); 


		if(!mirror && !flip && !buginsdlorsomewhere)
		{
#if 0
		for(oi=oroff;FIM_LIKELY(oi<lor);++oi)
		for(oj=ocoff;FIM_LIKELY(oj<loc);++oj)
		{
			ytimesw = (oi)*screen_->pitch/Bpp_;

			ii    = oi + idr;
			ij    = oj + idc;
			srcp  = ((fim_byte_t*)rgb)+(3*(ii*icskip+ij));

			setpixel(screen_, oj, ytimesw, (fim_coo_t)srcp[0], (fim_coo_t)srcp[1], (fim_coo_t)srcp[2]);
		}
#else
			FIM_CONSTEXPR Uint32 rmask=0x00000000,gmask=0x00000000,bmask=0x00000000,amask=0x00000000;
			if(SDL_Surface *src=SDL_CreateRGBSurfaceFrom(rgb,icols,irows,24,icols*3,rmask,gmask,bmask,amask))
			{
				SDL_Rect srcrect;
				SDL_Rect dstrect;
				srcrect.x=icoff;
				srcrect.y=iroff;
				srcrect.w=icols;
				srcrect.h=irows;
				dstrect.x=ocoff;
				dstrect.y=oroff;
				dstrect.w=ocols;
				dstrect.h=orows;
				SDL_UpperBlit(src,&srcrect,screen_,&dstrect);
				SDL_FreeSurface(src);
				/* FIXME: shall check error codes */
			}
			else
			{
				/* FIXME: need some error processing, here */
				return FIM_ERR_GENERIC;
			}
#endif
		}
		else
		for(oi=oroff;FIM_LIKELY(oi<lor);++oi)
		for(oj=ocoff;FIM_LIKELY(oj<loc);++oj)
		{

			ytimesw = (oi)*screen_->pitch/Bpp_;
			/* UNFINISHED : FIX ME */
			ii    = oi + idr;
			ij    = oj + idc;
			
			if(mirror)ij=((icols-1)-ij);
			if( flip )ii=((irows-1)-ii);
			srcp  = ((fim_byte_t*)rgb)+(3*(ii*icskip+ij));

			setpixel(screen_, oj, ytimesw, (fim_coo_t)srcp[0], (fim_coo_t)srcp[1], (fim_coo_t)srcp[2]);
		}

#if FIM_WANT_SDL_PROOF_OF_CONCEPT_MOUSE_SUPPORT
		if(fim_draw_help_map_ || fim_draw_help_map_tmp_)
			draw_help_map();
#endif /* FIM_WANT_SDL_PROOF_OF_CONCEPT_MOUSE_SUPPORT */
		if(SDL_MUSTLOCK(screen_)) SDL_UnlockSurface(screen_);

		SDL_Flip(screen_);

		return FIM_ERR_NO_ERROR;
	}

	bool SDLDevice::sdl_window_update(void)
	{
		vi_ = SDL_GetVideoInfo();
		if(!vi_)
			return false;
		current_w_=vi_->current_w;
		current_h_=vi_->current_h;
		bpp_      =vi_->vfmt->BitsPerPixel;
		Bpp_      =vi_->vfmt->BytesPerPixel;
		// FIXME: shall update want_windowed_ with effective flags contents
		return true;
	}

	fim_err_t SDLDevice::initialize(sym_keys_t &sym_keys)
	{
		/*
		 *
		 * */
		fim_coo_t want_width=current_w_, want_height=current_h_/*, want_bpp=0*/;
		fim_sdl_int want_flags=FIM_SDL_FLAGS;
		fim_sdl_int delay=0,interval=0;
		//want_flags|=SDL_NOFRAME;
		//std::cout << want_width << " : "<< want_height<<"\n";
#if 0
		//want_windowed_=true;
		want_height=480;
		want_width=480;
#endif
		if(want_windowed_)
			want_flags&=~SDL_FULLSCREEN;
#if FIM_SDL_WANT_RESIZE 
		if(want_resize_ && ! want_windowed_)
			want_flags|=SDL_RESIZABLE;
#endif /* FIM_SDL_WANT_RESIZE */
		//want_flags|=SDL_DOUBLEBUF;
		
		if(!allowed_resolution(want_width,want_height))
		{
			std::cout << "requested window size ("<<want_width<<":"<<want_height<<") smaller than the smallest allowed.\n";
			goto sdlerr;
		}

		if (SDL_Init(SDL_INIT_VIDEO) < 0 )
		{
			std::cout << "problems initializing SDL (SDL_Init)\n";
			goto sdlerr;
		}
		{
			if( const SDL_VideoInfo*bvip=SDL_GetVideoInfo() )
				bvi_=*bvip;
		}
		fim_perror(FIM_NULL);
		
		if(FIM_SDL_WANT_KEYREPEAT)
		{
		//	std::cout<<"interval:"<<interval<<"\n"; std::cout<<"delay :"<<delay <<"\n";
			delay=SDL_DEFAULT_REPEAT_DELAY;
			interval=SDL_DEFAULT_REPEAT_INTERVAL;
			if(SDL_EnableKeyRepeat(delay,interval)<0)
			{

			}
			else
			{
				SDL_GetKeyRepeat(&delay,&interval);
		//		std::cout<<"interval:"<<interval<<"\n"; std::cout<<"delay :"<<delay <<"\n";
			}
			fim_perror(FIM_NULL);
		}

		if(resize(want_width,want_height))
		{
			std::cout << "problems initializing SDL (SDL_SetVideoMode)\n";
			SDL_Quit();
			goto err;
		}
		fim_perror(FIM_NULL);

		/* Enable Unicode translation ( for a more flexible input handling ) */
	        SDL_EnableUNICODE( 1 );
		reset_wm_caption();
		fim_perror(FIM_NULL);

		sym_keys[FIM_KBD_PAGEUP]=SDLK_PAGEUP;
		sym_keys[FIM_KBD_PAGEDOWN]=SDLK_PAGEDOWN;
		sym_keys[FIM_KBD_LEFT]=SDLK_LEFT;
		sym_keys[FIM_KBD_RIGHT]=SDLK_RIGHT;
		sym_keys[FIM_KBD_UP]=SDLK_UP;
		sym_keys[FIM_KBD_DOWN]=SDLK_DOWN;
		sym_keys[FIM_KBD_SPACE]=SDLK_SPACE;
		sym_keys[FIM_KBD_END]=SDLK_END;
		sym_keys[FIM_KBD_HOME]=SDLK_HOME;
		sym_keys[FIM_KBD_BACKSPACE]=SDLK_BACKSPACE;
		sym_keys[FIM_KBD_BACKSPACE_]=SDLK_BACKSPACE;
		sym_keys[FIM_KBD_TAB]=SDLK_TAB;
		sym_keys[FIM_KBD_ENTER]=SDLK_RETURN;
		sym_keys[FIM_KBD_PAUSE]=SDLK_PAUSE;
		sym_keys[FIM_KBD_INS]=SDLK_INSERT;
		sym_keys[FIM_KBD_DEL]=SDLK_DELETE;
		sym_keys[FIM_KBD_MENU]=SDLK_MENU;
		sym_keys["F1" ]=SDLK_F1;
		sym_keys["F2" ]=SDLK_F2;
		sym_keys["F3" ]=SDLK_F3;
		sym_keys["F4" ]=SDLK_F4;
		sym_keys["F5" ]=SDLK_F5;
		sym_keys["F6" ]=SDLK_F6;
		sym_keys["F7" ]=SDLK_F7;
		sym_keys["F8" ]=SDLK_F8;
		sym_keys["F9" ]=SDLK_F9;
		sym_keys["F10"]=SDLK_F10;
		sym_keys["F11"]=SDLK_F11;
		sym_keys["F12"]=SDLK_F12;
		fim_perror(FIM_NULL);
		cc.key_syms_update();

		post_wmresize();
		return FIM_ERR_NO_ERROR;
sdlerr:
		if( const fim_char_t * const errstr = SDL_GetError() )
			std::cerr << "SDL error string: \"" << errstr  << "\"\n";
err:
		return FIM_ERR_GENERIC;
	}

	void SDLDevice::finalize(void)
	{
		finalized_=true;
		SDL_Quit();
	}

	fim_coo_t SDLDevice::get_chars_per_column(void)const
	{
		return height() / f_->sheight();
	}

	fim_coo_t SDLDevice::get_chars_per_line(void)const
	{
		return width() / f_->swidth();
	}

	fim_coo_t SDLDevice::width(void)const
	{
		return current_w_;
	}

	fim_coo_t SDLDevice::height(void)const
	{
		return current_h_;
	}

	inline void SDLDevice::setpixel(SDL_Surface *screen_, fim_coo_t x, fim_coo_t y, Uint8 r, Uint8 g, Uint8 b) FIM_NOEXCEPT
	{
		/*
		 * this function is taken straight from the sdl documentation: there are smarter ways to do this.
		 * */

		switch (bpp_)
		{
		case  8:
		{
			Uint8 *pixmem8;
			Uint8 colour;
			colour = SDL_MapRGB( screen_->format, b, g, r );
			pixmem8 = (Uint8*)((fim_byte_t*)( screen_->pixels)  + (y + x)*Bpp_);
			*pixmem8 = colour;
		}
		break;
		case 15:
		case 16:
		{
			Uint16 *pixmem16;
			Uint16 colour;
			colour = SDL_MapRGB( screen_->format, b, g, r );
			pixmem16 = (Uint16*)((fim_byte_t*)( screen_->pixels)  + (y + x)*Bpp_);
			*pixmem16 = colour;
		}
		break;
		case 24:
		{
			Uint32 *pixmem32;
			Uint32 colour;
			colour = SDL_MapRGB( screen_->format, b, g, r );
			pixmem32 = (Uint32*)((fim_byte_t*)( screen_->pixels)  + (y + x)*Bpp_);
			*pixmem32 = colour;
		}
		break;
		case 32:
		{
			Uint32 *pixmem32;
			Uint32 colour;
			colour = SDL_MapRGBA( screen_->format, b, g, r, 255 );
			pixmem32 = (Uint32*)((fim_byte_t*)( screen_->pixels)  + (y + x)*Bpp_);
			*pixmem32 = colour;
		}
		break;
		default:
		{
			/* 1,2,4 modes unsupported for NOW */
		}
		}

	}

	static fim_sys_int get_input_inner(fim_key_t * c, SDL_Event*eventp, fim_sys_int *keypressp, bool want_poll)
	{
//		fim_sys_int keypress_=0;
		bool ctrl_on=0;
		bool alt_on=0;
		//bool shift_on=0;
		fim_sys_int ret=0;
		SDL_Event event=*eventp;

		*c = FIM_SYM_NULL_KEY;

//		while(SDL_PollEvent(&event))
		if(want_poll)
			ret=SDL_PollEvent(&event);
		else
			ret=SDL_WaitEvent(&event);
		if(ret)
		{
			if(event.type==SDL_KEYUP)
				if(!SDL_PollEvent(&event))
					goto done;

			switch (event.type)
			{
#if FIM_SDL_WANT_RESIZE 
				case SDL_VIDEORESIZE:
						cc.resize(event.resize.w,event.resize.h);
				break;
#endif /* FIM_SDL_WANT_RESIZE */
				case SDL_QUIT:
#if FIM_SDL_ALLOW_QUIT
				*c=cc.find_keycode_for_bound_cmd(FIM_FLT_QUIT);
				return 1;
				//cc.quit();
#endif /* FIM_SDL_ALLOW_QUIT */
				*keypressp = 1;
				
				break;
				case SDL_KEYDOWN:

				if(event.key.keysym.mod == KMOD_RCTRL || event.key.keysym.mod == KMOD_LCTRL )
					ctrl_on=true;
				if(event.key.keysym.mod == KMOD_RALT  || event.key.keysym.mod == KMOD_LALT  ) 
					alt_on=true;
				if(event.key.keysym.mod == KMOD_RSHIFT  || event.key.keysym.mod == KMOD_LSHIFT  )
					;//shift_on=true;

			//	std::cout << "sym : " << event.key.keysym.sym << "\n" ;
			//	std::cout << "uni : " << event.key.keysym.unicode<< "\n" ;
			//	if(shift_on)std::cout << "shift_on\n";

				if( event.key.keysym.unicode == 0x0 )
				{
					/* arrows and stuff */
					FIM_SDL_INPUT_DEBUG(c,"no unicode");
					if(event.key.keysym.sym<256)
					{
						FIM_SDL_INPUT_DEBUG(c,"uhm");
						*c=event.key.keysym.sym;
						return 1;
					}
					else
					if(event.key.keysym.sym>=SDLK_F1 && event.key.keysym.sym<=SDLK_F12)
					{
						FIM_SDL_INPUT_DEBUG(c,"FXX?");
						*c=event.key.keysym.sym;
						return 1;
					}
					else
					if(
						event.key.keysym.sym!=SDLK_LSHIFT
					&&	event.key.keysym.sym!=SDLK_RSHIFT
					&&	event.key.keysym.sym!=SDLK_LALT
					&&	event.key.keysym.sym!=SDLK_RALT
					&&	event.key.keysym.sym!=SDLK_LCTRL
					&&	event.key.keysym.sym!=SDLK_RCTRL
					&&	event.key.keysym.sym!=SDLK_LSUPER
					&&	event.key.keysym.sym!=SDLK_RSUPER
					&&	event.key.keysym.sym!=SDLK_MENU	
					)
					{
						/* arrows.. .. */
						*c=event.key.keysym.sym;
						FIM_SDL_INPUT_DEBUG(c,"arrow");
						return 1;
					}
					else
					{
						if( event.key.keysym.sym==SDLK_LSHIFT )
							FIM_SDL_INPUT_DEBUG(c,"left shift");
						if( event.key.keysym.sym==SDLK_RSHIFT )
							FIM_SDL_INPUT_DEBUG(c,"right shift");
						if( event.key.keysym.sym==SDLK_MENU )
						{
							flip_draw_help_map_tmp(true);
							FIM_SDL_INPUT_DEBUG(c,"menu");
						}
						if( event.key.keysym.sym==SDLK_LSUPER )
							FIM_SDL_INPUT_DEBUG(c,"left win");
						if( event.key.keysym.sym==SDLK_RSUPER )
							FIM_SDL_INPUT_DEBUG(c,"right win");
						if( event.key.keysym.sym==SDLK_RALT )
							FIM_SDL_INPUT_DEBUG(c,"right alt");
						if( event.key.keysym.sym==SDLK_LALT )
							FIM_SDL_INPUT_DEBUG(c,"left alt");
						if( event.key.keysym.sym==SDLK_RCTRL )
							FIM_SDL_INPUT_DEBUG(c,"right control");
						if( event.key.keysym.sym==SDLK_LCTRL )
							FIM_SDL_INPUT_DEBUG(c,"left control");
						/* we ignore lone shift or alt .. */
						return 0;
					}
				}

				if(alt_on)
				{
					*c=(fim_byte_t)event.key.keysym.unicode;
					*c|=(1<<31);	/* FIXME : a dirty trick */
					return 1;
				}

				if( 	event.key.keysym.unicode < 0x80)
				{
					/* 
					 * SDL documentation 1.2.12 about unicode:
					 * It is useful to note that unicode values < 0x80 translate directly
					 * a characters ASCII value.
					 * */
			//		if(event.key.keysym.mod == KMOD_RCTRL || event.key.keysym.mod == KMOD_LCTRL ) std::cout << "ctrl ! \n";
					*c=(fim_byte_t)event.key.keysym.unicode;

					if(ctrl_on)
					{
						// if(*c-1+'a'=='c')std::exit(-1);//works 
#if 0
						if(*c-1+'a'<='z')
						{
							//std::cout << "with control : " << *c+'a'-1 << "\n";
						}
						else
						{
							/* other chars */
							*c-='a';
							*c+= 1 ;
						}
#else
#endif
						FIM_SDL_INPUT_DEBUG(c,"ctrl is on");
					}
					if(*c)	/* !iscntrl(c) */
					{
						/* the usual chars */
						FIM_SDL_INPUT_DEBUG(c,"keysym");
						return 1;
					}
					else	/*  iscntrl(c) */
					{
						FIM_SDL_INPUT_DEBUG(c,"iscntrl");
						return 0;
					}
					/*
					 * p.s.: note that we get 0 in some cases (e.g.: KMOD_RSHIFT, ...).
					 * */
				}
				else
				{
					FIM_SDL_INPUT_DEBUG(c,"wchar");
					cout << "sorry, no support for wide chars in fim\n";
					/*  no support for wide chars in fim */
					return 0;
				}
				FIM_SDL_INPUT_DEBUG(c,"unknown");
				return 0;

				break;
#if FIM_WANT_SDL_PROOF_OF_CONCEPT_MOUSE_SUPPORT
				//case SDL_MOUSEMOTION:
				case SDL_MOUSEBUTTONDOWN:
				//case SDL_MOUSEBUTTONUP:
				if(fim::string wmc = cc.getStringVariable(FIM_VID_WANT_MOUSE_CTRL))
				if( wmc.size()>=9 && strncpy(key_char_grid,wmc.c_str(),9) )
				{
					fim_coo_t x,y;
					Uint8 ms=SDL_GetMouseState(&x,&y);
#if FIM_WANT_SDL_CLICK_MOUSE_SUPPORT
					Viewport* cv = cc.current_viewport();
					fim_coo_t xt = cv->viewport_width()/3;
					fim_coo_t yt = cv->viewport_height()/3;

					if(!cc.inConsole() && ms&SDL_BUTTON_LMASK)
					{
					if( x < xt )
					{
						if( y < yt )
						{
							*c=key_char_grid[0]; return 1;
						}
						else
						if( y < 2*yt )
						{
							*c=key_char_grid[1]; return 1;
						}
						else
						{
							*c=key_char_grid[2]; return 1;
						}
					}
					else
					if( x < 2*xt )
					{
						if( y < yt )
						{
							*c=key_char_grid[3]; return 1;
						}
						else
						if( y < 2*yt )
						{
							*c=key_char_grid[4]; return 1;
						}
						else
						{
							*c=key_char_grid[5]; return 1;
						}
					}
					else
					{
						if( y < yt )
						{
							*c=key_char_grid[6]; return 1;
						}
						else
						if( y < 2*yt )
						{
							*c=key_char_grid[7]; return 1;
						}
						else
						{
							*c=key_char_grid[8]; return 1;
						}
					}
					}
#endif /* FIM_WANT_SDL_CLICK_MOUSE_SUPPORT */
					//cout << "mouse clicked at "<<x<<" "<<y<<" : "<< ((x>cv->viewport_width()/2)?'r':'l') <<"; state: "<<ms<<"\n";
#if 0
					if(ms&SDL_BUTTON_RMASK) cout << "rmask\n";
					if(ms&SDL_BUTTON_LMASK) cout << "lmask\n";
					if(ms&SDL_BUTTON_MMASK) cout << "mmask\n";
					if(ms&SDL_BUTTON_X1MASK) cout << "x1mask\n";
					if(ms&SDL_BUTTON_X2MASK) cout << "x2mask\n";
#endif /* 1 */
					if(!cc.inConsole())
					{
						if(ms&SDL_BUTTON_LMASK) { *c='n'; return 1; }
						if(ms&SDL_BUTTON_RMASK) { toggle_draw_help_map(); return 0; }
						if(ms&SDL_BUTTON_MMASK) { toggle_draw_help_map(); return 0; }
						//if(ms&SDL_BUTTON_RMASK) { *c='b'; return 1; }
						//if(ms&SDL_BUTTON_MMASK) { *c='q'; return 1; }
						if(ms&SDL_BUTTON_X1MASK	) { *c='+'; return 1; }
						if(ms&SDL_BUTTON_X2MASK	) { *c='-'; return 1; }
						if(ms&SDL_BUTTON(SDL_BUTTON_WHEELUP)) { *c='+'; return 1; }
						if(ms&SDL_BUTTON(SDL_BUTTON_WHEELDOWN)) { *c='-'; return 1; }
					}
				}
				break;
#endif /* FIM_WANT_SDL_PROOF_OF_CONCEPT_MOUSE_SUPPORT */
				case SDL_KEYUP:
				return 0;
				break;
				case SDL_MOUSEMOTION:
#if FIM_WANT_MOUSE_PAN
				FIM_SDL_INPUT_DEBUG(c,"SDL_MOUSEMOTION");
				{
					//std::cout << current_w_    << " " << event.motion.y    << "\n";
					//std::cout << event.motion.x    << " " << event.motion.y    << "\n";
					//std::cout << event.motion.xrel << " " << event.motion.yrel << "\n";
					Viewport* cv = cc.current_viewport();
					static bool discardedfirst = false;
					if(cv)
					if(discardedfirst)
					if(const Image* ci = cv->c_getImage())
					if( event.motion.x > 0 && event.motion.y )
					{
						// 1/(2*bf) of each screen side will be insensitive to mouse movement.
						fim_off_t bf = 5;
						fim_off_t vx = cv->viewport_width();
						fim_off_t vy = cv->viewport_height();
						fim_off_t bx = vx / bf;
						fim_off_t by = vy / bf;
						fim_coo_t riw = ci->width();
						fim_coo_t rih = ci->height();

#if FIM_WANT_POSITION_DISPLAYED
						gx = (riw<vx) ? -1 : event.motion.x;
						gy = (rih<vy) ? -1 : event.motion.y;
#endif /* FIM_WANT_POSITION_DISPLAYED */

						if(FIM_WANT_VARIABLE_RESOLUTION_MOUSE_SCROLL)
						{
							// variable resolution mouse scroll (maximal smoothess)
							// max of xres/yres discrete scrolls.
							fim_off_t xres = FIM_MIN(vx-bx,riw), yres = FIM_MIN(vy-by,rih);
							double px = FIM_DELIMIT_TO_X(FIM_INT_DET_PX(event.motion.x-bx/2,vx-bx,xres),xres);
							double py = FIM_DELIMIT_TO_X(FIM_INT_DET_PX(event.motion.y-by/2,vy-by,yres),yres);
							px*=100.0/xres;
							py*=100.0/yres;
							cv->pan_to(px,py);
						}
						else
						{
							// percentage scroll.
							// downside: if image very wide/tall/big, will
							// steps might be very large.
							fim_off_t px = FIM_DELIMIT_TO_100(FIM_INT_DET_PCNT(event.motion.x-bx/2,vx-bx));
							fim_off_t py = FIM_DELIMIT_TO_100(FIM_INT_DET_PCNT(event.motion.y-by/2,vy-by));
							//std::cout << "pct:"<< px << " " << py << "\n";
							cv->pan_to(px,py);
						}
						cv->display(); // draw only if necessary
						//cv->redisplay(); // draw always
					}
					discardedfirst=true;
				}
#endif /* FIM_WANT_MOUSE_PAN */
				break;
				default:
				FIM_SDL_INPUT_DEBUG(c,"default-unknown");
			}
			return 0;
		}
done:
		FIM_SDL_INPUT_DEBUG(c,"no key");
		flip_draw_help_map_tmp(false);
		return 0;
	}

	fim_sys_int SDLDevice::get_input(fim_key_t * c, bool want_poll)
	{
		fim_sys_int iv = get_input_inner(c,&event_,&keypress_,want_poll);
		return iv;
	}

	fim_err_t SDLDevice::fill_rect(fim_coo_t x1, fim_coo_t x2, fim_coo_t y1,fim_coo_t y2, fim_color_t color) FIM_NOEXCEPT
	{
		fim_coo_t y;
		/*
		 * This could be optimized
		 * */
		for(y=y1;y<=y2;++y)
		{
			fim_memset(((fim_byte_t*)(screen_->pixels)) + y*screen_->pitch + x1*Bpp_,color, (x2-x1+1)* Bpp_);
		}
		return FIM_ERR_NO_ERROR;
	}

	fim_err_t SDLDevice::fill_rect(fim_coo_t x1, fim_coo_t x2, fim_coo_t y1,fim_coo_t y2, fim_color_t icolor, fim_color_t ocolor) FIM_NOEXCEPT
	{
		/*
		 * This could be optimized.
		 * e.g. four lines around inner box would be better.
		 * */
		fim_err_t rv = FIM_ERR_NO_ERROR;
		rv = fill_rect(x1, x2, y1, y2, icolor);
		if(x2-x1<3 || y2-y1<3)
			rv = fill_rect(x1+1, x2-1, y1+1, y2-1, ocolor);
		return rv;
	}

	fim_err_t SDLDevice::clear_rect(fim_coo_t x1, fim_coo_t x2, fim_coo_t y1,fim_coo_t y2) FIM_NOEXCEPT
	{
		fim_coo_t y;
		/*
		 * This could be optimized
		 * */
		for(y=y1;y<=y2;++y)
		{
			fim_bzero(((fim_byte_t*)(screen_->pixels)) + y*screen_->pitch + x1*Bpp_, (x2-x1+1)* Bpp_);
		}
		return FIM_ERR_NO_ERROR;
	}

void SDLDevice::fs_render_fb(fim_coo_t x_, fim_coo_t y, FSXCharInfo *charInfo, fim_byte_t *data) FIM_NOEXCEPT
{

/* 
 * These preprocessor macros should serve *only* for font handling purposes.
 * */
#define BIT_ORDER       BitmapFormatBitOrderMSB
#ifdef BYTE_ORDER
#undef BYTE_ORDER
#endif
#define BYTE_ORDER      BitmapFormatByteOrderMSB
#define SCANLINE_UNIT   BitmapFormatScanlineUnit8
#define SCANLINE_PAD    BitmapFormatScanlinePad8
#define EXTENTS         BitmapFormatImageRectMin

#define SCANLINE_PAD_BYTES 1
#define GLWIDTHBYTESPADDED(bits, nBytes)                                    \
        ((nBytes) == 1 ? (((bits)  +  7) >> 3)          /* pad to 1 byte  */\
        :(nBytes) == 2 ? ((((bits) + 15) >> 3) & ~1)    /* pad to 2 bytes */\
        :(nBytes) == 4 ? ((((bits) + 31) >> 3) & ~3)    /* pad to 4 bytes */\
        :(nBytes) == 8 ? ((((bits) + 63) >> 3) & ~7)    /* pad to 8 bytes */\
        : 0)

	fim_coo_t row,bit,x;
	fim_sys_int bpr;
	FIM_CONSTEXPR Uint8 rc = 0xff, gc = 0xff, bc = 0xff;
	// const Uint8 rc = 0x00, gc = 0x00, bc = 0xff;

	bpr = GLWIDTHBYTESPADDED((charInfo->right - charInfo->left), SCANLINE_PAD_BYTES);
	for (row = 0; row < (charInfo->ascent + charInfo->descent); row++)
	{
		for (x = 0, bit = 0; bit < (charInfo->right - charInfo->left); bit++) 
		{
			if (data[bit>>3] & fs_masktab[bit&7])
			{	// WARNING !
#if FIM_FONT_MAGNIFY_FACTOR == 0
				const fim_int fim_fmf = fim::fim_fmf_; 
#endif	/* FIM_FONT_MAGNIFY_FACTOR */
#if FIM_FONT_MAGNIFY_FACTOR <  0
				fim_int fim_fmf = fim::fim_fmf_; 
#endif	/* FIM_FONT_MAGNIFY_FACTOR */
#if FIM_FONT_MAGNIFY_FACTOR == 1
				setpixel(screen_,x_+x,(y+row)*screen_->pitch/Bpp_,rc,gc,bc);
#else	/* FIM_FONT_MAGNIFY_FACTOR */
				for(fim_coo_t mi = 0; mi < fim_fmf; ++mi)
				for(fim_coo_t mj = 0; mj < fim_fmf; ++mj)
					setpixel(screen_,x_+fim_fmf*x+mj,(y+fim_fmf*row+mi)*screen_->pitch/Bpp_,rc,gc,bc);
#endif	/* FIM_FONT_MAGNIFY_FACTOR */
			}
			x += Bpp_/Bpp_;/* FIXME */
		}
		data += bpr;
	}

#undef BIT_ORDER
#undef BYTE_ORDER
#undef SCANLINE_UNIT
#undef SCANLINE_PAD
#undef EXTENTS
#undef SCANLINE_PAD_BYTES
#undef GLWIDTHBYTESPADDED
}

fim_err_t SDLDevice::fs_puts(struct fs_font *f_, fim_coo_t x, fim_coo_t y, const fim_char_t *str) FIM_NOEXCEPT
{
    fim_sys_int i,c/*,j,w*/;

    for (i = 0; str[i] != '\0'; i++) {
	c = (fim_byte_t)str[i];
	if (FIM_NULL == f_->eindex[c])
	    continue;
	/* clear with bg color */
//	w = (f_->eindex[c]->width+1)*Bpp_;
#if 0
#ifdef FIM_IS_SLOWER_THAN_FBI
	for (j = 0; j < f_->sheight(); j++) {
/////	    memset_combine(start,0x20,w);
	    fim_bzero(start,w);
	    start += fb_fix.line_length;
	}
#else
	//sometimes we can gather multiple calls..
	if(fb_fix.line_length==w)
	{
		//contiguous case
		fim_bzero(start,w*f_->sheight());
	    	start += fb_fix.line_length*f_->sheight();
	}
	else
	for (j = 0; j < f_->sheight(); j++) {
	    fim_bzero(start,w);
	    start += fb_fix.line_length;
	}
#endif
#endif
	/* draw character */
	//fs_render_fb(fb_fix.line_length,f_->eindex[c],f_->gindex[c]);
	fs_render_fb(x,y,f_->eindex[c],f_->gindex[c]);
	x += f_->eindex[c]->swidth();
	/* FIXME : SLOW ! */
	if (((fim_coo_t)x) > width() - f_->swidth())
		goto err;
    }
    // FIXME
	return FIM_ERR_NO_ERROR;
err:
	return FIM_ERR_GENERIC;
}

	fim_err_t SDLDevice::status_line(const fim_char_t *msg)
	{
		fim_err_t errval = FIM_ERR_NO_ERROR;
		fim_coo_t y,ys=3;// FIXME

		if(SDL_MUSTLOCK(screen_) && SDL_LockSurface(screen_) < 0)
		{
			 errval = FIM_ERR_GENERIC;
			 goto done;
		}

		if(get_chars_per_column()<1)
			goto done;
		y = height() - f_->sheight() - ys;
		if(y<0 )
			goto done;
		clear_rect(0, width()-1, y+1,y+f_->sheight()+ys-1);
		fs_puts(f_, 0, y+ys, msg);
		fill_rect(0,width()-1, y, y, FIM_CNS_WHITE);

		if(SDL_MUSTLOCK(screen_)) SDL_UnlockSurface(screen_);
		SDL_Flip(screen_);
done:
		return errval;
	}

	fim_key_t SDLDevice::catchInteractiveCommand(fim_ts_t seconds)const
	{
		/*
		 * Whether there is some input in the input queue.
		 * Note that in this way the event_ will be lost.
		 * */
		fim_key_t c=0;
		SDL_Event levent;
		fim_sys_int lkeypress=0;
		fim_tms_t sms=10,ms=seconds*1000;// sms: sleep ms
#if 0
		for(;seconds>0;--seconds)
			sleep(1);
		if(!get_input_inner(&c,&levent,&lkeypress))
		{
			//std::cout << "input:" << c << "\n";
			return c;
		}
		else
			return -1;
#else
		do
		{
			if(ms>0)
				usleep((useconds_t)(sms*1000)),ms-=sms;
			// we read input twice: it seems we have a number of "spurious" inputs. 
			if(1==get_input_inner(&c,&levent,&lkeypress,true)) goto done;
		}
		while(ms>0);
		return -1;
done:
		usleep((useconds_t)(sms*1000)),ms-=sms;
		return c;
#endif
	}

	void SDLDevice::flush(void)
	{
	}

	void SDLDevice::lock(void)
	{
		if(SDL_MUSTLOCK(screen_))
		{
			if(SDL_LockSurface(screen_) < 0) return;
		}
	}

	void SDLDevice::unlock(void)
	{
		if(SDL_MUSTLOCK(screen_)) SDL_UnlockSurface(screen_);
		SDL_Flip(screen_);
	}

	bool SDLDevice::allowed_resolution(fim_coo_t w, fim_coo_t h)
	{
		if(w==0 || h==0)
			goto ok;
		if(w<FIM_SDL_MINWIDTH || h<FIM_SDL_MINHEIGHT)
			return false;
		if(w<f_->swidth() || h<f_->sheight())
			return false;
ok:
		return true;
	}

	fim_err_t SDLDevice::resize(fim_coo_t w, fim_coo_t h)
	{
		SDL_Surface *nscreen_=FIM_NULL;
		fim_sdl_int want_flags=screen_?screen_->flags:FIM_SDL_FLAGS;
#if FIM_WANT_HARDCODED_ICON
		unsigned char icondata[] =
#include "default_icon_byte_array.h"
		SDL_Surface *icon = FIM_NULL;
#endif /* FIM_WANT_HARDCODED_ICON */

		if(want_resize_)
			want_flags|=SDL_RESIZABLE;
		else
			want_flags&=~SDL_RESIZABLE;

		if(want_windowed_)
			want_flags&=~SDL_FULLSCREEN;
		else
			want_flags|=SDL_FULLSCREEN;

		if(!allowed_resolution(w,h))
			return FIM_ERR_GENERIC;

		SDL_ShowCursor(want_mouse_display_?1:0);

		if(w==0 && h==0)
		{
			int nr = 1, dr = 2;

			if( want_flags & SDL_FULLSCREEN )
				nr = 1, dr = 1;

			w = FIM_FRAC(bvi_.current_w,nr,dr);
		       	h = FIM_FRAC(bvi_.current_h,nr,dr);
		}
		w = FIM_MIN(w,bvi_.current_w);
		h = FIM_MIN(h,bvi_.current_h);
		//std::cout << "using " << bvi_.current_w << " "<<  bvi_.current_h << " !\n";

#if FIM_WANT_HARDCODED_ICON
		icon = SDL_LoadBMP_RW(SDL_RWFromMem(icondata, sizeof(icondata)), 1);
		SDL_WM_SetIcon(icon, FIM_NULL);
		SDL_FreeSurface(icon);
#endif /* FIM_WANT_HARDCODED_ICON */

		//std::cout << "resizing to " << w << " "<< h << "\n";
		if (FIM_NULL==(nscreen_ = SDL_SetVideoMode(w, h, bpp_, want_flags)))
		{
			///std::cout << "resizing to " << w << " "<< h << " FAILED!\n";
			return FIM_ERR_GENERIC;
		}

		screen_=nscreen_;
		if(want_flags&SDL_FULLSCREEN)
			reset_wm_caption();
		if(!sdl_window_update())
		{
			std::cout << "problems initializing SDL (SDL_GetVideoInfo)\n";
			return FIM_ERR_GENERIC;
		}

		post_wmresize();
		return FIM_ERR_NO_ERROR;
	}

	fim_err_t SDLDevice::reinit(const fim_char_t *rs)
	{
#if FIM_SDL_WANT_RESIZE 
#else
		cout << "reinit not allowed\n";
		goto err;
#endif
		// FIXME: a wrong command string shall be ignored!
		if(parse_optstring(rs)!=FIM_ERR_NO_ERROR)
			goto err;
		return cc.resize(current_w_,current_h_);
	err:
		//std::cerr<<"problems!\n";
		return FIM_ERR_GENERIC;
	}

	fim_err_t SDLDevice::set_wm_caption(const fim_char_t *msg)
	{
		fim_err_t rc=FIM_ERR_NO_ERROR;
#if FIM_WANT_CAPTION_CONTROL
		if((!msg) || (!want_windowed_))
		{
		       	rc=FIM_ERR_UNSUPPORTED;
		       	goto err;
		}
		SDL_WM_SetCaption(msg,FIM_SDL_ICONPATH);
#else
		rc=FIM_ERR_UNSUPPORTED;
#endif
err:
		return rc;
	}
	
	fim_err_t SDLDevice::reset_wm_caption(void)
	{
		SDL_WM_SetCaption(FIM_CNS_FIM_APPTITLE,FIM_SDL_ICONPATH);
		return FIM_ERR_NO_ERROR;
	}

	fim_coo_t SDLDevice::status_line_height(void)const
	{
		return f_ ? border_height_ + f_->sheight() : 0;
	}

	fim_err_t SDLDevice::post_wmresize(void)
	{
		cc.setVariable(FIM_VID_SCREEN_WIDTH, current_w_);
		cc.setVariable(FIM_VID_SCREEN_HEIGHT,current_h_);
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
		// textual console reformatting
		//mc_.setRows ( get_chars_per_column()/(2*f_->sheight()) );
		mc_.setGlobalVariable(FIM_VID_CONSOLE_ROWS,(height()/(2*f_->sheight())));
		mc_.reformat(    width() /    f_->swidth()   );
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */
		return FIM_ERR_NO_ERROR;
	}
#endif


