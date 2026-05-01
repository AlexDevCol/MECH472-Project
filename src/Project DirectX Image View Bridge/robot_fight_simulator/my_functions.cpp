
// user defined functions

#include <iostream>
#include <cmath>
#include <fstream>

#include <windows.h>

using namespace std;

// include this header file for basic image transfer functions
#include "image_transfer7.h"

// include this header file for computer vision functions
#include "vision.h"

#include "my_functions.h"

namespace {
constexpr bool kVisionWriteDebugFiles = false;
constexpr bool kVisionVerboseLog = false;
}

int load_image(image& rgb, image& rgb0, bool useCamera, int cam_number) {
	int width = 640;
	int height = 480;
	char image[] = "image.bmp";

	/*
	if (useCamera == true) {
		// Capture
		activate_camera(cam_number, height, width);
		acquire_image(rgb, cam_number); //rgb is of type image&
	}
	else {
	*/
		load_rgb_image(image, rgb);
		cout << "\nImage loaded: " << image << endl;
	//}

	copy(rgb, rgb0);          // SAVE THE ORIGINAL do this immediately after load

	//if (useCamera == true) stop_camera(cam_number);

	return 0;
}


int process_image(image& rgb, image& label, bool draw_ics, bool light_bg, image* segmented_grey_out) {
	int nlabels;
	int R, G, B;

	R = 0;
	G = 0;
	B = 255;

	// 1. Declare and allocate images
	if (kVisionVerboseLog)
		cout << "\n\nProcessing image.......";

	image a, b_work;  // declare all images you'll need upfront

	// Every image needs .type, .width, .height set BEFORE allocate_image()
	a.type = GREY_IMAGE;
	a.width = rgb.width;
	a.height = rgb.height;
	b_work.type = GREY_IMAGE;
	b_work.width = rgb.width;
	b_work.height = rgb.height;

	// allocate memory for the images
	allocate_image(a);
	allocate_image(b_work);


	// 3. Convert and display intermediate results

	// Convert RGB → greyscale
	copy(rgb, a);

	if (kVisionWriteDebugFiles) {
		copy(a, rgb); // RGB wrapper for saving/viewing greyscale
		save_rgb_image("grey.bmp", rgb);
	}

	// Stretch contrast: if image looks washed out/dark, scale fixes it
	scale(a, b_work);
	copy(b_work, a);               // result is always in b_work — put it back into a

	if (kVisionWriteDebugFiles) {
		copy(a, rgb);
		save_rgb_image("scaled_preview.bmp", rgb);
	}

	// 4. Reduce noise and threshold
	if (kVisionVerboseLog)
		cout << "\n\nReducing noise and threshold.......";

	lowpass_filter(a, b_work);
	copy(b_work, a);               // blur smooths sensor noise before thresholding

	if (kVisionWriteDebugFiles) {
		copy(a, rgb);
		save_rgb_image("after_lowpass.bmp", rgb);
	}

	// Histogram BEFORE thresholding — helps you choose threshold value
	int nhist = 60;
	double hist[255], hmin, hmax;
	histogram(a, hist, nhist, hmin, hmax);

	if (kVisionWriteDebugFiles) {
		ofstream fout("hist.csv");
		for (int j = 0; j < nhist; j++) {
			double x = hmin + (hmax - hmin) / nhist * j;
			fout << x << "," << hist[j] << "\n";
		}
	}

	// Threshold: pixels above 70 → 255 (white), rest → 0 (black)
	threshold(a, b_work, 70);
	copy(b_work, a);

	if (kVisionVerboseLog)
		cout << "\nImage after threshold function is applied";

	// If objects are dark on light background, invert:
	if (light_bg == true) {
		invert(a, b_work);
		copy(b_work, a);

		if (kVisionVerboseLog)
			cout << "\nImage after invert function is applied";
	}

	// 6. Label and measure

	// label_image requires BINARY input (values must be exactly 0 or 255)
	// It numbers each disconnected white blob: blob 1, blob 2, etc.
	label_image(a, label, nlabels);
	if (kVisionVerboseLog)
		cout << "\n\nIdentified blobs = " << nlabels;

	if (draw_ics == true) {
		double ic, jc;
		for (int i = 1; i <= nlabels; i++) {
			// compute the centroid of the last object
			centroid(a, label, i, ic, jc);
			if (kVisionVerboseLog)
				cout << "\ncentroid #" << i << ": ic = " << ic << " jc = " << jc;

			// mark the centroid point on the image with a colored point
			draw_point_rgb(rgb, (int)ic, (int)jc, R, G, B);
		}
		if (kVisionVerboseLog)
			cout << "\nAll centroids marked";
	}

	if (kVisionWriteDebugFiles) {
		copy(a, rgb);
		save_rgb_image("image_output.bmp", rgb);
	}

	if (segmented_grey_out != nullptr)
		copy(a, *segmented_grey_out);

	// Always free in the same order you allocated
	free_image(a);
	free_image(b_work);


	return nlabels;
}



/*
Module 2

HSV Colour Space (A7 Q1 walkthrough)

HSV separates colour (Hue) from brightness (Value). The exam question asks you to build a greyscale image where each pixel stores the hue of the original pixel. Then take a histogram of hue values.
Key insight: You call calculate_hue_image(rgb0, a) on the ORIGINAL colour image, NOT the processed one. Then you treat the resulting hue image like any greyscale image (histogram, view, etc.).


Red: hue > 340 OR hue < 20

Orange: hue > 20 AND hue < 45

Green: hue > 90 AND hue < 170

Blue: hue > 200 AND hue < 260

(Pro-tip: We can also check if sat > 0.3 to make sure we aren't accidentally looking at a gray/black pixel that happens to mathematically evaluate to a weird hue).

*/


// The calculate_HSV function (write this from memory)

void calculate_HSV(int R, int G, int B, double& hue, double& sat, double& value)
{
	int max = R, min = R, delta;
	double H;

	// find max and min of R, G, B
	if (G > max) max = G;   if (B > max) max = B;
	if (G < min) min = G;   if (B < min) min = B;

	delta = max - min;

	value = max;   // brightness = max channel

	// saturation: how "pure" is the colour (0 = grey, 1 = fully saturated)
	if (delta == 0) {
		sat = 0.0;
	}
	else {
		sat = delta / value;   // NOTE: value here is max, not 255
	}

	// hue: which colour (0-360 degrees around the colour wheel)
	if (delta == 0) {
		H = 0;   // undefined for grey — set to 0
	}
	else if (max == R) {
		H = (double)(G - B) / delta;        // between yellow and magenta
	}
	else if (max == G) {
		H = (double)(B - R) / delta + 2;    // between cyan and yellow
	}
	else {   // max == B
		H = (double)(R - G) / delta + 4;    // between magenta and cyan
	}

	hue = 60 * H;
	if (hue < 0) hue += 360;   // wrap negative values
}

/*
Mental check — verify with known colours:

Pure red (R=255, G=0, B=0): max=R, H=(0-0)/255=0, hue=0°  ✓
Pure green (R=0, G=255, B=0): max=G, H=(0-0)/255+2=2, hue=120°  ✓
Pure blue (R=0, G=0, B=255): max=B, H=(0-0)/255+4=4, hue=240°  ✓
*/


// The calculate_hue_image function

void calculate_hue_image(image& rgb, image& hue_image)
{
	int k, height = rgb.height, width = rgb.width;
	int nhist;
	double hist[255], hmin, hmax, x;
	ibyte* p = rgb.pdata;      // 3 bytes/pixel source (BGR order!)
	ibyte* ph = hue_image.pdata; // 1 byte/pixel output

	int R, G, B, hint;
	double hue, sat, value;

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			k = j * width + i;

			// BGR order — byte 0=Blue, byte 1=Green, byte 2=Red
			B = p[k * 3];
			G = p[k * 3 + 1];
			R = p[k * 3 + 2];

			calculate_HSV(R, G, B, hue, sat, value);

			// scale hue (0-360°) to greyscale (0-255)
			hint = (int)(hue / 360.0 * 255);
			ph[k] = hint;
		}
	}

	// make a histogram
	nhist = 60; // make 60 bins -- each bin is 255/60 range of intensity
	// eg bin1 = 0-3 
	// bin2 = 4-8,
	// etc.
	histogram(hue_image, hist, nhist, hmin, hmax);

	// *** note when examing the hue image you need to multiply
	// the pixel intensities by 360/255 to get the hue in degrees

	// save to a csv file you can open/plot with microsoft excel
	ofstream hueCsv("hist_hue.csv");
	for (int j = 0; j < nhist; j++) {
		if (j != 0)
			hueCsv << '\n';
		x = hmin + (hmax - hmin) / nhist * j;
		hueCsv << x << " , " << hist[j];
	}
}

//  To recover degrees from stored pixel : degrees = pixel_value * 360.0 / 255



/*
Module 3

Sobel Edge Detection

Sobel measures the image gradient at each pixel — how fast intensity is changing.
It outputs two images: mag (edge strength) and theta (edge direction).
The function takes a greyscale image and writes to two output GREY images.

*/


void display_sobel_images(image& grey_image) {
	// Declare two extra GREY images for output
	image mag, theta, rgb;
	
	rgb.type = RGB_IMAGE;    rgb.width = 640;  rgb.height = 480;
	mag.type = GREY_IMAGE;  mag.width = 640;  mag.height = 480;
	theta.type = GREY_IMAGE;  theta.width = 640; theta.height = 480;
	allocate_image(mag);
	allocate_image(theta);
	allocate_image(rgb);

	sobel(grey_image, mag, theta);  // run the Sobel filter

	copy(mag, rgb); view_rgb_image(rgb); printf("\nsobel mag"); pause();
	copy(theta, rgb); view_rgb_image(rgb); printf("\nsobel theta"); pause();

	free_image(mag);
	free_image(theta);
	free_image(rgb);
}


/*
The sobel() function

The Sobel function processes each interior pixel (skipping the border row/column) using a 3×3 neighbourhood. The neighbourhood layout (memorize this):

pa7  pa8  pa9   ← top row
pa4  pa5  pa6   ← middle row (pa5 = centre pixel)
pa1  pa2  pa3   ← bottom row
*/

int sobel(image& a, image& mag, image& theta)
{
	i4byte size, i, j;
	ibyte* pa, * pa1, * pa2, * pa3, * pa4, * pa5, * pa6, * pa7, * pa8, * pa9;
	ibyte* p_mag, * p_theta;
	i2byte width, height;

	// note we use a signed in here since sx, sy could be < 0
	int sx, sy, M;
	int kx[10], ky[10];
	double A;

	// check for compatibility image sizes and types
	if (a.height != mag.height || a.width != mag.width ||
		a.height != theta.height || a.width != theta.width)
	{
		printf("\nerror in convolution: sizes images are not the same!");
		return 1;
	}

	if (a.type != GREY_IMAGE || mag.type != GREY_IMAGE
		|| theta.type != GREY_IMAGE)
	{
		printf("\nerror in convolution: input types are not valid!");
		return 1;
	}

	width = a.width;
	height = a.height;

	// initialize pointers
	pa = a.pdata + width + 1;
	p_mag = mag.pdata + width + 1;
	p_theta = theta.pdata + width + 1;

	// set neighbourhood pointers

	// make sure they don't point outside of the images at the boundaries
	// when you use them

	// note the order of the neighbourhood is correctly given below
	// as discussed in class (the old order was for a different
	// image coord system in an older version of the library).
	// pa7 pa8 pa9
	// pa4 pa5 pa6
	// pa1 pa2 pa3
	pa1 = pa - width - 1;
	pa2 = pa - width;
	pa3 = pa - width + 1;
	pa4 = pa - 1;
	pa5 = pa;
	pa6 = pa + 1;
	pa7 = pa + width - 1;
	pa8 = pa + width;
	pa9 = pa + width + 1;

	// number of pixels to process
	size = (i4byte)a.width * a.height - 2 * width - 2;

	// set convolution coefficients for sx and sy
	// k7 k8 k9
	// k4 k5 k6
	// k1 k2 k3
	kx[7] = -1; kx[8] = 0; kx[9] = 1;
	kx[4] = -2; kx[5] = 0; kx[6] = 2;
	kx[1] = -1; kx[2] = 0; kx[3] = 1;

	ky[7] = 1;  ky[8] = 2;  ky[9] = 1;
	ky[4] = 0;  ky[5] = 0;  ky[6] = 0;
	ky[1] = -1; ky[2] = -2; ky[3] = -1;

	// calculate sx and sy
	// here I calculate both at the same time in the loop
	// since I don't want to store them into an image array
	// (they can't store negative numbers which might occur for sx
	// and sy) and I need both to calculate mag and theta.
	for (i = 0; i < size; i++) {

		sx = kx[1] * (*pa1) + kx[2] * (*pa2) + kx[3] * (*pa3) +
			kx[4] * (*pa4) + kx[5] * (*pa5) + kx[6] * (*pa6) +
			kx[7] * (*pa7) + kx[8] * (*pa8) + kx[9] * (*pa9);

		sy = ky[1] * (*pa1) + ky[2] * (*pa2) + ky[3] * (*pa3) +
			ky[4] * (*pa4) + ky[5] * (*pa5) + ky[6] * (*pa6) +
			ky[7] * (*pa7) + ky[8] * (*pa8) + ky[9] * (*pa9);

		// might consider directly substituting kx, ky above
		// to reduce computation time

		// calculate mag and theta
		M = abs(sx) + abs(sy); // fast approx of sqrt(sx*sx + sy*sy)

		if (M > 255) M = 255; // check for overflow
		// alternatively M can be scaled by 1/2, 1/4, 1/8
		// to reduce (1/2) or avoid (1/8) possibility of overlow

		*p_mag = M;

		A = atan2((double)sy, (double)sx) / 3.14159 * 180; // deg
		// note that A ranges from -180 to 180 deg

		// scale A so that it ranges from 0 to 255
		// and will fit in a greyscale image range
		// -- add 0.01 to account for roundoff error
		A = (A + 180) / 360 * 255 + 0.01;

		*p_theta = (int)A;

		// note this line might be useful to cut down
		// on the noise / irrelevant info from theta
		if (M < 75) *p_theta = 0;

		// increment pointers
		pa1++; pa2++; pa3++; pa4++; pa5++;
		pa6++; pa7++; pa8++; pa9++;
		p_mag++, p_theta++;
	}

	// copy edges of image from valid regions
	p_mag = mag.pdata;
	p_theta = theta.pdata;

	// number of pixels
	size = (i4byte)a.width * a.height;

	for (i = 0; i < width; i++) {
		p_mag[i] = p_mag[i + width]; // bottom
		p_mag[size - i - 1] = p_mag[size - i - 1 - width]; // top
		p_theta[i] = p_theta[i + width]; // bottom
		p_theta[size - i - 1] = p_theta[size - i - 1 - width]; // top
	}

	for (i = 0, j = 0; i < height; i++, j += width) {
		p_mag[j] = p_mag[j + 1]; // left
		p_mag[size - j - 1] = p_mag[size - j - 2]; // right
		p_theta[j] = p_theta[j + 1]; // left
		p_theta[size - j - 1] = p_theta[size - j - 2]; // right
	}

	return 0;
}



/*
Module 4

Colour Detection & Robot Position
*/

void average_colour(image& rgb, image& label_image, int label_num, double& R, double& G, double& B)
{
	int i, j, k, label, N;
	int height, width; // ints are 4 bytes on the PC
	ibyte* p; // pointer to colour components in the rgb image
	i2byte* pl; // pointer to the label image

	height = rgb.height;
	width = rgb.width;

	p = rgb.pdata;
	pl = (i2byte*)label_image.pdata;

	// initialize the summation varibles to compute average colour
	R = 0.0;
	G = 0.0;
	B = 0.0;
	N = 0; // number of pixels with the label number of interest

	// method #3 -- pointers only !
	for (k = 0; k < width * height; k++) { // loop for kth pixel

		// how to get j and i from k ?
		i = k % width;
		j = (k - i) / width;
		label = *pl;

		// collect data if the pixel has the label of interest
		if (label == label_num) {
			N++;
			// 3 bytes per pixel -- colour in order BGR
			B += *p; // 1st byte in pixel
			G += *(p + 1); // 2nd byte in pixel
			R += *(p + 2); // 3rd
		}

		// increment pointers
		p += 3; // 3 bytes per pixel
		pl++;

	}

	// compute average colour
	R = R / N;
	G = G / N;
	B = B / N;
}

