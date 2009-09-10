/*
	FAST-EDGE
	MIT License
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define PI 3.14159265
#define HIGH_THRESHOLD_PERCENTAGE 0.15 // percentage of pixels that meet the high threshold - for example 0.15 will ensure that at least 15% of edge pixels are considered to meet the high threshold

//#define CLOCK			// uncomment to show running times of image processing functions (in seconds)
//#define ABS_APPROX		// uncomment to use the absolute value approximation of sqrt(Gx ^ 2 + Gy ^2)
//#define PRINT_HISTOGRAM	// uncomment to print the histogram used to estimate the threshold

struct image {
	int width;
	int height;
	unsigned char * pixel_data;
};

void write_pgm_image(struct image * img) 
{
	FILE *fp_out;
	int i = 0;
	if((fp_out =fopen("fast_canny_output.pgm","w"))== 0)
		printf("Error opening output file.");
	fprintf(fp_out, "P5\n#FAST-EDGE\n%d %d\n255\n", img->width, img->height);
	for(i = 0; i < (img->height * img->width); i++)
		fputc(img->pixel_data[i], fp_out);
	fclose(fp_out);
}

int read_pgm_hdr(FILE *fp, int *w, int *h)
{
	char filetype[4];
	int maxval;
	if(skipcomment(fp) == EOF || fscanf(fp, "%2s\n", filetype) != 1 || strcmp(filetype, "P5") || skipcomment(fp) == EOF || fscanf(fp, "%d", w) != 1 || skipcomment(fp) == EOF || fscanf(fp, "%d", h) != 1 || skipcomment(fp) == EOF || fscanf(fp, "%d%*c", &maxval) != 1 || maxval > 255) {
		return(-1);
	} else { 
		return(0);
	}
}

int skipcomment(FILE *fp)
{
	int i;
	if((i = getc(fp)) == '#') {
		while((i = getc(fp)) != '\n' && i != EOF);
	}
	return(ungetc(i, fp));
}
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
	int w, h, x, y, max_x, max_y, n;
	n = 0;
	w = img_in->width;
	h = img_in->height;
	img_out->width = w - 4;
	img_out->height = h - 4;
	max_x = w - 2;
	max_y = w * (h - 2);
	for (y = w * 2; y < max_y; y += w) {
		for (x = 2; x < max_x; x++) {
			img_out->pixel_data[n++] = (2 * img_in->pixel_data[x + y - 2 - w - w] + 
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
	int w, h, x, y, max_x, max_y, n;
	float g_div;
	w = img_in->width;
	h = img_in->height;
	max_x = w - 1;
	max_y = w * (h - 1);
	n = 0;
	for (y = w; y < max_y; y += w) {
		for (x = 1; x < max_x; x++) {
			g_x[n] = (2 * img_in->pixel_data[x + y + 1] 
				+ img_in->pixel_data[x + y - w + 1]
				+ img_in->pixel_data[x + y + w + 1]
				- 2 * img_in->pixel_data[x + y - 1] 
				- img_in->pixel_data[x + y - w - 1]
				- img_in->pixel_data[x + y + w - 1]);
			g_y[n] = 2 * img_in->pixel_data[x + y - w] 
				+ img_in->pixel_data[x + y - w + 1]
				+ img_in->pixel_data[x + y - w - 1]
				- 2 * img_in->pixel_data[x + y + w] 
				- img_in->pixel_data[x + y + w + 1]
				- img_in->pixel_data[x + y + w - 1];
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
	int w, h, x, y, max_x, max_y, s;
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
	i = 0;
	while (histogram[i] == 0) {
		i++;
	}
	*low = (*high + i) / 2;
	#ifdef PRINT_HISTOGRAM
	for (i = 0; i < 256; i++) {
		printf("i %d count %d\n", i, histogram[i]);
	}
	#endif
	
	#ifdef CLOCK
	printf("Estimate threshold - time elapsed: %f\n", ((double)clock() - start) / CLOCKS_PER_SEC);
	#endif
}

int main(int argc, char *argv[])
{
	FILE *fp;
	int w, h, i;
	if ((fp = fopen(argv[1], "r")) == NULL) {
		printf("ERROR: %s can't open %s!", argv[0], argv[1]);
	} else {
		if (read_pgm_hdr(fp, &w, &h) != -1) {
			struct image img;
			printf("*** PGM file recognized, reading data into image struct ***\n");
			img.width = w;
			img.height = h;
			unsigned char *img_data = malloc(w * h * sizeof(char));
			for (i = 0; i < w * h; i++) {
				img_data[i] = fgetc(fp);
			}
			img.pixel_data = img_data;
			struct image img_gauss;
			unsigned char *img_gauss_data = malloc(w * h * sizeof(char));
			img_gauss.pixel_data = img_gauss_data;
			printf("*** image struct initialized ***\n");
			printf("*** performing gaussian noise reduction ***\n");
			gaussian_noise_reduce(&img, &img_gauss);
			int * g_x = malloc(img_gauss.width * img_gauss.height * sizeof(int));
			int * g_y = malloc(img_gauss.width * img_gauss.height * sizeof(int));
			int * g = malloc(img_gauss.width * img_gauss.height * sizeof(int));
			int * dir = malloc(img_gauss.width * img_gauss.height * sizeof(int));
			printf("*** calculating gradient ***\n");
			calc_gradient_sobel(&img_gauss, g_x, g_y, g, dir);
			struct image img_out;
			img_out.width = w - 6;
			img_out.height = h - 6;
			unsigned char *img_out_data = malloc(w * h * sizeof(char));
			img_out.pixel_data = img_out_data;
			printf("*** performing non-maximum suppression ***\n");
			non_max_suppression(&img_out, g, dir);
			
			int high, low;
			estimate_threshold(&img_out, &high, &low);
			
			printf("*** estimated thresholds: hi %d low %d ***\n", high, low);
			/*
			// threshold at high threshold
			for (i = 0; i < img_out.width * img_out.height; i++) {
				if (img_out.pixel_data[i] > high) {
					img_out.pixel_data[i] = 0xFF;
				} else {
					img_out.pixel_data[i] = 0x00;
				}
			}
			*/
			write_pgm_image(&img_out);
		} else {
			printf("ERROR: %s is not a PGM file!", argv[1]);
		}
	}

}
