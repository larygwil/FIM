/* $LastChangedDate: 2017-08-19 11:38:33 +0200 (Sat, 19 Aug 2017) $ */
/*
 CACADevice.h : cacalib device Fim driver header file

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
#ifndef FIM_CACADEVICE_H
#define FIM_CACADEVICE_H
#ifdef FIM_WITH_CACALIB

#include "DisplayDevice.h"
#include <caca0.h>

#define FIM_CACALIB_FONT_HEIGHT 1
/*  20080504 this CACA driver doesn't work yet */
class CACADevice FIM_FINAL:public DisplayDevice 
{
	private:
	unsigned int r[256], g[256], b[256], a[256];

	int XSIZ, YSIZ;
	struct caca_bitmap *caca_bitmap;
	fim_char_t *bitmap;

	public:
#ifndef FIM_WANT_NO_OUTPUT_CONSOLE
	CACADevice(MiniConsole& mc_):DisplayDevice(mc_){}
#else /* FIM_WANT_NO_OUTPUT_CONSOLE */
	CACADevice():DisplayDevice(){}
#endif /* FIM_WANT_NO_OUTPUT_CONSOLE */

	fim_err_t display(
		const void *ida_image_img, // source image structure (struct ida_image *)(but we refuse to include header files here!)
		//void* rgb,// destination gray array and source rgb array
		fim_coo_t iroff,fim_coo_t icoff, // row and column offset of the first input pixel
		fim_coo_t irows,fim_coo_t icols,// rows and columns in the input image
		fim_coo_t icskip,	// input columns to skip for each line
		fim_coo_t oroff,fim_coo_t ocoff,// row and column offset of the first output pixel
		fim_coo_t orows,fim_coo_t ocols,// rows and columns to draw in output buffer
		fim_coo_t ocskip,// output columns to skip for each line
		fim_flags_t flags// some flags
		);
	int initialize(sym_keys_t &sym_keys);
	void finalize(void);

	fim_coo_t get_chars_per_line(void)const;
	int txt_width(void)const;
	int txt_height(void)const;
	int width(void)const;
	int height(void)const;
	fim_err_t status_line(const fim_char_t *msg);
	void status_screen(int desc,int draw_output){}
	fim_err_t console_control(fim_cc_t code);
	fim_bool_t handle_console_switch(void);
	int clear_rect_(
		void* dst,
		int oroff,int ocoff,
		int orows,int ocols,
		int ocskip);
	fim_err_t clear_rect(fim_coo_t x1, fim_coo_t x2, fim_coo_t y1,fim_coo_t y2);
	fim_err_t fill_rect(fim_coo_t x1, fim_coo_t x2, fim_coo_t y1,fim_coo_t y2, fim_color_t color) {/* FIXME: bogus implementation */ return clear_rect(x1,x2,y1,y2); }
	fim_err_t fs_puts(struct fs_font *f, fim_coo_t x, fim_coo_t y, const fim_char_t *str);

	fim_coo_t get_chars_per_column(void)const;
	fim_bpp_t get_bpp(void)const
	{
		return 1;
	}
	virtual fim_coo_t status_line_height(void)const;
	virtual fim_coo_t font_height(void)const;
	fim_sys_int get_input(fim_key_t * c, bool want_poll=false);
};


#endif /* FIM_WITH_CACALIB */
#endif /* FIM_CACADEVICE_H */
