/* $LastChangedDate: 2018-01-09 22:49:26 +0100 (Tue, 09 Jan 2018) $ */
/*
 FbiStuffPdf.cpp : fim functions for decoding PDF files

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
 * this code should be fairly correct, although unfinished
 * */

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "FbiStuff.h"
#include "FbiStuffLoader.h"

#ifdef HAVE_LIBPOPPLER

/*
 * Note : at the time of writing this, the poppler API is unstable,
 * and subject to change.
 * So when changing these headers here, take care of changing them
 * in the configure script, too.
 * And please don't blame me (fim's author)!
 */
#include <poppler/cpp/poppler-version.h>
#if (POPPLER_VERSION_MINOR>=21)
#if (POPPLER_VERSION_MINOR< 24)
#include <poppler/splash/SplashBitmap.h>
#include <poppler/splash/SplashTypes.h>
#else
#include <splash/SplashBitmap.h>
#include <splash/SplashTypes.h>
#endif /* (POPPLER_VERSION_MINOR< 24) */
#include <poppler/poppler-config.h>
#include <poppler/OutputDev.h>
#include <poppler/PDFDoc.h>
#include <poppler/SplashOutputDev.h>
#include <poppler/Page.h>
#include <poppler/GlobalParams.h>	/* globalParams lives here */
#endif /* (POPPLER_VERSION_MINOR>=21) */

#if HAVE_FILENO
#define FIM_PDF_USE_FILENO 1
#else
#define FIM_PDF_USE_FILENO 0
#endif /* HAVE_FILENO */

/*								*/

namespace fim
{
extern CommandConsole cc;

/* ---------------------------------------------------------------------- */
/* load                                                                   */

struct pdf_state_t {
	SplashBitmap*	    bmp ;
	PDFDoc *            pd ;
	SplashOutputDev *   od ;
	int row_stride;    /* physical row width in output buffer */
	fim_byte_t * first_row_dst;
};


/* ---------------------------------------------------------------------- */

static SplashColor splashColRed;
static SplashColor splashColGreen;
static SplashColor splashColBlue;
static SplashColor splashColWhite;
static SplashColor splashColBlack;
//static SplashColorMode gSplashColorMode = splashModeBGR8;
static SplashColorMode gSplashColorMode = splashModeRGB8;
#define SPLASH_COL_RED_PTR (SplashColorPtr)&(splashColRed[0])
#define SPLASH_COL_GREEN_PTR (SplashColorPtr)&(splashColGreen[0])
#define SPLASH_COL_BLUE_PTR (SplashColorPtr)&(splashColBlue[0])
#define SPLASH_COL_WHITE_PTR (SplashColorPtr)&(splashColWhite[0])
#define SPLASH_COL_BLACK_PTR (SplashColorPtr)&(splashColBlack[0])

static SplashColorPtr  gBgColor = SPLASH_COL_WHITE_PTR;

static void splashColorSet(SplashColorPtr col, Guchar red, Guchar green, Guchar blue, Guchar alpha)
{
    switch (gSplashColorMode)
    {
        case splashModeBGR8:
            col[0] = blue;
            col[1] = green;
            col[2] = red;
            break;
        case splashModeRGB8:
            col[0] = red;
            col[1] = green;
            col[2] = blue;
            break;
        default:
            assert(0);
            break;
    }
}

void SplashColorsInit(void)
{
    splashColorSet(SPLASH_COL_RED_PTR, 0xff, 0, 0, 0);
    splashColorSet(SPLASH_COL_GREEN_PTR, 0, 0xff, 0, 0);
    splashColorSet(SPLASH_COL_BLUE_PTR, 0, 0, 0xff, 0);
    splashColorSet(SPLASH_COL_BLACK_PTR, 0, 0, 0, 0);
    splashColorSet(SPLASH_COL_WHITE_PTR, 0xff, 0xff, 0xff, 0);
}

static void*
pdf_init(FILE *fp, const fim_char_t *filename, unsigned int page,
	  struct ida_image_info *i, int thumbnail)
{
	fim_char_t _[1];
	_[0]='\0';
	struct pdf_state_t * ds=FIM_NULL;
	int rotation=0,pageNo=page+1;
	double zoomReal=100.0;
	double hDPI;
	double vDPI;
	GBool  useMediaBox ;
	GBool  crop        ;
	GBool  doLinks     ;
	fim_int prd=cc.getIntVariable(FIM_VID_PREFERRED_RENDERING_DPI);
	prd=prd<1?FIM_RENDERING_DPI:prd;

	if(filename==std::string(FIM_STDIN_IMAGE_NAME))
	{
		std::cerr<<"sorry, stdin multipage file reading is not supported\n";
		goto retnull;
	}	/* a drivers's problem */ 

#if !FIM_PDF_USE_FILENO
	if(fp) fclose(fp);
#else
	if(fp)
	{
		// FIXME: this hack will only work on Linux.
		static fim_char_t linkname[FIM_LINUX_LINKFILENAME_BUFSIZE];
		sprintf(linkname,"/proc/self/fd/%d",fileno(fp));
		//printf("%s\n",linkname);
		filename=linkname;
		if(-1==access(filename,R_OK))
			return FIM_NULL;
	}
#endif /* FIM_PDF_USE_FILENO */


	ds = (struct pdf_state_t*)fim_calloc(1,sizeof(struct pdf_state_t));

	if(!ds)
		return FIM_NULL;

    	ds->first_row_dst = FIM_NULL;
	ds->bmp = FIM_NULL;
	ds->pd = FIM_NULL;
	ds->od = FIM_NULL;

	SplashColorsInit();

	// WARNING : a global variable from libpoppler! damn!!
	globalParams = new GlobalParams();
	if (!globalParams)
		goto err;

	globalParams->setErrQuiet(gFalse);

#if defined(POPPLER_VERSION_MINOR) && (POPPLER_VERSION_MINOR<22)
	globalParams->setBaseDir(_);
#endif /* defined(POPPLER_VERSION_MINOR) && (POPPLER_VERSION_MINOR<22) */

	ds->pd = new PDFDoc(new GooString(filename), FIM_NULL, FIM_NULL, (void*)FIM_NULL);
	if (!ds->pd)
        	goto err;

	if (!ds->pd->isOk())
		goto err;

	if (!ds->od)
	{
        	GBool bitmapTopDown = gTrue;
        	ds->od = new SplashOutputDev(gSplashColorMode, /*4*/3, gFalse, gBgColor, bitmapTopDown,gFalse/*antialias*/);
	        if (ds->od)
#ifdef POPPLER_VERSION	/* as of 0.20.2, from poppler/poppler-config.h */
			/* FIXME: this is an incomplete fix (triggered on 20120719's email on fim-devel);
			  I don't really know which version of poppler defines this macro first, but I assume 0.20.2 or so */
			ds->od->startDoc(ds->pd);
#else /* POPPLER_VERSION */
			ds->od->startDoc(ds->pd->getXRef());
#endif /* POPPLER_VERSION */
    	}
        if (!ds->od)
		goto err;

	i->dpi    = prd;
	hDPI = (double)i->dpi* (zoomReal * 0.01);
	vDPI = (double)i->dpi* (zoomReal * 0.01);

	useMediaBox = gFalse;
	crop        = gTrue;
	doLinks     = gTrue;

	i->npages = ds->pd->getNumPages();
	if(page>=i->npages || page<0)goto err;
	
	ds->pd->displayPage(ds->od, pageNo, hDPI, vDPI, rotation, useMediaBox, crop, doLinks, FIM_NULL, FIM_NULL);


	if(!ds->pd) goto err;

	ds->bmp = ds->od->takeBitmap();
	if(!ds->bmp) goto err;

	i->width  = ds->bmp->getWidth();
	i->height = ds->bmp->getHeight();

	return ds;
err:

	if(ds->pd)		delete ds->pd ;
	if(ds->od)	delete ds->od ;
	if (globalParams)	delete globalParams;
	globalParams = FIM_NULL;
	if(ds)fim_free(ds);
retnull:
	return FIM_NULL;
}

static void
pdf_read(fim_byte_t *dst, unsigned int line, void *data)
{
    	struct pdf_state_t *ds = (struct pdf_state_t*)data;
	if(!ds)return;

    	if(ds->first_row_dst == FIM_NULL)
    		ds->first_row_dst = dst;
	else return;

	memcpy(dst,ds->bmp->getDataPtr(),ds->bmp->getHeight()*ds->bmp->getWidth()*3);
}

static void
pdf_done(void *data)
{
    	struct pdf_state_t *ds = (struct pdf_state_t*)data;
	if(!ds) return;

	if(ds->pd)		delete ds->pd ;
	if(ds->od)	delete ds->od ;
	if (globalParams)	delete globalParams;
	globalParams = FIM_NULL;

	fim_free(ds);
}

/*
0000000: 2550 4446 2d31 2e34 0a25 d0d4 c5d8 0a35  %PDF-1.4.%.....5
*/
static struct ida_loader pdf_loader = {
    /*magic:*/ "%PDF-",// FI/*XME :*/ are sure this is enough ?
    /*moff:*/  0,
    /*mlen:*/  5,
    /*name:*/  "libpoppler",
    /*init:*/  pdf_init,
    /*read:*/  pdf_read,
    /*done:*/  pdf_done,
};

static void fim__init init_rd(void)
{
    fim_load_register(&pdf_loader);
}

}
#endif // ifdef HAVE_LIBPOPPLER
