//#!/usr/bin/tcc -run
/****************************************************************
 * $ID: word-boxes.c   Fri, 26 Apr 2013 10:56:16 +0800  mhfan $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  ∑∂√¿ª‘(MeiHui FAN)  <mhfan@ustc.edu>            *
 *                                                              *
 * CopyLeft (c)  2013  M.H.Fan                                  *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	        *
 *   under the terms of the GNU General Public Licence (GPL).   *
 ****************************************************************/

#include <errno.h>
#include <string.h>

#include "allheaders.h"

#ifndef INT_MAX
#define INT_MAX 100000000
#endif

int main(int argc, char* argv[])
{
    PIX *pixs, *pixb, *pixt;
    int minb;

    if (argc < 2) {
USAGE:	fprintf(stderr, "Usage:  %s </path/to/text-image>\n"
		"\t\t[binarize-threshold] [minw:minh:maxw:maxh]\n",
		strrchr(argv[0], '/') + 1);
	return EINVAL;
    }

    if (2 < argc) {	errno = 0;
	minb = strtol(argv[2], NULL, 10);

	if (errno < 0) {
	    fprintf(stderr, "strtol: %s\n", strerror(errno));
	    goto USAGE;
	}
    } else minb = 180;

    if (!(pixs = pixRead(argv[1]))) ;

    if (1 && (pixt = pixBackgroundNormMorph(pixs, NULL, 4, 5, 248))) {
	pixDestroy(&pixs);	pixs = pixt;
    } else
    if (0 && (pixt = pixBackgroundNorm(pixs, NULL, NULL,
	    10, 15, 60, 40, 248, 2, 1))) {
	pixDestroy(&pixs);	pixs = pixt;
    }

    if (1 && (pixt = pixFindSkewAndDeskew(pixs, 1, NULL, NULL))) {
	pixDestroy(&pixs);	pixs = pixt;
    }	if (0 && pixDisplay(pixs, 0, 0)) ;

    if (1) {	PTA *ptas, *ptad;
	if (!(pixb = pixConvertTo1(pixs, minb))) ;

	// pixt = pixDeskewLocal(pixs, 10, 0, 0, 0.0, 0.0, 0.0))
	if (!pixGetLocalSkewTransform(pixb,
		10, 0, 0, 0.0, 0.0, 0.0, &ptas, &ptad)) {
	    if ((pixt = pixProjectiveSampledPta(pixs,
		ptad, ptas, L_BRING_IN_WHITE))) {
		pixDestroy(&pixs);	pixs = pixt;
	    }	ptaDestroy(&ptas);	ptaDestroy(&ptad);
	}	pixDestroy(&pixb);
    }

    if (0 && (pixt = pixGammaTRC(NULL, pixs, 1.0, 30, 180))) {
	pixDestroy(&pixs);	pixs = pixt;
    }

    if (!(pixb = pixConvertTo1(pixs, minb))) ;

    if (0) { pixDestroy(&pixs); pixs = pixCopy(pixs, pixb); }	// XXX:

    if (1) {
	BOX* box;
	int i, n, j, m;
	PIX *pixi, *pixl;
	BOXA *boxi, *boxl;
	int x, y, w, h, wid;
	int X = INT_MAX, Y = INT_MAX, W = 0, H;

	// XXX: do smaller(or no) pixOpenBrick
	if (pixGetRegionsBinary(pixb, &pixi, &pixl, NULL, 0)) ;

	boxl = pixConnComp(pixl, NULL, 4);
	n = boxaGetCount(boxl);

	for (i = 0; i < n; ++i) {   BOXA* boxa;
	    box = boxaGetBox(boxl, i, L_CLONE);
	    boxGetGeometry(box, &x, &y, &w, &h);

	    if (w < 30 || h < 30 || w < h || h < (w / 40)) {
		boxDestroy(&box);	continue;
		boxaRemoveBox(boxl, i);
	    }

	    if (x < X) X = x;	if (y < Y) Y = y; if (W < w) W = w;

	    pixt = pixClipRectangle(pixb, box, NULL);
	    boxDestroy(&box);

	    // XXX: for English
	    if (0) pixt = pixDilateBrick(pixt, pixt, h >> 1, h >> 1); else

	    pixt = pixDilateBrick(pixt, pixt, 16 < h ? h >> 4 : 1, h << 1);
	    if (0 && pixDisplay(pixt, 0, 0)) ;

	    boxa = pixConnComp(pixt, NULL, 8);
	    pixDestroy(&pixt);

	    wid = (h * 3) >> 2;
	    //boxaShift(boxa, x, y);
	    m = boxaGetCount(boxa);

	    for (j = 0; j < m; ++j) {
		int x0, y0, w0;

		box = boxaGetBox(boxa, j, L_CLONE);
		boxGetGeometry(box, &x0, &y0, &w0, NULL);

		// merge adjacent 2 or 3 small boxes
		if (1 && w0 < wid && (j + 1) < m) {
		    BOX* boxn;	int xn, wn;

		    boxn = boxaGetBox(boxa, j + 1, L_CLONE);
		    boxGetGeometry(boxn, &xn, NULL, &wn, NULL);

		    if ((w0 = xn + wn - x0) < h) {
			boxaSparseClearBox(boxa, ++j);

			if (w0 < wid && (j + 1) < m) {
			    boxDestroy(&boxn);
			    boxn = boxaGetBox(boxa, j + 1, L_CLONE);
			    boxGetGeometry(boxn, &xn, NULL, &wn, NULL);

			    if ((wn = xn + wn - x0) < h) {
				boxaSparseClearBox(boxa, ++j);
				w0 = wn;
			    }
			}

			boxSetGeometry(box, -1, -1, w0, -1);
		    }	boxDestroy(&boxn);
		}

		boxSetGeometry(box, x + x0, y + y0, -1, -1);
		boxDestroy(&box);
	    }	boxaSparseCompact(boxa);

	    if (1 && (pixt = pixDrawBoxa(pixs, boxa, 1, 0xff000000))) {
		pixDestroy(&pixs);	pixs = pixt;
	    }	boxaDestroy(&boxa);
	}   H = y + h;

	pixDestroy(&pixb);
	boxaDestroy(&boxl);

	boxi = pixConnComp(pixi, NULL, 4);
	n = boxaGetCount(boxi);

	for (i = 0; i < n; ++i) {   PIX* pixm;
	    box = boxaGetBox(boxi, i, L_CLONE);
	    boxGetGeometry(box, &x, &y, &w, &h);

	    //if (w < 30 || h < 30) continue;
	    if (x < X) X = x;	if (y < Y) Y = y; if (W < w) W = w;

	    pixm = pixClipRectangle(pixi, box, NULL);
	    pixt = pixClipMasked(pixs, pixm, x, y, 0);
	    pixDestroy(&pixm);

	    // TODO:

	    pixDestroy(&pixt);
            boxDestroy(&box);
	}   if (H < y + h) H = y + h;

	if (0 && pixDisplay(pixi, 0, 0)) ;
	pixDestroy(&pixi);

	if (0 < n && (pixt = pixDrawBoxa(pixs, boxi, 1, 0x0000ff00))) {
	    pixDestroy(&pixs);	pixs = pixt;
	}

	// box = boxaGetBound(boxl);
	if ((box = boxCreate(X, Y, W, H - Y))) {
	    PTA* pta = generatePtaBox(box, 1);
	    boxDestroy(&box);

	    pixRenderPtaArb(pixs, pta, 0x00, 0x00, 0xff);
	    ptaDestroy(&pta);
	}

	pixDestroy(&pixl);
	boxaDestroy(&boxi);
    } else {	BOXA *boxa;	NUMA *nai;
	int minw, minh, maxw, maxh;

	if (3 < argc) {	char* str;
	    str = argv[3] - 1;  errno = 0;

	    minw = strtol(str + 1, &str, 10);
	    minh = strtol(str + 1, &str, 10);
	    maxw = strtol(str + 1, &str, 10);
	    maxh = strtol(str + 1, &str, 10);

	    if (errno < 0) {
		fprintf(stderr, "strtol: %s\n", strerror(errno));
		goto USAGE;
	    }
	} else minw = 6, minh = 4, maxw = 500, maxh = 100;

	if (pixGetWordBoxesInTextlines(pixb,
		1, minw, minh, maxw, maxh, &boxa, &nai)) ;
	pixDestroy(&pixb);

	//if ((pixt = pixDrawBoxaRandom(pixs, boxa, 1)))
	if ((pixt = pixDrawBoxa(pixs, boxa, 1, 0xff000000))) {
	    pixDestroy(&pixs);	pixs = pixt;
	}   boxaDestroy(&boxa);
    }

    if (pixDisplay(pixs, 0, 0)) ;
    pixDestroy(&pixs);

    return 0;
}

// vim:sts=4:ts=8:
