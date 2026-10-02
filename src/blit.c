#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include "blit.h"

static Uint8 *blitMap = NULL;

static void CopyBlitImageTo1Byte(int width, int height, Uint8 *src,
                        int srcskip, Uint8 *dst, int dstskip,
						Uint8 *map)
{
	int c;

	while ( height-- ) {
	    Uint8 byte = 0, bit;
		for ( c=0; c<width; ++c ) {
			if ( (c&7) == 0 ) {
				byte = *src++;
			}
			bit = (byte&0x80)>>7;
			if ( bit ) 
 			   *dst = map[1];
            else
               *dst = map[0];
			dst++;
			byte <<= 1;
		}
		src += srcskip;
		dst += dstskip;
	}
}
static void CopyBlitImageTo2Byte(int width, int height, Uint8 *src,
                        int srcskip, Uint16 *dst, int dstskip,
						Uint16 *map)
{
	int c;

	while ( height-- ) {
	        Uint8 byte = 0, bit;
	    	for ( c=0; c<width; ++c ) {
			if ( (c&7) == 0 ) {
				byte = *src++;
			}
			bit = (byte&0x80)>>7;
			if ( bit ) 
			   *dst = map[1];
            else
               *dst = map[0];
			byte <<= 1;
			dst++;
		}
		src += srcskip;
		dst += dstskip;
	}
}

static void CopyBlitImageTo3Byte(int width, int height, Uint8 *src,
                        int srcskip, Uint8 *dst, int dstskip,
						Uint8 *map)
{
	int c;

	while ( height-- ) {
	        Uint8 byte = 0, bit;
	    	for ( c=0; c<width; ++c ) {
			if ( (c&7) == 0 ) {
				byte = *src++;
			}
			bit = (byte&0x80)>>7;
			if ( bit ) {
              dst[0] = map[3];
              dst[1] = map[4];
              dst[2] = map[5];
              }
            else {
			  dst[0] = map[0];
			  dst[1] = map[1];
			  dst[2] = map[2];
            }
			byte <<= 1;
			dst += 3;
		}
		src += srcskip;
		dst += dstskip;
	}
}
static void CopyBlitImageTo4Byte(int width, int height, Uint8 *src,
                        int srcskip, Uint32 *dst, int dstskip,
						Uint32 *map)
{
	int c;

	while ( height-- ) {
	        Uint8 byte = 0, bit;
	    	for ( c=0; c<width; ++c ) {
			if ( (c&7) == 0 ) {
				byte = *src++;
			}
			bit = (byte&0x80)>>7;
			if ( bit ) 
    		   *dst = map[1];
            else
               *dst = map[0];
			byte <<= 1;
			dst++;
		}
		src += srcskip;
		dst += dstskip;
	}
}
static void XorBlitImageTo1Byte(int width, int height, Uint8 *src,
                        int srcskip, Uint8 *dst, int dstskip,
						Uint8 *map)
{
	int c;

	while ( height-- ) {
        Uint8 byte = 0, bit;
   		for ( c=0; c<width; ++c ) {
			if ( (c&7) == 0 ) {
				byte = *src++;
			}
			bit = (byte&0x80)>>7;
			if ( bit ) {
				if (*dst == map[0])
					*dst = map[1];
				else
					*dst = map[0];
			}
			dst++;
			byte <<= 1;
		}
		src += srcskip;
		dst += dstskip;
	}
}
static void XorBlitImageTo2Byte(int width, int height, Uint8 *src,
                        int srcskip, Uint16 *dst, int dstskip,
						Uint16 *map)
{
	int c;

	while ( height-- ) {
	        Uint8 byte = 0, bit;
	    	for ( c=0; c<width; ++c ) {
			if ( (c&7) == 0 ) {
				byte = *src++;
			}
			bit = (byte&0x80)>>7;
			if ( bit ) {
                if (*dst == map[0])
				   *dst = map[1];
                else
                   *dst = map[0];
			}
			byte <<= 1;
			dst++;
		}
		src += srcskip;
		dst += dstskip;
	}
}

static void XorBlitImageTo3Byte(int width, int height, Uint8 *src,
                        int srcskip, Uint8 *dst, int dstskip,
						Uint8 *map)
{
	int c, o;

	while ( height-- ) {
	        Uint8 byte = 0, bit;
	    	for ( c=0; c<width; ++c ) {
			if ( (c&7) == 0 ) {
				byte = *src++;
			}
			bit = (byte&0x80)>>7;
			if ( bit ) {
			    o = bit * 4;
                if ((dst[0] == map[0]) && (dst[1] == map[1]) && (dst[2] == map[2])) {
                  dst[0] = map[3];
                  dst[1] = map[4];
                  dst[2] = map[5];
                  }
                else {
				  dst[0] = map[0];
				  dst[1] = map[1];
				  dst[2] = map[2];
                  }
			}
			byte <<= 1;
			dst += 3;
		}
		src += srcskip;
		dst += dstskip;
	}
}
static void XorBlitImageTo4Byte(int width, int height, Uint8 *src,
                        int srcskip, Uint32 *dst, int dstskip,
						Uint32 *map)
{
	int c;

	while ( height-- ) {
	        Uint8 byte = 0, bit;
	    	for ( c=0; c<width; ++c ) {
			if ( (c&7) == 0 ) {
				byte = *src++;
			}
			bit = (byte&0x80)>>7;
			if ( bit ) {
                if (*dst == map[0])
				   *dst = map[1];
                else
                   *dst = map[0];
			}
			byte <<= 1;
			dst++;
		}
		src += srcskip;
		dst += dstskip;
	}
}


/* The general purpose software blit routine */
int TrsSoftBlit(SDL_Surface *src, SDL_Rect *srcrect,
				SDL_Surface *dst, SDL_Rect *dstrect, int xor)
{
	int dst_locked;
	Uint8 *srcpix, *dstpix, *map;
	int width, height, srcskip, dstskip;

	/* Lock the destination if it's in hardware */
	dst_locked = 0;
	if ( SDL_MUSTLOCK(dst) ) {
		if ( SDL_LockSurface(dst) < 0 ) {
			return(-1);
		} else {
			dst_locked = 1;
		}
	}

		/* Set up the blit information */
	srcpix = (Uint8 *)src->pixels +
			(Uint16)srcrect->y*src->pitch +
			((Uint16)srcrect->x*SDL_BITSPERPIXEL(src->format))/8;
	srcskip=src->pitch-(((int)srcrect->w)*SDL_BITSPERPIXEL(src->format))/8;
	dstpix = (Uint8 *)dst->pixels +
			(Uint16)dstrect->y*dst->pitch +
			(Uint16)dstrect->x*SDL_BYTESPERPIXEL(dst->format);
	width = dstrect->w;
	height = dstrect->h;
	dstskip=dst->pitch-((int)dstrect->w)*SDL_BYTESPERPIXEL(dst->format);
	map = blitMap;

	/* Run the actual software blit */
    switch(SDL_BYTESPERPIXEL(dst->format)) {
       case 1:
		   if (xor)
				XorBlitImageTo1Byte(width, height, srcpix, srcskip,
							dstpix, dstskip, map);
		   else
				CopyBlitImageTo1Byte(width, height, srcpix, srcskip,
							dstpix, dstskip, map);
           break;
       case 2:
		   if (xor)
				XorBlitImageTo2Byte(width, height, srcpix, srcskip,
							(Uint16 *) dstpix, dstskip/2, (Uint16 *) map);
		   else
				CopyBlitImageTo2Byte(width, height, srcpix, srcskip,
							(Uint16 *) dstpix, dstskip/2, (Uint16 *) map);
           break;
       case 3:
		   if (xor)
				XorBlitImageTo3Byte(width, height, srcpix, srcskip,
							dstpix, dstskip, map);
		   else
				CopyBlitImageTo3Byte(width, height, srcpix, srcskip,
							dstpix, dstskip, map);
           break;
       case 4:
		   if (xor)
				XorBlitImageTo4Byte(width, height, srcpix, srcskip,
							(Uint32 *) dstpix, dstskip/4, (Uint32 *) map);
		   else
				CopyBlitImageTo4Byte(width, height, srcpix, srcskip,
							(Uint32 *) dstpix, dstskip/4, (Uint32 *) map);
           break;
       default:
           return(-1);
           break;
    }

	if ( dst_locked ) {
		SDL_UnlockSurface(dst);
	}

	return(0);
}




