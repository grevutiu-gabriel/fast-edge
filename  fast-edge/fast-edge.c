/*
	FAST-EDGE
	Copyright (c) 2009 Benjamin C. Haynor

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
*/

/*
	build commands: gcc -Wall imageio.c fast-edge.c -o fastedge -lm -g -O3 -ffast-math
	
	> Perl uses gcc -g -O -pedantic -Wall -W -Wshadow -Wpointer-arith
> -Wbad-function-cast -Wcast-qual -Wcast-align -Wwrite-strings
> -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Wno-long-long

And FreeBSD's make.conf sample has the following:

# BDECFLAGS are a set of gcc warning settings that Bruce Evans has suggested
# for use in developing FreeBSD and testing changes.  They can be used by
# putting "CFLAGS+=3D${BDECFLAGS}" in /etc/make.conf.  -Wconversion is not
# included here due to compiler bugs, e.g., mkdir()'s mode_t argument.
#
BDECFLAGS=3D	-W -Wall -ansi -pedantic -Wbad-function-cast -Wcast-align \
		-Wcast-qual -Wchar-subscripts -Winline \
		-Wmissing-prototypes -Wnested-externs -Wpointer-arith \
		-Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

*/

/*
STRICT BUILD
gcc -W -Wall -ansi -pedantic -Wbad-function-cast -Wcast-align -Wcast-qual -Wchar-subscripts -Winline -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings imageio.c fast-edge.c example.c -o example -lm -g -O3 -ffast-math
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "imageio.h"
#include "fast-edge.h"

/*
	GAUSSIAN_NOISE_ REDUCE
	apply 5x5 Gaussian convolution filter, shrinks the image by 4 pixels in each direction, using Gaussian filter found here:
	http://en.wikipedia.org/wiki/Canny_edge_detector
*/
void gaussian_noise_reduce(struct image * img_in, struct image * img_out)
{
	#ifdef CLOCK
	clock_t start = clock();
	#endif
	int w, h, x, y, max_x, max_y;
	w = img_in->width;
	h = img_in->height;
	img_out->width = w;
	img_out->height = h;
	max_x = w - 2;
	max_y = w * (h - 2);
	for (y = w * 2; y < max_y; y += w) {
		for (x = 2; x < max_x; x++) {
			img_out->pixel_data[x + y] = (2 * img_in->pixel_data[x + y - 2 - w - w] + 
			4 * img_in->pixel_data[x + y - 1 - w - w] + 
			5 * img_in->pixel_data[x + y - w - w] + 
			4 * img_in->pixel_data[x + y + 1 - w - w] + 
			2 * img_in->pixel_data[x + y + 2 - w - w] + 
			4 * img_in->pixel_data[x + y - 2 - w] + 
			9 * img_in->pixel_data[x + y - 1 - w] + 
			12 * img_in->pixel_data[x + y - w] + 
			9 * img_in->pixel_data[x + y + 1 - w] + 
			4 * img_in->pixel_data[x + y + 2 - w] + 
			5 * img_in->pixel_data[x + y - 2] + 
			12 * img_in->pixel_data[x + y - 1] + 
			15 * img_in->pixel_data[x + y] + 
			12 * img_in->pixel_data[x + y + 1] + 
			5 * img_in->pixel_data[x + y + 2] + 
			4 * img_in->pixel_data[x + y - 2 + w] + 
			9 * img_in->pixel_data[x + y - 1 + w] + 
			12 * img_in->pixel_data[x + y + w] + 
			9 * img_in->pixel_data[x + y + 1 + w] + 
			4 * img_in->pixel_data[x + y + 2 + w] + 
			2 * img_in->pixel_data[x + y - 2 + w + w] + 
			4 * img_in->pixel_data[x + y - 1 + w + w] + 
			5 * img_in->pixel_data[x + y + w + w] + 
			4 * img_in->pixel_data[x + y + 1 + w + w] + 
			2 * img_in->pixel_data[x + y + 2 + w + w]) / 159;
		}
	}
	#ifdef CLOCK
	printf("Gaussian noise reduction - time elapsed: %f\n", ((double)clock() - start) / CLOCKS_PER_SEC);
	#endif
}

/*
	CALC_GRADIENT_SOBEL
	calculates the result of the Sobel operator - http://en.wikipedia.org/wiki/Sobel_operator - and estimates edge direction angle
*/
void calc_gradient_sobel(struct image * img_in, int g_x[], int g_y[], int g[], int dir[]) {//float theta[]) {
	#ifdef CLOCK
	clock_t start = clock();
	#endif
	int w, h, x, y, max_x, max_y;
	float g_div;
	w = img_in->width;
	h = img_in->height;
	max_x = w - 3;
	max_y = w * (h - 3);
	for (y = w * 3; y < max_y; y += w) {
		for (x = 3; x < max_x; x++) {
			g_x[x + y] = (2 * img_in->pixel_data[x + y + 1] 
				+ img_in->pixel_data[x + y - w + 1]
				+ img_in->pixel_data[x + y + w + 1]
				- 2 * img_in->pixel_data[x + y - 1] 
				- img_in->pixel_data[x + y - w - 1]
				- img_in->pixel_data[x + y + w - 1]);
			g_y[x + y] = 2 * img_in->pixel_data[x + y - w] 
				+ img_in->pixel_data[x + y - w + 1]
				+ img_in->pixel_data[x + y - w - 1]
				- 2 * img_in->pixel_data[x + y + w] 
				- img_in->pixel_data[x + y + w + 1]
				- img_in->pixel_data[x + y + w - 1];
			#ifndef ABS_APPROX
			g[x + y] = sqrt(g_x[x + y] * g_x[x + y] + g_y[x + y] * g_y[x + y]);
			#endif
			#ifdef ABS_APPROX
			g[x + y] = abs(g_x[x + y]) + abs(g_y[x + y]);
			#endif
			if (g_x[x + y] == 0) {
				dir[x + y] = 2;
			} else {
				g_div = g_y[x + y] / (float) g_x[x + y];
				/* the following commented-out code is slightly faster than the code that follows, but is a slightly worse approximation for determining the edge direction angle
				if (g_div < 0) {
					if (g_div < -1) {
						dir[n] = 0;
					} else {
						dir[n] = 1;
					}
				} else {
					if (g_div > 1) {
						dir[n] = 0;
					} else {
						dir[n] = 3;
					}
				}
				*/
				if (g_div < 0) {
					if (g_div < -2.41421356237) {
						dir[x + y] = 0;
					} else {
						if (g_div < -0.414213562373) {
							dir[x + y] = 1;
						} else {
							dir[x + y] = 2;
						}
					}
				} else {
					if (g_div > 2.41421356237) {
						dir[x + y] = 0;
					} else {
						if (g_div > 0.414213562373) {
							dir[x + y] = 3;
						} else {
							dir[x + y] = 2;
						}
					}
				}
			}
		}
		
	}	
	#ifdef CLOCK
	printf("Calculate gradient Sobel - time elapsed: %f\n", ((double)clock() - start) / CLOCKS_PER_SEC);
	#endif
}

/*
	CALC_GRADIENT_SCHARR
	calculates the result of the Scharr version of the Sobel operator - http://en.wikipedia.org/wiki/Sobel_operator - and estimates edge direction angle
	may have better rotational symmetry
*/
void calc_gradient_scharr(struct image * img_in, int g_x[], int g_y[], int g[], int dir[]) {//float theta[]) {
	#ifdef CLOCK
	clock_t start = clock();
	#endif
	int w, h, x, y, max_x, max_y, n;
	float g_div;
	w = img_in->width;
	h = img_in->height;
	max_x = w - 1;
	max_y = w * (h - 1);
	n = 0;
	for (y = w; y < max_y; y += w) {
		for (x = 1; x < max_x; x++) {
			g_x[n] = (10 * img_in->pixel_data[x + y + 1] 
				+ 3 * img_in->pixel_data[x + y - w + 1]
				+ 3 * img_in->pixel_data[x + y + w + 1]
				- 10 * img_in->pixel_data[x + y - 1] 
				- 3 * img_in->pixel_data[x + y - w - 1]
				- 3 * img_in->pixel_data[x + y + w - 1]);
			g_y[n] = 10 * img_in->pixel_data[x + y - w] 
				+ 3 * img_in->pixel_data[x + y - w + 1]
				+ 3 * img_in->pixel_data[x + y - w - 1]
				- 10 * img_in->pixel_data[x + y + w] 
				- 3 * img_in->pixel_data[x + y + w + 1]
				- 3 * img_in->pixel_data[x + y + w - 1];
			#ifndef ABS_APPROX
			g[n] = sqrt(g_x[n] * g_x[n] + g_y[n] * g_y[n]);
			#endif
			#ifdef ABS_APPROX
			g[n] = abs(g_x[n]) + abs(g_y[n]);
			#endif
			if (g_x[n] == 0) {
				dir[n] = 2;
			} else {
				g_div = g_y[n] / (float) g_x[n];
				if (g_div < 0) {
					if (g_div < -2.41421356237) {
						dir[n] = 0;
					} else {
						if (g_div < -0.414213562373) {
							dir[n] = 1;
						} else {
							dir[n] = 2;
						}
					}
				} else {
					if (g_div > 2.41421356237) {
						dir[n] = 0;
					} else {
						if (g_div > 0.414213562373) {
							dir[n] = 3;
						} else {
							dir[n] = 2;
						}
					}
				}
			}
			n++;
		}
	}	
	#ifdef CLOCK
	printf("Calculate gradient Scharr - time elapsed: %f\n", ((double)clock() - start) / CLOCKS_PER_SEC);
	#endif
}
/*
	NON_MAX_SUPPRESSION
	using the estimates of the Gx and Gy image gradients and the edge direction angle determines whether the magnitude of the gradient assumes a local  maximum in the gradient direction
	if the rounded edge direction angle is 0 degrees, checks the north and south directions
	if the rounded edge direction angle is 45 degrees, checks the northwest and southeast directions
	if the rounded edge direction angle is 90 degrees, checks the east and west directions
	if the rounded edge direction angle is 135 degrees, checks the northeast and southwest directions
*/
void non_max_suppression(struct image * img, int g[], int dir[]) {//float theta[]) {
	#ifdef CLOCK
	clock_t start = clock();
	#endif
	int w, h, x, y, max_x, max_y;
	w = img->width;
	h = img->height;
	max_x = w;
	max_y = w * h;
	for (y = 0; y < max_y; y += w) {
		for (x = 0; x < max_x; x++) {
			switch (dir[x + y]) {
				case 0:
					if (g[x + y] > g[x + y - w] && g[x + y] > g[x + y + w]) {
						if (g[x + y] > 255) {
						img->pixel_data[x + y] = 0xFF;
						} else {
							img->pixel_data[x + y] = g[x + y];
						}
					} else {
						img->pixel_data[x + y] = 0x00;
					}
					break;
				case 1:
					if (g[x + y] > g[x + y - w - 1] && g[x + y] > g[x + y + w + 1]) {
						if (g[x + y] > 255) {
						img->pixel_data[x + y] = 0xFF;
						} else {
							img->pixel_data[x + y] = g[x + y];
						}
					} else {
						img->pixel_data[x + y] = 0x00;
					}
					break;
				case 2:
					if (g[x + y] > g[x + y - 1] && g[x + y] > g[x + y + 1]) {
						if (g[x + y] > 255) {
						img->pixel_data[x + y] = 0xFF;
						} else {
							img->pixel_data[x + y] = g[x + y];
						}
					} else { 
						img->pixel_data[x + y] = 0x00;
					}
					break;
				case 3:
					if (g[x + y] > g[x + y - w + 1] && g[x + y] > g[x + y + w - 1]) {
						if (g[x + y] > 255) {
						img->pixel_data[x + y] = 0xFF;
						} else {
							img->pixel_data[x + y] = g[x + y];
						}
					} else {
						img->pixel_data[x + y] = 0x00;
					}
					break;
				default:
					printf("ERROR - direction outside range 0 to 3");
					break;
			}
		}
	}
	#ifdef CLOCK
	printf("Non-maximum suppression - time elapsed: %f\n", ((double)clock() - start) / CLOCKS_PER_SEC);
	#endif
}
/*
	ESTIMATE_THRESHOLD
	estimates hysteresis threshold, assuming that the top X% (as defined by the HIGH_THRESHOLD_PERCENTAGE) of edge pixels with the greatest intesity are true edges
	and that the low threshold is equal to the quantity of the high threshold plus the total number of 0s at the low end of the histogram divided by 2
*/
void estimate_threshold(struct image * img, int * high, int * low) {
	#ifdef CLOCK
	clock_t start = clock();
	#endif
	int i, max, pixels, high_cutoff;
	int histogram[256];
	max = img->width * img->height;
	for (i = 0; i < 256; i++) {
		histogram[i] = 0;
	}
	for (i = 0; i < max; i++) {
		histogram[img->pixel_data[i]]++;
	}
	pixels = (max - histogram[0]) * HIGH_THRESHOLD_PERCENTAGE;
	high_cutoff = 0;
	i = 255;
	while (high_cutoff < pixels) {
		high_cutoff += histogram[i];
		i--;
	}
	*high = i;
	i = 1;
	while (histogram[i] == 0) {
		i++;
	}
	*low = (*high + i) * LOW_THRESHOLD_PERCENTAGE;
	#ifdef PRINT_HISTOGRAM
	for (i = 0; i < 256; i++) {
		printf("i %d count %d\n", i, histogram[i]);
	}
	#endif
	
	#ifdef CLOCK
	printf("Estimate threshold - time elapsed: %f\n", ((double)clock() - start) / CLOCKS_PER_SEC);
	#endif
}

void hysteresis (int high, int low, struct image * img, struct image * img_mag)
{
	#ifdef CLOCK
	clock_t start = clock();
	#endif
	int x, y, n, max;
	printf("hysteresis\n");
	max = img->width * img->height;
	for (n = 0; n < max; n++) {
		img->pixel_data[n] = 0x00;
	}
	for (y=0; y < img->height; y++) {
	  for (x=0; x < img->width; x++) {
			if (img_mag->pixel_data[y * img->width + x] >= high) {
				trace (x, y, low, img, img_mag);
			}
		}
	}
	#ifdef CLOCK
	printf("Hysteresis - time elapsed: %f\n", ((double)clock() - start) / CLOCKS_PER_SEC);
	#endif
}

int trace(int x, int y, int low, struct image * img_out, struct image * img_grad)
{
	int y_off, x_off;//, flag;
	if (img_out->pixel_data[y * img_out->width + x] == 0)
	{
		img_out->pixel_data[y * img_out->width + x] = 0xFF;
		//flag = 0;
		for (y_off = -1; y_off <=1; y_off++)
		{
		    for(x_off = -1; x_off <= 1; x_off++)
		    {
				if (!(y == 0 && x_off == 0) && range(img_grad, x + x_off, y + y_off) && img_grad->pixel_data[(y + y_off) * img_out->width + x + x_off] >= low) {
					if (trace(x + x_off, y + y_off, low, img_out, img_grad))
					{
					    return(1);
					}
				}
		    }
		}
		return(1);
	}
	return(0);
}

int range(struct image * img, int x, int y)
{
	if ((x < 0) || (x >= img->width)) {
		return(0);
	}
	if ((y < 0) || (y >= img->height)) {
		return(0);
	}
	return(1);
}

void dilate_1d_h(struct image * img, struct image * img_out) {
	int x, y, offset, y_max;
	y_max = img->height * (img->width - 2);
	for (y = 2 * img->width; y < y_max; y += img->width) {
		for (x = 2; x < img->width - 2; x++) {
			offset = x + y;
			img_out->pixel_data[offset] = max(max(max(max(img->pixel_data[offset-2], img->pixel_data[offset-1]), img->pixel_data[offset]), img->pixel_data[offset+1]), img->pixel_data[offset+2]);	
		}
	}
}

void dilate_1d_v(struct image * img, struct image * img_out) {
	int x, y, offset, y_max;
	y_max = img->height * (img->width - 2);
	for (y = 2 * img->width; y < y_max; y += img->width) {
		for (x = 2; x < img->width - 2; x++) {
			offset = x + y;
			img_out->pixel_data[offset] = max(max(max(max(img->pixel_data[offset-2 * img->width], img->pixel_data[offset-img->width]), img->pixel_data[offset]), img->pixel_data[offset+img->width]), img->pixel_data[offset+2*img->width]);	
		}
	}
}

void erode_1d_h(struct image * img, struct image * img_out) {
	int x, y, offset, y_max;
	y_max = img->height * (img->width - 2);
	for (y = 2 * img->width; y < y_max; y += img->width) {
		for (x = 2; x < img->width - 2; x++) {
			offset = x + y;
			img_out->pixel_data[offset] = min(min(min(min(img->pixel_data[offset-2], img->pixel_data[offset-1]), img->pixel_data[offset]), img->pixel_data[offset+1]), img->pixel_data[offset+2]);	
		}
	}
}

void erode_1d_v(struct image * img, struct image * img_out) {
	int x, y, offset, y_max;
	y_max = img->height * (img->width - 2);
	for (y = 2 * img->width; y < y_max; y += img->width) {
		for (x = 2; x < img->width - 2; x++) {
			offset = x + y;
			img_out->pixel_data[offset] = min(min(min(min(img->pixel_data[offset-2 * img->width], img->pixel_data[offset-img->width]), img->pixel_data[offset]), img->pixel_data[offset+img->width]), img->pixel_data[offset+2*img->width]);	
		}
	}
}

void erode(struct image * img_in, struct image * img_scratch, struct image * img_out) {
	erode_1d_h(img_in, img_scratch);
	erode_1d_v(img_scratch, img_out);
}

void dilate(struct image * img_in, struct image * img_scratch, struct image * img_out) {
	dilate_1d_h(img_in, img_scratch);
	dilate_1d_v(img_scratch, img_out);
}

void morph_open(struct image * img_in, struct image * img_scratch, struct image * img_scratch2, struct image * img_out) {
	erode(img_in, img_scratch, img_scratch2);
	dilate(img_scratch2, img_scratch, img_out);
}

void morph_close(struct image * img_in, struct image * img_scratch, struct image * img_scratch2, struct image * img_out) {
	dilate(img_in, img_scratch, img_scratch2);
	erode(img_scratch2, img_scratch, img_out);
}
