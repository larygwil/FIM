/* $LastChangedDate: 2017-09-10 14:53:32 +0200 (Sun, 10 Sep 2017) $ */
/*
 FimWindow.h : Fim's own windowing system header file

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
#ifndef FIM_WINDOW_H
#define FIM_WINDOW_H

#include "fim.h"
#include "DisplayDevice.h"

#ifdef FIM_WINDOWS

#include <vector>
#include <iostream>

#define FIM_DISABLE_WINDOW_SPLITTING 1

namespace fim
{
#define FIM_BUGGED_ENLARGE 0

/*
  The window class should model the behaviour of a binary splitting window
 in a portable manner.
  It should not be tied to a particular window system or graphical environment,
 but it should mimic the behaviour of Vim's windowing system.

 (x,y) : upper left point in
 +--------------+
 |              |
 |              |
 |              |
 +--------------+
 |              |
 +--------------+
                 (x+w,y+h) : lower right point out
*/
class Rect  FIM_FINAL
{
	public:
	fim_coo_t x_,y_,w_,h_;	// units, not pixels
	void print(void);

	Rect(fim_coo_t x,fim_coo_t y,fim_coo_t w,fim_coo_t h);

	Rect(const Rect& rect);

	public:

	enum Splitmode{ Left,Right,Upper,Lower};

	Rect hsplit(Splitmode s);
	Rect vsplit(Splitmode s);
	Rect split(Splitmode s);

	fim_err_t vlgrow(fim_coo_t units=FIM_CNS_WGROW_STEPS_DEFAULT);
	fim_err_t vlshrink(fim_coo_t units=FIM_CNS_WGROW_STEPS_DEFAULT);
	fim_err_t vugrow(fim_coo_t units=FIM_CNS_WGROW_STEPS_DEFAULT);
	fim_err_t vushrink(fim_coo_t units=FIM_CNS_WGROW_STEPS_DEFAULT);

	fim_err_t hlgrow(fim_coo_t units=FIM_CNS_WGROW_STEPS_DEFAULT);
	fim_err_t hrshrink(fim_coo_t units=FIM_CNS_WGROW_STEPS_DEFAULT);
	fim_err_t hrgrow(fim_coo_t units=FIM_CNS_WGROW_STEPS_DEFAULT);
	fim_err_t hlshrink(fim_coo_t units=FIM_CNS_WGROW_STEPS_DEFAULT);
	bool operator==(const Rect&rect)const;

	fim_coo_t setwidth(fim_coo_t w);
	fim_coo_t setheight(fim_coo_t h);
	fim_coo_t setxorigin(fim_coo_t x);
	fim_coo_t setyorigin(fim_coo_t y);
	fim_coo_t height(void)const;
	fim_coo_t width(void)const;
	fim_coo_t xorigin(void)const;
	fim_coo_t yorigin(void)const;
};

#ifdef FIM_NAMESPACES
class FimWindow FIM_FINAL:public Namespace
#else /* FIM_NAMESPACES */
class FimWindow FIM_FINAL
#endif /* FIM_NAMESPACES */
{
	private:

	enum Spacings{ hspacing=0, vspacing=0};
	enum Moves{Up,Down,Left,Right,NoMove};

	DisplayDevice *displaydevice_;
	public:
	Rect corners_;//,status,canvas;
	private:
	bool focus_;	// if 0 left/up ; otherwise right/lower

	FimWindow *first_,*second_;
	bool amroot_;
	
	void split(void);
	void hsplit(void);
	void vsplit(void);
	bool close(void);
	bool swap(void); // new
	void balance(void);
	bool chfocus(void);
	Moves move_focus(Moves move);
	Moves reverseMove(Moves move);
	bool normalize(void);
	fim_err_t enlarge(fim_coo_t units);
	fim_err_t henlarge(fim_coo_t units);
	fim_err_t venlarge(fim_coo_t units);

	bool can_vgrow(const FimWindow& window, fim_coo_t howmuch)const;
	bool can_hgrow(const FimWindow& window, fim_coo_t howmuch)const;

	private:
	explicit FimWindow(const FimWindow& root);
	bool isleaf(void)const;
	bool isvalid(void)const;
	bool issplit(void)const;
	bool ishsplit(void)const;
	bool isvsplit(void)const;
	fim_err_t hnormalize(fim_coo_t x, fim_coo_t w);
	fim_err_t vnormalize(fim_coo_t y, fim_coo_t h);
	int count_hdivs(void)const;
	int count_vdivs(void)const;

	fim_err_t vlgrow(fim_coo_t units);
	fim_err_t vugrow(fim_coo_t units);
	fim_err_t vushrink(fim_coo_t units);
	fim_err_t vlshrink(fim_coo_t units);

	fim_err_t hlgrow(fim_coo_t units);
	fim_err_t hrgrow(fim_coo_t units);
	fim_err_t hlshrink(fim_coo_t units);
	fim_err_t hrshrink(fim_coo_t units);

	FimWindow& focused(void)const;
	FimWindow& shadowed(void)const;

	FimWindow& upper(void);
	FimWindow& lower(void);
	FimWindow& left(void);
	FimWindow& right(void);

	bool operator==(const FimWindow&window)const;

	Viewport *viewport_;

	const FimWindow& c_focused(void)const;
	const FimWindow& c_shadowed(void)const;

	Viewport& current_viewport(void)const;
	CommandConsole& commandConsole_;

	FimWindow& operator= (const FimWindow& w);

	public:
	void setroot(void);	// only one root window should exist

	/* The only public member function launching exceptions is the constructor now.
	 * */
	FimWindow(CommandConsole& c, DisplayDevice *dd, const Rect& corners, Viewport* vp=FIM_NULL); // throws FIM_E_NO_MEM exception
	fim_err_t update(const Rect& corners);

	Viewport * current_viewportp(void)const;
        fim_cxr fcmd_cmd(const args_t&args);
	bool recursive_redisplay(void)const;	//exception safe
	bool recursive_display(void)const;		//exception safe

	const Image* getImage(void)const;		//exception safe
#if 0
	void print(void);
	void print_focused(void);
	void draw(void)const;
#endif

	~FimWindow(void);
	virtual size_t byte_size(void)const;
	void should_redraw(enum fim_redraw_t sr = FIM_REDRAW_NECESSARY); /* FIXME: this is a wrapper to Viewport's, until multiple windows get introduced again. */
	fim_bool_t need_redraw(void)const;
};
} /* namespace fim */
#endif /* FIM_WINDOWS */
#endif /* FIM_WINDOW_H */
