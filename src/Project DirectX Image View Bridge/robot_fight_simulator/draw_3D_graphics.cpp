
// this example shows how to acquire images in order to apply
// computer vision functions to a simulation with the 
// 3D graphics library (ie the car simulation example)

// changes to the car simulation example have a * marked
// beside them below

// * note that if the 3D graphics window is too large the 
// computer might not be fast enough to peform all the 
// operations required for both 3D graphics and vision

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <windows.h>

// user defined functions

#include "timer.h"
#include "3D_graphics.h"
#include "graphics.h"

#include "image_transfer7.h"

#include "robot.h"
#include "controller.h"
#include "world.h"

const double PI = 4*atan(1.0);

#include "vision.h"

#include "my_functions.h"

void find_robots(int nblobs, double** blob_matrix, double robot_ic[2], double robot_jc[2], double robot_theta[2], bool robot_found[2]);

bool UseCamera = false;

#define SQR(x)	((x)*(x))

// dynamic arrays for storing DirectX vertices
double *XB, *YB, *ZB, *RB, *GB, *BB, *UB, *VB;

// maximum number of vertices in the dynamic arrays
int MAX_VERTICES = 100000;

// Default window size (also used for captured image size).
// 768: width*3 = 2304 bytes/row (256-byte aligned) — avoids screen_grab / image_view streaking vs 720.
int WINDOW_WIDTH = 768;
int WINDOW_HEIGHT = 768;

// 1 - use border, 0 - no border (border lets you move/resize the window)
int BORDER_DEFAULT = 1;

// background colour (R,G,B)
double BACKG[3] = { 0.1, 0.1, 0.1 };

// default min and max viewing distances.
// objects closer than VMIN and farther than VMAX are not drawn (ie cannot be seen).
// note the ratio of VMAX/VMIN should be less than 60,000 for most graphics cards.
double VMIN = 1.0; // units of m (or whatever units you draw your object in)
double VMAX = 10000.0; // units of m

// ambient light color (R,G,B)
double AMBIENT[3] = { 1.0, 1.0, 1.0 };

// flag to turn directional lights on/off (note: there are 3 lights)
int LIGHT[3] = {0, 0, 0}; // 1 - on, 0 - off

// light direction
double DIR1[3] = { 0.0, 0.0, -1.0 }; // light #1
double DIR2[3] = { 0.0, 0.0, -1.0 }; // light #2
double DIR3[3] = { 0.0, 0.0, -1.0 }; // light #3

// light colour (RGB)
double COLOUR1[3] = { 0.0, 0.0, 0.0 }; // light #1
double COLOUR2[3] = { 0.0, 0.0, 0.0 }; // light #2
double COLOUR3[3] = { 0.0, 0.0, 0.0 }; // light #3

// FOV - field of view (PI/4 rad is a typical value)
double FOV = PI/4;

int VIEW_2D = 1; // 2D_view mode flag
// 0 - use 3D view
// 1 - X-Y coordinates
// 2 - X-Z coordinates
// 3 - Y-Z coordinates

// 2D orthographic span must match the swap chain in *pixel units* so sprites (draw_sprite_2D)
// and line primitives share one space. Simulation inches → pixels via world::CameraToScreen.
double WIDTH_2D = (double)WINDOW_WIDTH;

double HEIGHT_2D = (double)WINDOW_HEIGHT;

// sync fps with screen refresh rate (0 - no sync, 1 - sync)
// Match car vision example: 0 avoids tear/partial frames during screen_grab.
int SYNC_FPS_WITH_SCREEN_REFRESH = 0;

// * setting SYNC_FPS_WITH_SCREEN_REFRESH = 1 can often give
// better / smoother performance with screen_grab function

// 1 - on, 0 - off
// note: don't turn this setting on with SYNC_FPS
int SWAP_EFFECT_FLIP = 0;

extern double X0_VIEW;
extern double Y0_VIEW;
extern double Z0_VIEW;
extern double THETA_VIEW;

using namespace std;

static ofstream fout("timing.txt");

world* pWorld = 0;

int nblobs;
image b, rgb0, label; // declare some image structures
image grey_image;

void draw_3D_graphics()
{
	static int init = 0;

	if( !init ) {
		activate_vision();

		b.width  = WINDOW_WIDTH;
		b.height = WINDOW_HEIGHT;
		b.type = RGB_IMAGE;
		allocate_image(b);
		rgb0.width = WINDOW_WIDTH;
		rgb0.height = WINDOW_HEIGHT;
		rgb0.type = RGB_IMAGE;
		allocate_image(rgb0);
		label.width = WINDOW_WIDTH;
		label.height = WINDOW_HEIGHT;
		label.type = LABEL_IMAGE;
		allocate_image(label);
		grey_image.width = WINDOW_WIDTH;
		grey_image.height = WINDOW_HEIGHT;
		grey_image.type = GREY_IMAGE;
		allocate_image(grey_image);

		// Robot sprites (BMPs copied to OutDir by post-build step)
		create_sprite_2D("robot_A.bmp", 0);
		create_sprite_2D("robot_B.bmp", 1);

		pWorld = new world();
		init = 1;
	}

	calculate_control_inputs();

	// Center camera on the battlefield (72\" × 72\") — in pixel coords that's the window center.
	view_2D(0.5 * (double)WINDOW_WIDTH, 0.5 * (double)WINDOW_HEIGHT, 0.0);

	if (pWorld) {
		pWorld->Update(0.0);
		pWorld->Draw();
	}

	screen_grab((unsigned char *)(b.pdata),
		(int)(b.width), (int)(b.height));

	copy(b, rgb0);

    nblobs = process_image(b, label);

    copy(rgb0, b);



    /////////////////////////////////////////////// Find Robots ///////////////////////////////////////////////////////
    // Allocate memory for blob analysis
    double** blob_matrix = new double* [nblobs];
    for (int i = 0; i < nblobs; i++) {
        blob_matrix[i] = new double[5];
    }


    // Find/Process average color and display centorid of that color
    double R_ave, G_ave, B_ave;
    double ic, jc;
    copy(b, grey_image);

    for (int i = 1; i <= nblobs; i++) {
        // use the original colour image rgb0 here
        centroid(grey_image, label, i, ic, jc);
        average_colour(rgb0, label, i, R_ave, G_ave, B_ave);

        cout << "\nBlob #" << i;
        cout << "\nR_ave = " << R_ave;
        cout << "\nG_ave = " << G_ave;
        cout << "\nB_ave = " << B_ave;

        // mark the centroid point on the image with a colored point
        draw_point_rgb(b, (int)ic, (int)jc, R_ave, G_ave, B_ave);

        // Store in the matrix (remember matrix index is 0-based, blob loop is 1-based)
        blob_matrix[i - 1][0] = ic;
        blob_matrix[i - 1][1] = jc;
        blob_matrix[i - 1][2] = R_ave;
        blob_matrix[i - 1][3] = G_ave;
        blob_matrix[i - 1][4] = B_ave;
    }

    // Prepare Output arrays
    double robot_centers_i[2];
    double robot_centers_j[2];
    double robot_angles[2];
    bool robot_status[2];

    find_robots(nblobs, blob_matrix, robot_centers_i, robot_centers_j, robot_angles, robot_status);

    if (robot_status[0]) {
        cout << "\nRed-Green Robot: (" << robot_centers_i[0] << ", " << robot_centers_j[0] << ") at " << robot_angles[0] << " deg";
    }
    draw_point_rgb(b, (int)robot_centers_i[0], (int)robot_centers_j[0], 255, 182, 193);
    if (robot_status[1]) {
        cout << "\nBlue-Orange Robot: (" << robot_centers_i[1] << ", " << robot_centers_j[1] << ") at " << robot_angles[1] << " deg";
    }
    draw_point_rgb(b, (int)robot_centers_i[1], (int)robot_centers_j[1], 255, 182, 193);


    // Must delete the columns first, then the rows!
    for (int i = 0; i < nblobs; i++) {
        delete[] blob_matrix[i];
    }
    delete[] blob_matrix;

    cout << "\nAverage color centroids displayed";

    /////////////////////////////////////////////// Find Robots ///////////////////////////////////////////////////////

	view_rgb_image(b);
}



// Function to find robots using a 2D dynamic matrix.
// Matrix structure per row: [0] = ic, [1] = jc, [2] = R, [3] = G, [4] = B
void find_robots(int nblobs, double** blob_matrix, double robot_ic[2], double robot_jc[2], double robot_theta[2], bool robot_found[2])
{
    robot_found[0] = false;
    robot_found[1] = false;

    double red_i = -1, red_j = -1;
    double green_i = -1, green_j = -1;
    double blue_i = -1, blue_j = -1;
    double orange_i = -1, orange_j = -1;

    double hue, sat, value;

    // 1. Loop through the rows of the matrix
    for (int k = 0; k < nblobs; k++) {
        // Extract data from the matrix for readability
        double ic = blob_matrix[k][0];
        double jc = blob_matrix[k][1];
        int R = (int)blob_matrix[k][2];
        int G = (int)blob_matrix[k][3];
        int B = (int)blob_matrix[k][4];

        // Convert to HSV
        calculate_HSV(R, G, B, hue, sat, value);

        /*
        Red: hue > 340 OR hue < 20
        Orange: hue > 20 AND hue < 45
        Green: hue > 90 AND hue < 170
        Blue: hue > 200 AND hue < 260
        */

        if (sat > 0.3) { // Discard grey marks
            // Classify by Hue Ranges
            if (hue < 20 || hue > 340) {
                red_i = ic; red_j = jc;
                cout << "\nFound red centroid!";
            }
            else if (hue >= 20 && hue <= 60) {
                orange_i = ic; orange_j = jc;
                cout << "\nFound orange centroid!";
            }
            else if (hue >= 90 && hue <= 170) {
                green_i = ic; green_j = jc;
                cout << "\nFound green centroid!";
            }
            else if (hue >= 200 && hue <= 260) {
                blue_i = ic; blue_j = jc;
                cout << "\nFound blue centroid!";
            }
        }
    }

    // 2. Robot 0 (Red/Green)
    if (red_i != -1 && green_i != -1) {
        robot_found[0] = true;
        robot_ic[0] = (red_i + green_i) / 2.0;
        robot_jc[0] = (red_j + green_j) / 2.0;
        robot_theta[0] = atan2(green_j - red_j, green_i - red_i) * (180.0 / 3.14159);
    }

    // 3. Robot 1 (Blue/Orange)
    if (blue_i != -1 && orange_i != -1) {
        robot_found[1] = true;
        robot_ic[1] = (blue_i + orange_i) / 2.0;
        robot_jc[1] = (blue_j + orange_j) / 2.0;
        robot_theta[1] = atan2(orange_j - blue_j, orange_i - blue_i) * (180.0 / 3.14159);
    }
}


