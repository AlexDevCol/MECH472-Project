

//STANDARD IMPORTS

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <Windows.h>

using namespace std;


/*
MODULE 1 - VISION

THE VISION PIPELINE

The Big Picture

Every vision program has the same skeleton:
1. Declare and allocate images
2. Load (or capture) the image
3. Process: grey → enhance → threshold → morphology → label
4. Measure: centroid, colour, etc.
5. Display results and save
6. Free memory

The libraries involved: `image_transfer.h` for image lifecycle and I/O, `vision.h` for all processing functions.
*/
#include "Libraries\image_transfer.h"
#include "Libraries\vision.h"

//files .cpp and .h must be in project directory


// 1. Declare and allocate images

image a, b, rgb, rgb0, label;  // declare all images you'll need upfront

// Every image needs .type, .width, .height set BEFORE allocate_image()
rgb.type = RGB_IMAGE;    rgb.width = 640;  rgb.height = 480;
rgb0.type = RGB_IMAGE;    rgb0.width = 640;  rgb0.height = 480;
a.type = GREY_IMAGE;   a.width = 640;  a.height = 480;
b.type = GREY_IMAGE;   b.width = 640;  b.height = 480;
label.type = LABEL_IMAGE;  label.width = 640;  label.height = 480;

// allocate memory for the images
allocate_image(rgb);   allocate_image(rgb0);
allocate_image(a);     allocate_image(b);     allocate_image(label);

activate_vision();




// 2. Load (or capture) the image

load_rgb_image("a.bmp", rgb);
copy(rgb, rgb0);          // SAVE THE ORIGINAL — do this immediately after load

	// Capture

// set camera number (normally 0 or 1)
bool UseCamera = true;
int cam_number = 0;

if (UseCamera == true) {
	activate_camera(cam_number, height, width);
	acquire_image(rgb, cam_number); //a is of type image&
}

if (UseCamera == true) {
	stop_camera(cam_number);
}

/*
	uncomment camera activation,
	replace load_rgb_image(...) with acquire_image(rgb, cam_number),
	optionally loop if you want continuous frames,
	and call stop_camera(cam_number) before shutdown.
*/



// 3. Convert and display intermediate results

// Convert RGB → greyscale
copy(rgb, a);

// To VIEW a greyscale image, convert it back to RGB first (library needs RGB to display) 
// MUST HAVE LAUNCHED image_view.exe before
copy(a, rgb);
view_rgb_image(rgb);
cout << "\ngreyscale"; pause();

// Stretch contrast: if image looks washed out/dark, scale fixes it
scale(a, b);
copy(b, a);               // result is always in b — put it back into a

copy(a, rgb); view_rgb_image(rgb);
cout << "\nafter scale"; pause();

save_rgb_image("grey.bmp", rgb);   // professor often saves intermediate images




// 4. Reduce noise and threshold

lowpass_filter(a, b);
copy(b, a);               // blur smooths sensor noise before thresholding

copy(a, rgb); view_rgb_image(rgb);
cout << "\nafter lowpass"; pause();

// Histogram BEFORE thresholding — helps you choose threshold value
int nhist = 60;
double hist[255], hmin, hmax;
histogram(a, hist, nhist, hmin, hmax);

ofstream fout("hist1.csv");
for (int j = 0; j < nhist; j++) {
	double x = hmin + (hmax - hmin) / nhist * j;
	fout << x << "," << hist[j] << "\n";
}
fout.close();

// Threshold: pixels above 70 → 255 (white), rest → 0 (black)
threshold(a, b, 70);
copy(b, a);

// If objects are dark on light background, invert:
invert(a, b);
copy(b, a);


// 5. Morphological cleanup

// erode removes small noise blobs (shrinks white regions)
erode(a, b);
copy(b, a);

// dialate grows white regions back (fills holes) — used AFTER erode to restore size
// NOTE: A5 Q1 skips dialate here because it introduces background pixels
// that would corrupt average_colour() results
// dialate(a, b); copy(b, a);    // use this only if you need to fill gaps


// 6. Label and measure

// label_image requires BINARY input (values must be exactly 0 or 255)
// It numbers each disconnected white blob: blob 1, blob 2, etc.
label_image(a, label, nlabels);
cout << "\nnlabels = " << nlabels;

// centroid of object #2
double ic, jc;
centroid(a, label, 2, ic, jc);
cout << "\ncentroid: ic=" << ic << " jc=" << jc;

// average colour of object #2 — use rgb0 (original), NOT the processed image
double R_ave, G_ave, B_ave;
average_colour(rgb0, label, 2, R_ave, G_ave, B_ave);


// 7. Display and clean up

copy(a, rgb);   // convert binary result back to RGB for display
int R = 0, G = 0, B = 255;
draw_point_rgb(rgb, (int)ic, (int)jc, R, G, B);   // mark centroid blue

view_rgb_image(rgb);
cout << "\nresult with centroid"; pause();

save_rgb_image("aout.bmp", rgb);

// Always free in the same order you allocated
free_image(a); 
free_image(b); 
free_image(label); 
free_image(rgb); 
free_image(rgb0);
deactivate_vision();







///////////////////////////////////////////////////////////////////////////////////////////////


/*
Module 2 

HSV Colour Space (A7 Q1 walkthrough)

HSV separates colour (Hue) from brightness (Value). The exam question asks you to build a greyscale image where each pixel stores the hue of the original pixel. Then take a histogram of hue values.
Key insight: You call calculate_hue_image(rgb0, a) on the ORIGINAL colour image, NOT the processed one. Then you treat the resulting hue image like any greyscale image (histogram, view, etc.).

*/


// The calculate_HSV function (write this from memory)

void calculate_HSV(int R, int G, int B, double& hue, double& sat, double& value)
{
    int max = R, min = R, delta;

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
}

//  To recover degrees from stored pixel : degrees = pixel_value * 360.0 / 255







///////////////////////////////////////////////////////////////////////////////////////////////


/*
Module 3

Sobel Edge Detection

Sobel measures the image gradient at each pixel — how fast intensity is changing. 
It outputs two images: mag (edge strength) and theta (edge direction). 
The function takes a greyscale image and writes to two output GREY images.

*/


// in main()
// 
// Declare two extra GREY images for output
image mag, theta;
mag.type = GREY_IMAGE;  mag.width = 640;  mag.height = 480;
theta.type = GREY_IMAGE;  theta.width = 640; theta.height = 480;
allocate_image(mag);
allocate_image(theta);

load_rgb_image("a.bmp", rgb);
copy(rgb, a);          // RGB → grey
scale(a, b); copy(b, a);   // enhance contrast

sobel(a, mag, theta);  // run the Sobel filter

copy(mag, rgb); view_rgb_image(rgb); printf("\nsobel mag"); pause();
copy(theta, rgb); view_rgb_image(rgb); printf("\nsobel theta"); pause();


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




///////////////////////////////////////////////////////////////////////////////////////////////


/*
Module 4

Colour Detection & Robot Position

When you know there's exactly one red and one green object, skip labeling. 
Scan all pixels, accumulate the centroid of red pixels and centroid of green pixels separately. Position = midpoint, angle = atan2.

*/


// average_colour 
// Pattern: walk both the RGB pointer(p, 3 bytes / pixel) and the label pointer(pl, 2 bytes / pixel = i2byte) in sync.

void average_colour(image& rgb, image& label_image, int label_num,
	double& R, double& G, double& B)
{
	int k, N = 0;
	ibyte* p = rgb.pdata;
	i2byte* pl = (i2byte*)label_image.pdata;   // cast for 2-byte label pixels

	R = G = B = 0.0;

	for (k = 0; k < rgb.width * rgb.height; k++) {
		if (*pl == label_num) {   // check label first
			N++;
			B += *p;        // byte 0 = Blue (BGR order)
			G += *(p + 1);
			R += *(p + 2);
		}
		p += 3;   // next RGB pixel
		pl++;     // next label pixel (i2byte — pointer advances 2 bytes automatically)
	}

	R /= N;  G /= N;  B /= N;
}





///////////////////////////////////////////////////////////////////////////////////////////////


/*
Module 6

Pointers

These are "what is the output?" questions. Trace mechanically.

*/