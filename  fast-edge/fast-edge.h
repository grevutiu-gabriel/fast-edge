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

#ifndef _FASTEDGE
#define _FASTEDGE

void gaussian_noise_reduce(struct image * img_in, struct image * img_out);
void calc_gradient_sobel(struct image * img_in, int g_x[], int g_y[], int g[], int dir[]);
void calc_gradient_scharr(struct image * img_in, int g_x[], int g_y[], int g[], int dir[]);
void non_max_suppression(struct image * img, int g[], int dir[]);
void estimate_threshold(struct image * img, int * high, int * low);

#endif
