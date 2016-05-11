IFS Fractal Program
===================

This is a program to demonstrate the capabilities of IFS (Iterated
Function Series) using Affine maps. It plots, and outputs as a bitmap
file, a rendition of a 2D affine IFS fractal.

I wrote it while at university in 1997.

See https://www.databasesandlife.com/ifs-fractal-program/ for more information.

Description
-----------

A shape, an IFS fractal, is defined by a number of transforms. Each one
of these transforms map from the whole shape to a smaller self-similar
part of it. In an *affine* IFS fractal, each one of these transforms is
affine.

An affine transform is a linear transform with translation. Examples of
2D affine transforms are rotation about an arbitrary point, reflection
in an arbitrary line, translation by any amount, etc.

So an affine IFS shape can be described by a set of transforms. An
implementation generally has probabilities associated with these.

Implementation
--------------

Handily, the PC shareware program 'fractint' includes a file which
defines several affine IFS images. This is parsed. Each shape is defined
over a number of lines, each line contains the description of the
transform and its associated probability.

The program performs the following steps:

1.  Parse command line (run without parameters to see options)
2.  Parse fractint's IFS file (included) to find this image
3.  Generate the image (see later)
4.  Write the image to a BMP file

Algorithm
---------

Given a point on the object, the point transformed by any of the
transforms will also be on the object. The algorithm works by this
method. Given a point on the object, a transform is chosen according to
the probabilities attached to them and the point is transformed. The new
point is then marked as having been visited. The process is repeated
until some bound on the number of points to be plotted has been reached.
The initial point is chosen at random: after a few transforms the point
will lie on the object. Hence the first couple of points found are not
plotted.

The program
-----------

The source files are:

-   *The files 'bitmap.c' and 'bitmap.h'.* These are BMP writing code,
    written by Robin Salih.
-   *The file 'ifs.c'.* This is the file concerned with the IFS image
    generation. This was written by me, Adrian Smith.

The files 'bitmap.c' and 'ifs.c' should be compiled together using an
ANSI C compiler. Run the resulting executable with no parameters to
receive information on options.
