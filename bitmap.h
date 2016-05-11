#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <math.h>

/* Virtual Bitmap Routines, by Robin Salih */

struct bmpfile
	{
	unsigned short tag;
	unsigned long length;
	unsigned short res1, res2;
	unsigned long offset;
	unsigned long headersize;
	unsigned long width, height;
	unsigned short planes, bpp;
	unsigned long compression, imagesize, xres, yres, clrs, impclrs;
	} ;

struct hand
	{
	int xpos, ypos, oldxpos, oldypos;
	char  *data;
	char filename[64];
	unsigned char forcol, backcol, graphcol, graphmeth, mode, vdu5, vdu21;
	unsigned short width, height, bpp;
	unsigned short gwx1, gwy1, gwx2, gwy2, twx1, twx2, twy1, twy2;
	} ;

typedef struct hand IMAGE;

int newbitmap (IMAGE **hand, char *filename, int width, int height, int bpp);
void close_bitmap(IMAGE *hand);
unsigned long round_to_word(long int size_in_bits);
void define_colour(IMAGE *hand, unsigned char col, unsigned char red, unsigned char green, unsigned char blue);
void greyscale(IMAGE *hand);
void setpixel(IMAGE *hand, unsigned char red, unsigned char green, unsigned char blue, int xpos, int ypos);
void clear(IMAGE *hand);
void get_coords(IMAGE *hand, int x, int y, int *realx, int *realy);
void line(IMAGE *hand, int x1, int y1, int x2, int y2, unsigned char red, unsigned char green, unsigned char blue);
void xline(IMAGE *hand, int x, int y1, int y2, unsigned char red, unsigned char green, unsigned char blue);
void yline(IMAGE *hand, int y, int x1, int x2, unsigned char red, unsigned char green, unsigned char blue);
void ellipse(IMAGE *hand, int rad_x, int rad_y, int cent_x, int cent_y, unsigned char red, unsigned char green, unsigned char blue);
void filled_ellipse(IMAGE *hand, int rad_x, int rad_y, int cent_x, int cent_y, unsigned char red, unsigned char green, unsigned char blue);
void triangle(IMAGE *hand, int x1, int y1, int x2, int y2, int x3, int y3, unsigned char red, unsigned char green, unsigned char blue);

