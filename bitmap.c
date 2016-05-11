#include "bitmap.h"

#define UNIX 1

/* By Robin Salih */

int newbitmap (IMAGE *(*handle), char *filename, int width, int height, int bpp)
	{
	struct bmpfile bmp;
	unsigned long size;
	bmp.tag = 0x4d42;
	bmp.res1 = bmp.res2 = 0;
	bmp.planes = 1;
	bmp.bpp = bpp;
	bmp.headersize = 40;
	bmp.offset = sizeof(struct bmpfile) + 4 * (1 << bpp);
	bmp.width = width;
	bmp.height = height;
	(*handle) = malloc(sizeof(struct hand));
	if (!(*handle)) { printf ("memory allocation failed\n");return (0);}
	strcpy((*handle) -> filename, filename);
	(*handle) -> bpp = bmp.bpp;
	(*handle) -> width = bmp.width;
	(*handle) -> height = bmp.height;
	(*handle) -> xpos = (*handle) -> ypos = (*handle) -> oldxpos = (*handle) -> oldypos = 0;
	(*handle) -> twx1 = (*handle) -> twy1 = (*handle) -> gwx1 = (*handle) -> gwy1 = 0;
	(*handle) -> twx2 = (width / 8) - 1;
	(*handle) -> twy2 = (height / 8) - 1;
	(*handle) -> gwx2 = width - 1;
	(*handle) -> gwy2 = height - 1;
	(*handle) -> forcol = 0;
	(*handle) -> backcol = (unsigned char) ((1 << bpp) - 1);
	(*handle) -> graphcol = (*handle) -> graphmeth = 0;
	(*handle) -> vdu5 = (*handle) -> vdu21 = 0;
	(*handle) -> mode = 7;
	bmp.imagesize = 4 * round_to_word(bpp * width) * height;
	bmp.clrs = bmp.impclrs = 1 << bpp;
	bmp.compression = 0;
	bmp.xres = bmp.yres = 0;
	size = bmp.offset + 4 * round_to_word(bpp * width) * height;
	bmp.length = round_to_word(size * 8);
	/*if (size > 131072)
		{
		register unsigned short i = 6;
		do
			{
			i++;
			}
		while ((1 << i) < (short) size);
		size = 1 << i;
		}*/
	(*handle) -> data = (char *) malloc((unsigned) size);
	if (!(*handle) -> data) return (0);
	memcpy ((*handle) -> data, &bmp, sizeof(struct bmpfile));
	return(1);
	}

void close_bitmap(IMAGE (*handle))
	{
	struct bmpfile bmp;
	FILE *fp;
	memcpy(&bmp, handle -> data, sizeof(struct bmpfile));
	if (UNIX)
		{
		char *data = handle -> data;
		data[0] = bmp.tag & 0xff;
		data[1] = (bmp.tag & 0xff00) >> 8;
		data[2] = bmp.length & 0xff;
		data[3] = (bmp.length & 0xff00) >> 8;
		data[4] = (bmp.length & 0xff0000) >> 16;
		data[5] = (bmp.length & 0xff000000) >> 24;
		data[6] = data[7] = data[8] = data[9] = (char) 0;
		data[10] = bmp.offset & 0xff;
		data[11] = (bmp.offset & 0xff00) >> 8;
		data[12] = data[13] = 0;
		data[14] = 40; data[15] = data[16] = data[17] = 0;
		data[18] = bmp.width & 0xff;
		data[19] = (bmp.width & 0xff00) >> 8;
		data[22] = bmp.height & 0xff;
		data[23] = (bmp.height & 0xff00) >> 8;
		data[20] = data[21] = data[24] = data[25] = 0;
		data[26] = 1; data[27] = 0; data[28] = bmp.bpp; data[29] = 0;
		data[30] = data[31] = data[32] = data[33] = 0;
		data[34] = bmp.imagesize & 0xff;
		data[35] = (bmp.imagesize & 0xff00) >> 8;
		data[36] = (bmp.imagesize & 0xff0000) >> 16;
		data[37] = (bmp.imagesize & 0xff000000) >> 24;
		data[38] = 0; data[39] = data[40] = data[41] = 0;
		data[42] = 0; data[43] = data[44] = data[45] = 0;
		data[46] = bmp.clrs & 0xff;
                data[47] = (bmp.clrs & 0xff00) >> 8;
                data[48] = (bmp.clrs & 0xff0000) >> 16;
                data[49] = (bmp.clrs & 0xff000000) >> 24;
                data[50] = bmp.impclrs & 0xff;
                data[51] = (bmp.impclrs & 0xff00) >> 8;
                data[52] = (bmp.impclrs & 0xff0000) >> 16;
                data[53] = (bmp.impclrs & 0xff000000) >> 24;
		if (bmp.bpp != 24) memcpy(data + 54, data + sizeof(struct bmpfile), 4 * (1 << bmp.bpp));
		}	
	fp = fopen(handle -> filename, "wb");
	fwrite (handle -> data, bmp.length * 4, 1, fp);
	fclose (fp);
	free (handle -> data);
	free (handle);
	}

unsigned long round_to_word(long int size_in_bits)

/*  This function receives a number of bits, and rounds them up to the nearest word.
eg. Input: 30, Output: 1
    Input: 124, Output: 32 */

	{
	if (size_in_bits % 32)
		{
		return ((size_in_bits + 32 - (size_in_bits % 32)) / 32);
		}
	else
		{
		return (size_in_bits / 32);
		}
	}

void define_colour(IMAGE (*handle), unsigned char col, unsigned char red, unsigned char green, unsigned char blue)
	{
	if (handle -> bpp == 24) return;
	col %= (1 << handle -> bpp);
	*(handle -> data + sizeof(struct bmpfile) + 4 * col) = blue;
	*(handle -> data + sizeof(struct bmpfile) + 4 * col + 1) = green;
	*(handle -> data + sizeof(struct bmpfile) + 4 * col + 2) = red;
	}

void greyscale(IMAGE (*handle))
	{
	register int i;
	if (handle -> bpp == 24) return;
	for (i = 0; i < (int) (1 << (handle -> bpp)); i++)
		{
		unsigned char value = (unsigned char) (i * (255 / ((1 << handle -> bpp) - 1)));
		define_colour(handle, (unsigned char) i, value, value, value);
		}
	}

void setpixel(IMAGE (*handle), unsigned char red, unsigned char green, unsigned char blue, int xpos, int ypos)
	{
	long byte;
	char bit;
	if (xpos < 0 || ypos < 0 || xpos >= (int) handle -> width || ypos >= (int) handle -> height) return;
	bit = (char) ((xpos * handle -> bpp) % 8);
	byte = (long) ((xpos * handle -> bpp) / 8 + sizeof(struct bmpfile) + 4 * (1 << handle -> bpp) + ypos * round_to_word(handle -> width * handle -> bpp) * 4);
	switch (handle -> bpp)
		{
		case 1 :
			red %= 2;
			if (red) *(byte + handle -> data) |= (1 << (7 - bit));
			else *(byte + handle -> data) &= (255 - (1 << (7 - bit)));
			break;
		case 4 :
			red %= 16;
			if (bit)
				{
				*(handle -> data + byte) &= 0xf0;
				*(handle -> data + byte) |= red;
				}
			else
				{
				*(handle -> data + byte) &= 0x0f;
				*(handle -> data + byte) |= red << 4;
				}
			break;
		case 8:
			*(handle -> data + byte) = red;
			break;
		case 24:
			*(handle -> data + byte) = blue;
			*(handle -> data + byte + 1) = green;
			*(handle -> data + byte + 2) = red;
			break;
		}
	}

void clear(IMAGE (*handle))
	{
	long size = round_to_word(handle -> bpp * handle -> width) * 4 * handle -> height + sizeof(struct bmpfile) + 3 * (1 << handle -> bpp);
	long i;
	for (i = sizeof(struct bmpfile) + 3 * (1 << handle -> bpp); i < size; i++)
		{
		*(handle -> data + i) = 0;
		}
	}

void get_coords(IMAGE (*handle), int x, int y, int *realx, int *realy)
	{
	x += handle -> gwx1;
	y += handle -> gwy1;
	switch (handle -> mode)
		{
		case 0 :
			x >>= 1;
			y >>= 2;
			break;
		case 1 :
		case 4 :
			x >>= 2;
			y >>= 2;
			break;
		case 5 :
		case 2 :
			x >>= 4;
			y >>= 2;
			break;
		}
	*realx = x;
	*realy = y;
	}

void line(IMAGE (*handle), int x1, int y1, int x2, int y2, unsigned char red, unsigned char green, unsigned char blue)
	{
	register int i,j;
	double step, stept;
	if (x1 == x2)
		{
		xline(handle, x1, y1, y2, red, green, blue);
		return;
		}
	if (y1 == y2)
		{
		yline(handle, y1, x1, x2, red, green, blue);
		return;
		}
	i = (x1 - x2);
	j = (y1 - y2);
	if (abs(i) > abs(j))
		{
		step = (double) j / (double) i;
		if (x1 > x2)
			{
			j = x1;
			x1 = x2;
			x2 = j;
			j = y1;
			y1 = y2;
			y2 = j;
			}
		stept = y1;
		for (i = x1; i <= x2; i++)
			{
			setpixel(handle, red, green, blue, i, (int) stept);
			stept += step;
			}
		}
	else
		{
		step = (double) i / (double) j;
		if (y1 > y2)
			{
			j = x1;
			x1 = x2;
			x2 = j;
			j = y1;
			y1 = y2;
			y2 = j;
			}
		stept = x1;
		for (i = y1; i <= y2; i++)
			{
			setpixel(handle, red, green, blue, (int) stept, i);
			stept += step;
			}
		}
	}

void xline(IMAGE (*handle), int x, int y1, int y2, unsigned char red, unsigned char green, unsigned char blue)
	{
	int i;
	if (y1 == y2)
		{
		setpixel(handle, red, green, blue, x, y1);
		return;
		}
	if (y2 < y1)
		{
		i = y2;
		y2 = y1;
		y1 = i;
		}
	for (i = y1; i <= y2; i++)
		{
		setpixel(handle, red, green, blue, x, i);
		}
	}

void yline(IMAGE (*handle), int y, int x1, int x2, unsigned char red, unsigned char green, unsigned char blue)
	{
	int i;
	if (x2 < x1)
		{
		i = x2;
		x2 = x1;
		x1 = i;
		}
	for (i = x1; i <= x2; i++)
		{
		setpixel(handle, red, green, blue, i, y);
		}
	}

void filled_ellipse(IMAGE (*handle), int rad_x, int rad_y, int cent_x, int cent_y, unsigned char red, unsigned char green, unsigned char blue)
	{
	register int y;
	double x;
	if (!(rad_x && rad_y)) return;
	rad_y = abs (rad_y);
	for (y = 0; y <= rad_y; y++)
		{
	/*	x = (double) rad_x * sqrt(1 - ((double) y / (double) rad_y) * ((double) y / (double) rad_y));
		xline(handle, cent_y + y, cent_x + (int) x, cent_x - (int) x, red, green, blue);
		xline(handle, cent_y - y, cent_x + (int) x, cent_x - (int) x, red, green, blue);
		*/}
	}

void ellipse(IMAGE (*handle), int rad_x, int rad_y, int cent_x, int cent_y, unsigned char red, unsigned char green, unsigned char blue)
	{
	register int y;
	double x;
	if (!(rad_x && rad_y)) return;
	rad_y = abs (rad_y);
	for (y = 0; y <= rad_y; y++)
		{
		/*x = (double) rad_x * sqrt(1 - ((double) y / (double) rad_y) * ((double) y / (double) rad_y));*/
		setpixel(handle, red, green, blue, cent_y - y, cent_x - (int) x);
		setpixel(handle, red, green, blue, cent_y - y, cent_x + (int) x);
		setpixel(handle, red, green, blue, cent_y + y, cent_x - (int) x);
		setpixel(handle, red, green, blue, cent_y + y, cent_x + (int) x);
		}
	}

void triangle(IMAGE (*handle), int x1, int y1, int x2, int y2, int x3, int y3, unsigned char red, unsigned char green, unsigned char blue)
	{
	register int i;
	double step1, step2, step1t, step2t;
	if (x1 == x2 && x2 == x3 && y1 == y2 && y2 == y3)
		{
		setpixel(handle, red, green, blue, x1, y1);
		return;
		}
	if (x1 == x2 && x2 == x3)
		{
		yline(handle, x1, y1, y2, red, green, blue);
		yline(handle ,x2, y1, y3, red, green, blue);
		return;
		}
	if (y1 == y2 && y2 == y3)
		{
		xline(handle, y1, x1, x2, red, green, blue);
		xline(handle, y1, x1, x3, red, green, blue);
		return;
		}
	if (!(x3 == x1 || x2 == x3) && !(y1 == y3 || y2 == y3))
		{
		if (((double) (x3 - x1) /  (double) (y3 - y1)) == ((double) (x3 - x2) / (double) (y3 - y2)))
			{
			line(handle, x1, y1, x2, y2, red, green, blue);
			line(handle, x1, y1, x3, y3, red, green, blue);
			return;
			}
		}
	if (y1 == y2 || y2 == y3 || y1 == y3)
		{
		if (x2 < x1)
			{
			i = x1;
			x1 = x2;
			x2 = i;
			i = y1;
			y1 = y2;
			y2 = i;
			}
		if (x1 > x3)
			{
			i = x1;
			x1 = x3;
			x3 = i;
			i = y1;
			y1 = y3;
			y3 = i;
			}
		if (x2 < x3)
			{
			i = x2;
			x2 = x3;
			x3 = i;
			i = y2;
			y2 = y3;
			y3 = i;
			}
		if (x1 == x2 || x2 == x3 || x3 == x1)
			{
			if (x2 == y2)
				{
				i = x1;
				x1 = x2;
				x2 = i;
				i = y1;
				y1 = y2;
				y2 = i;
				}
			if (x3 == y3)
				{
				i = x1;
				x1 = x3;
				x3 = i;
				i = y1;
				y1 = y3;
				y3 = i;
				}
			if (y2 == y1) y2 = y3;
			if (x2 == x1) x2 = x3;
			if (x2 > x1)
				{
				step1t = (double) y2;
				step1 = (double) (y2 - y1) / (double) (x2 - x1);
				for (i = x1; i <= x2; i++)
					{
					xline(handle, i, y1, (int) step1t, red, green, blue);
					step1t -= step1;
					}
				}
			else
				{
				step1t = (double) y2;
				step1 = (double) (y2 - y1) / (double) (x2 - x1);
				for (i = x1; i <= x2; i--)
					{
					xline(handle, i, y1, (int) step1t, red, green, blue);
					step1t -= step1;
					}
				}
			return;
			}
		step1 = (double) (y3 - y1) / (double) (x3 - x1);
		step2 = (double) (y2 - y1) / (double) (x2 - x1);
		step1t = step2t = (double) y1;
		for (i = x1; i <= x3; i++)
			{
			xline(handle, i, (int) step1t, (int) step2t, red, green, blue);
			step1t += step1;
			step2t += step2;
			}
		step1 = (double) (y2 - y3) / (double) (x2 - x3);
		step1t = (double) y3;
		for (i = x3; i <= x2; i++)
			{
			xline(handle, i, (int) step1t, (int) step2t, red, green, blue);
			step1t += step1;
			step2t += step2;
			}
		}
	else
		{
		if (y2 < y1)
			{
			i = x1;
			x1 = x2;
			x2 = i;
			i = y1;
			y1 = y2;
			y2 = i;
			}
		if (y1 > y3)
			{
			i = x1;
			x1 = x3;
			x3 = i;
			i = y1;
			y1 = y3;
			y3 = i;
			}
		if (y2 < y3)
			{
			i = x2;
			x2 = x3;
			x3 = i;
			i = y2;
			y2 = y3;
			y3 = i;
			}
		step1 = (double) (x3 - x1) / (double) (y3 - y1);
		step2 = (double) (x2 - x1) / (double) (y2 - y1);
		step1t = step2t = (double) x1;
		for (i = y1; i <= y3; i++)
			{
			yline(handle, i, (int) step1t, (int) step2t, red, green, blue);
			step1t += step1;
			step2t += step2;
			}
		step1 = (double) (x2 - x3) / (double) (y2 - y3);
		step1t = (double) x3;
		for (i = y3; i <= y2; i++)
			{
			yline(handle, i, (int) step1t, (int) step2t, red, green, blue);
			step1t += step1;
			step2t += step2;
			}
		}
	}
