/* $LastChangedDate: 2017-08-19 16:24:46 +0200 (Sat, 19 Aug 2017) $ */
/*
 CACADevice.cpp : cacalib device Fim driver file

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

/*  20080504 this CACA driver doesn't work yet */

#include "fim.h"

#ifdef FIM_WITH_CACALIB

#include "CACADevice.h"

#define min(x,y) ((x)<(y)?(x):(y))

	int CACADevice::clear_rect_(
		void* dst,	// destination gray array and source rgb array
		int oroff,int ocoff,	// row  and column  offset of the first output pixel
		int orows,int ocols,	// rows and columns drawable in the output buffer
		int ocskip		// output columns to skip for each line
	)
	{
		/* output screen variables */
		int 
			oi,// output image row index
			oj;// output image columns index

		int gray;
		int idr,idc,lor,loc;
    		
		if( oroff <0 ) return -8;
		if( ocoff <0 ) return -9;
		if( orows <=0 ) return -10;
		if( ocols <=0 ) return -11;
		if( ocskip<0 ) return -12;

		if( oroff>orows ) return -8-10*100;
		if( ocoff>ocols ) return -9-11*100;

		if( ocskip<ocols ) return -12-11*100;

		/*
		 * orows and ocols is the total number of rows and columns in the output window.
		 * no more than orows-oroff rows and ocols-ocoff columns will be rendered, however
		 * */

		lor = orows-1;
		loc = ocols-1;

/*		cout << iroff << " " << icoff << " " << irows << " " << icols << " " << icskip << "\n";
		cout << oroff << " " << ocoff << " " << orows << " " << ocols << " " << ocskip << "\n";
		cout << idr << " " << idc << " " << "\n";
		cout << loc << " " << lor << " " << "\n";*/

		/* TODO : unroll me :) */
		for(oi=oroff;oi<lor;++oi)
		for(oj=ocoff;oj<loc;++oj)
		{
			((char*)(dst))[oi*ocskip+oj]=0;
		}
		return  0;
		
	}


	int _matrix_copy_rgb_to_gray(
		void* dst, void* src,	// destination gray array and source rgb array
		int iroff,int icoff,	// row  and column  offset of the first input pixel
		int irows,int icols,	// rows and columns in the input image
		int icskip,		// input columns to skip for each line
		int oroff,int ocoff,	// row  and column  offset of the first output pixel
		int orows,int ocols,	// rows and columns drawable in the output buffer
		int ocskip,		// output columns to skip for each line
		int flags		// some flags :flag values in fim.h : FIXME
	)
	{
		/*
		 * This routine will be optimized, some day.
		 * Note that it is not a member function.
		 * TODO : optimize and generalize, in all possible ways:
		 * 	RGBTOGRAY
		 * 	RGBTOGRAYGRAYGRAY
		 * 	...
		 *
		 * This routine copies the pixelmap starting at (iroff,icoff) in the input image
		 * to the output buffer, starting at (oroff,ocoff).
		 *
		 * The last source pixel copied will be the one :
		 * 	(min(irows-1,orows-1-oroff+iroff),min(icols-1,ocols-1-ocoff+icoff))
		 * in the input buffer and will be written to:
		 * 	(min(orows-1,irows-1-iroff+oroff),min(ocols-1,icols-1-icoff+ocoff))
		 * in the output buffer.
		 *
		 * It assumes that both pixelmaps are stored in row major order, with row strides
		 * of icskip (columns in the  input matrix between rows) for the  input array src,
		 * of ocskip (columns in the output matrix between rows) for the output array dst.
		 *
		 * The routine needs to know the  input image rows (irows), columns (icols).
		 * The routine needs to know the output image rows (orows), columns (ocols).
		 *
		 * It assumes the input pixelmap having three bytes for pixel, and the puts in the 
		 * output buffer the corresponding single byte gray values.
		 * */

		int
			ii,// output image row index
			ij;// output image columns index

		/* output screen variables */
		int 
			oi,// output image row index
			oj;// output image columns index

		int gray;
		char *srcp;
		int idr,idc,lor,loc;
    		
		int mirror=flags&FIM_FLAG_MIRROR, flip=flags&FIM_FLAG_FLIP;//STILL UNUSED : FIXME

		if ( !src ) return FIM_ERR_GENERIC;
	
		if( iroff <0 ) return -3;
		if( icoff <0 ) return -4;
		if( irows <=0 ) return -5;
		if( icols <=0 ) return -6;
		if( icskip<0 ) return -7;
		if( oroff <0 ) return -8;
		if( ocoff <0 ) return -9;
		if( orows <=0 ) return -10;
		if( ocols <=0 ) return -11;
		if( ocskip<0 ) return -12;
		if( flags <0 ) return -13;

		if( iroff>irows ) return -3-4*100 ;
		if( icoff>icols ) return -4-6*100;
		if( oroff>orows ) return -8-10*100;
		if( ocoff>ocols ) return -9-11*100;

		if( icskip<icols ) return -7-6*100;
		if( ocskip<ocols ) return -12-11*100;

		/*
		 * orows and ocols is the total number of rows and columns in the output window.
		 * no more than orows-oroff rows and ocols-ocoff columns will be rendered, however
		 * */

//		dr=(min(irows-1,orows-1-oroff+iroff))-(min(orows-1,irows-1-iroff+oroff));
//		dc=(min(icols-1,ocols-1-ocoff+icoff))-(min(ocols-1,icols-1-icoff+ocoff));

		idr = iroff-oroff;
		idc = icoff-ocoff;

		lor = (min(orows-1,irows-1-iroff+oroff));
		loc = (min(ocols-1,icols-1-icoff+ocoff));

/*		cout << iroff << " " << icoff << " " << irows << " " << icols << " " << icskip << "\n";
		cout << oroff << " " << ocoff << " " << orows << " " << ocols << " " << ocskip << "\n";
		cout << idr << " " << idc << " " << "\n";
		cout << loc << " " << lor << " " << "\n";*/

		/* TODO : unroll me and optimize me :) */
		if(!mirror && !flip)
		for(oi=oroff;FIM_LIKELY(oi<lor);++oi)
		for(oj=ocoff;FIM_LIKELY(oj<loc);++oj)
		{
			ii    = oi + idr;
			ij    = oj + idc;
			srcp  = ((char*)src)+(3*(ii*icskip+ij));
			gray  = (int)srcp[0];
			gray += (int)srcp[1];
			gray += (int)srcp[2];
			((char*)(dst))[oi*ocskip+oj]=(char)(gray/3);
		}
		else
		for(oi=oroff;FIM_LIKELY(oi<lor);++oi)
		for(oj=ocoff;FIM_LIKELY(oj<loc);++oj)
		{
			/*
			 * FIXME : these expressions () are correct, but some garbage is printed in flip mode!
			 * therefore instead of lor-oi+1 we use lor-oi !!
			 * DANGER
			 * */
			/*if(flip)  ii    = (lor-oi) + idr;
			else      ii    = oi + idr;

			if(mirror)ij    = (loc-oj) + idc;
			else      ij    = oj + idc;
			if(ij<0)return FIM_ERR_GENERIC;*/

			ii    = oi + idr;
			ij    = oj + idc;
			
			if(mirror)ij=((icols-1)-ij);
			if( flip )ii=((irows-1)-ii);
			srcp  = ((char*)src)+(3*(ii*icskip+ij));
			if(mirror)ij=((icols-1)-ij);
			if( flip )ii=((irows-1)-ii);

			gray  = (int)srcp[0];
			gray += (int)srcp[1];
			gray += (int)srcp[2];
			((char*)(dst))[oi*ocskip+oj]=(char)(gray/3);
		}

		return  0;
	}

//#define width() aa_imgwidth(ascii_context)
//#define height() aa_imgheight(ascii_context)

//#define width() aa_scrwidth(ascii_context)
//#define height() aa_scrheight(ascii_context)

	fim_err_t CACADevice::display(
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
		 * shareable with FramebufferDevice would be nice, if implemented in CACADevice.
		 * */
		void* rgb = ida_image_img?((const struct ida_image*)ida_image_img)->data:FIM_NULL;// source rgb array
		if ( !rgb ) return FIM_ERR_GENERIC;
	
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
	
		orows  = min( orows, height());
		ocols  = min( ocols,  width()); 
		ocskip = width(); 	//FIXME maybe this is not enough and should be commented or rewritten!

		if( orows  > height() ) return -9 -99*100;
		if( ocols  >  width() ) return -10-99*100;
		if( ocskip <  width() ) return -11-99*100;
		if( icskip<icols ) return -6-5*100;

//		I must still decide on the destiny of the following
//		orows  = min( irows-iroff, height());
//		ocols  = min( icols-icoff,  width());// rows and columns to draw in output buffer
//		ocskip = width();// output columns to skip for each line
		
		caca_set_window_title("caca-fim");
		caca_clear();
		caca_refresh();

		caca_draw_bitmap(0, 0, caca_get_width() - 1, caca_get_height() - 1, caca_bitmap, bitmap);
		caca_draw_bitmap(0, 0, caca_get_width() - 1, caca_get_height() - 1, caca_bitmap, rgb);
		caca_putstr(0,0,"Unfinished display interface!");
		caca_printf(0,txt_height()-1,"%s","sample caca msg");
		caca_refresh();

		ocskip = width();// output columns to skip for each line
		ocskip = width();// output columns to skip for each line

		return FIM_ERR_NO_ERROR;
	}

	fim_err_t CACADevice::initialize(sym_keys_t &sym_keys)
	{
		int rc=0;

		rc = caca_init();
		if(rc)return rc;

		XSIZ = caca_get_width() * 2;
		YSIZ = caca_get_height() * 2 - 4;

		caca_bitmap = caca_create_bitmap(8, XSIZ, YSIZ - 2, XSIZ, 0, 0, 0, 0);
		if( !caca_bitmap ) return FIM_ERR_GENERIC;
		caca_set_bitmap_palette(caca_bitmap, r, g, b, a);
		bitmap = (char*)malloc(4 * caca_get_width() * caca_get_height() * sizeof(char));
		if(!bitmap) return FIM_ERR_GENERIC;
		fim_bzero(bitmap, 4 * caca_get_width() * caca_get_height());

		caca_clear();
		caca_set_color(CACA_COLOR_BLACK,CACA_COLOR_WHITE);
		caca_set_color(CACA_COLOR_RED,CACA_COLOR_BLACK);
		caca_putstr(0,0,"What a caca!");
		caca_refresh();
		return rc?FIM_ERR_GENERIC:FIM_ERR_NO_ERROR;
	}

	void CACADevice::finalize(void)
	{
		caca_end();
	}

	fim_coo_t CACADevice::get_chars_per_line(void)const{return txt_width();}
	int CACADevice::txt_width(void)const { return width() ;}
	int CACADevice::txt_height(void)const{ return width() ;}
	int CACADevice::width(void)const { return caca_get_height();}
	int CACADevice::height(void)const{ return caca_get_width() ;}
	fim_err_t CACADevice::status_line(const fim_char_t *msg)
	{
		caca_printf(0,txt_height()-1,"%s",msg);
		caca_printf(0,0,"foooooooo");
		caca_putstr(0,0,"foooooooo");
		return FIM_ERR_NO_ERROR;
	}
	fim_err_t CACADevice::fs_puts(struct fs_font *f, fim_coo_t x, fim_coo_t y, const fim_char_t *str){return FIM_ERR_NO_ERROR;}
	fim_err_t CACADevice::clear_rect(fim_coo_t x1, fim_coo_t x2, fim_coo_t y1,fim_coo_t y2)
	{
		/* FIXME : only if initialized !*/
		return FIM_ERR_GENERIC;
	}
	fim_bool_t CACADevice::handle_console_switch(void){return true;}
	fim_err_t CACADevice::console_control(fim_cc_t code){return FIM_ERR_GENERIC;}
/*
 * This is embryo code and should be used for experimental purposes only!
 */
	fim_coo_t CACADevice::get_chars_per_column(void)const{return height();}
	fim_coo_t CACADevice::font_height(void)const
	{
		return FIM_CACALIB_FONT_HEIGHT;
	}

	fim_coo_t CACADevice::status_line_height(void)const
	{
		return FIM_CACALIB_FONT_HEIGHT;
	}

	fim_sys_int CACADevice::get_input(fim_key_t * c, bool want_poll)
	{
		/* FIXME: better make this virtual pure before writing the next Device ..  */
		int ce = caca_wait_event(CACA_EVENT_KEY_PRESS);
		//caca_get_event(...);
		return ce;
	}
#endif /* FIM_WITH_CACALIB */
