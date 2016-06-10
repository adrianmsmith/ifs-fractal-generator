/* ifs.c */

/* 2D IFS generation program.  Run with no parameters to recieve help.
 * Adrian M Smith (ams97r@ecs.soton.ac.uk), Dec 1997
 * Version 1.0	Original
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "bitmap.h"

#define DEFXSIZE 100
#define DEFYSIZE 100
#define DEFOUTPUT "output.bmp"
#define DEFDEFS "fractint.ifs"
#define DEFSKIP 10
#define DEFPOINTS 1000

#define GAMMA 0.4		/* gamma corrects image for greater contrast */
#define MAXCHANCE 100000	/* all probabilities are multiplied by this */
#define BUFLEN 1024		/* for IFS file reading */
#define POSMAX 12.0		/* value range is [0, POSMAX] x [0, POSMAX] */

#ifdef __sun__
  #define RAND_MAX 0xFFFFFFFFU
#endif

struct D2affine			/* 2D affine transform: do */
{
   double a, b;			/*         (a b 0)		*/
   double c, d;			/*  (x y 1)(c d 0) = (x' y' 1)	*/
   double e, f;			/*         (e f 1)		*/
} ;

struct ifs_component
{
   struct D2affine transform;
   int chance;			/* 0 <= chance < MAXCHANCE */
} ;

struct image
{
   unsigned short *data;
   int max;			/* max value of any datum in 'data' */
   struct box {
      int x0, y0, x1, y1;	/* minima are inclusive, */
   } bound;			/*   maxima are exclusive.  x0==-1 => undef. */
} ;

struct params
{
   struct output {
      struct image image;
      char *fname;
   } output;
   struct ifs {
      char *ifsfile;
      char *ifsname;
      struct ifs_component *components;
      int ncomponents;
   } ifs;
   struct render {
      int xsize, ysize;
      int nskip, npoints;
   } render;
} ;

char *progname;

void error(char *mess)
{
   fprintf(stderr, "%s: %s\n", progname, mess);
   exit (1);
}

/* params - takes a set of arguments, argc and argv.
 * writes the results to the parameter structure 'p'
 */

void help(void)
{
   printf(
     "2D IFS 1.0, Adrian Smith, 1997\n"
     "Parameters:\n"
     "  Size of image          -size    100x100\n"
     "  Filename to write to   -output  output.bmp\n"
     "  File containing defs   -defs    fractint.ifs\n"
     "  Name of IFS            -ifs     fern\n"
     "  No. of points to skip  -skip    10\n"
     "  No. of points to calc  -points  1000\n"
     "Note: Image is cropped\n"
   );
}

void params(int c, char **v, struct params *p)
{
   /* 1. Initialise parameter structure with defaults */

   p->output.image.data = 0;
   p->output.fname = DEFOUTPUT;
   p->ifs.ifsfile = DEFDEFS;
   p->ifs.ifsname = 0;
   p->ifs.ncomponents = 0;
   p->render.xsize = DEFXSIZE;
   p->render.ysize = DEFYSIZE;
   p->render.nskip = DEFSKIP;
   p->render.npoints = DEFPOINTS;

   /* 2. Move data from parameters (v,c) into structure (p) */

   for (; c >= 2; c -= 2, v += 2)
   {
      if (strcmp(*v, "-size") == 0) {
	 int n = sscanf(v[1], "%d x%d", &p->render.xsize, &p->render.ysize);
	 if (n != 2) error("Parameter -size must be of the form 'NxN'");
      } else if (strcmp(*v, "-output") == 0) p->output.fname = v[1];
      else if (strcmp(*v, "-defs") == 0) p->ifs.ifsfile = v[1];
      else if (strcmp(*v, "-ifs") == 0) p->ifs.ifsname = v[1];
      else if (strcmp(*v, "-skip") == 0) p->render.nskip = atoi(v[1]);
      else if (strcmp(*v, "-points") == 0) p->render.npoints = atoi(v[1]);
      else error("Could not understand parameter");
   }
   if (c) error("Junk on end of command line");

   /* 3. Make validity checks of input */

   if (!p->ifs.ifsname) error("Parameter -ifs is compulsory");
}

/* ifsfile - reads in an IFS file.  File must be specified in ifs
 * structure.  The parser is a state machine, having three states.
 * OUTSIDE is not within any { } structure, INSIDE is inside a IFS
 * description ( {} structure) but not the one we are searching for,
 * and READING is inside the one we were searching for.  When we are in
 * READING, each line is interpreted as an affine transform and a
 * probability.
 */

void readcomponent(char *buf, struct ifs_component *c)
{
   struct D2affine *t = &c->transform;
   double chance;
   int n = sscanf(buf, "%lf%lf%lf%lf%lf%lf%lf", &t->a, &t->c, &t->b, &t->d,
      &t->e, &t->f, &chance);
   if (n != 7) error("Affine transform line doesn't have seven parameters");
   c->chance = chance * MAXCHANCE;
}

void ifsfile(struct ifs *i)
{
   enum { OUTSIDE, INSIDE, READING } state = OUTSIDE;
   FILE *f = fopen(i->ifsfile, "r");
   char buf[BUFLEN];
   int totalchance=0;

   if (!f) error("Could not open IFS file");

   i->components = 0;
   i->ncomponents = 0;

   for (;;)
   {
      char *comment;

      /* read line */
      if (!fgets(buf, BUFLEN, f)) error("Could not find IFS in file");

      /* remove comments */
      comment = strchr(buf, ';');
      if (comment) *comment = 0;

      /* remove triling whitespace */
      while (isspace(*(comment = buf+strlen(buf)-1)) && comment>=buf)
	 *comment = 0;
      if (!*buf) continue;

      /* process line */
      switch (state)
      {
	 case OUTSIDE:
	    if (buf[strlen(buf)-1] == '{')
	       state = strstr(buf, i->ifsname) ? READING : INSIDE;
            break;
         case INSIDE:
	    if (strchr(buf, '}')) state = OUTSIDE;
	    break;
         case READING:
	    if (strchr(buf, '}')) goto done;
	    i->components = realloc(i->components,
	       ++i->ncomponents*sizeof(struct ifs_component));
            if (!i->components) error("Out of memory");
            readcomponent(buf, i->components + i->ncomponents - 1);
	    totalchance += i->components[i->ncomponents-1].chance;
	    break;
      }
   }

done:
   fclose(f);
   i->components[i->ncomponents-1].chance += MAXCHANCE - totalchance;
}

/* calcimage - generates the image */

void initimage(struct image *i, int xsize, int ysize)
{
   int el = xsize * ysize;
   unsigned short *p = i->data = malloc(el * sizeof(unsigned short));
   while (el--) *p++ = 0;
   i->max = 0;
   i->bound.x0 = -1;
}

struct D2affine *choosetransform(struct ifs *i)
{
   int point = rand() / (RAND_MAX / MAXCHANCE + 1);/* point in [0,1] we want */
   int sum = 0;					   /* so far */
   int t;
   
   for (t = 0; t < i->ncomponents; t++)
   {
      sum += i->components[t].chance;
      if (sum > point) return &i->components[t].transform;
   }
   
   error("choosetransform: transform range error");
   return 0;
}

void applyaffine(struct D2affine *t, double *x, double *y)
{
   double newx = t->a * *x + t->c * *y + t->e;
   double newy = t->b * *x + t->d * *y + t->f;
   *x = newx; *y = newy;
}

void plotpixel(struct image *i, int x, int y, int xsize)
{
   const int newval = ++i->data[x + y*xsize];
   if (newval > i->max) i->max = newval;
   if (i->bound.x0 == -1)
   {
       i->bound.x1 = (i->bound.x0 = x) + 1;
       i->bound.y1 = (i->bound.y0 = y) + 1;
   } else {
       if (x < i->bound.x0) i->bound.x0 = x;
       if (y < i->bound.y0) i->bound.y0 = y;
       if (x >= i->bound.x1) i->bound.x1 = x+1;
       if (y >= i->bound.y1) i->bound.y1 = y+1;
   }
}

void calcimage(struct ifs *i, struct image *im, struct render *r)
{
   const int points = r->nskip + r->npoints;
   double x = rand() / (double)RAND_MAX;
   double y = rand() / (double)RAND_MAX;
   int j;

   initimage(im, r->xsize, r->ysize);

   for (j = 0; j < points; j++)
   {
      /* generate new point */
      struct D2affine *t = choosetransform(i);
      applyaffine(t, &x, &y);
      
      /* plot point */
      if (j >= r->nskip)
      {
         static const double base = 0.5/POSMAX;
         int px = (x+POSMAX) * base * r->xsize;
	 int py = (y+POSMAX) * base * r->ysize;
	 
         if (px >= 0 && px < r->xsize && py >= 0 && py < r->ysize)
	    plotpixel(im, px, py, r->xsize);
      }
   }
}

/* write image - Goes through each pixel within boundary and puts it into
 * a BMP structure (see bitmap.h).  Each pixel is gamma corrected; this means
 * that if the pixel has been visited by the IFS procedure x times, and the
 * maximum value is y, then the pixel value is (x/y)**GAMMA in [0,1].
 */

void writeimage(struct output *o, struct render *r)
{
   IMAGE *bmp;
   struct box *b = &o->image.bound;
   int xsize = b->x1 - b->x0;
   int ysize = b->y1 - b->y0;
   int x, y;

   if (xsize % 4) xsize += 4 - xsize%4;		/* hack for bad BMP readers */

   if (!newbitmap(&bmp, o->fname, xsize, ysize, 8))
      error("Could not create bitmap");
   greyscale(bmp);

   for (x = b->x0; x < b->x1; x++)
      for (y = b->y0; y < b->y1; y++)
      {
	 int val = o->image.data[x + y*r->xsize];
	 double normval = (double)val / o->image.max;
	 double realcol = pow(normval, GAMMA);
	 int col = realcol>1.0 ? 255 : realcol<0.0 ? 0 : realcol*255;
	 setpixel(bmp, col, 0, 0, x - b->x0, y - b->y0);
      }

   close_bitmap(bmp);
}

/* main */

int main(int argc, char **argv)
{
   struct params p;
   
   progname = argv[0];
   if (argc == 1) return help(), 0;

   params(argc-1, argv+1, &p);			   /* Read cmd line parms */
   ifsfile(&p.ifs);				   /* Load IFS from file */
   calcimage(&p.ifs, &p.output.image, &p.render);  /* Render IFS to memory */
   writeimage(&p.output, &p.render);		   /* Write image to disk */
   
   puts("IFS rendered and written");
   return 0;
}
