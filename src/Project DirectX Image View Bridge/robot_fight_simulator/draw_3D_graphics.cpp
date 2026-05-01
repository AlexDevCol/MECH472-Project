
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

image b;

void draw_3D_graphics()
{
	static int init = 0;

	if( !init ) {
		activate_vision();

		b.width  = WINDOW_WIDTH;
		b.height = WINDOW_HEIGHT;
		b.type = RGB_IMAGE;
		allocate_image(b);

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
	view_rgb_image(b);
}



