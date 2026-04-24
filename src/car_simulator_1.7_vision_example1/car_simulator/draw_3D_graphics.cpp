
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
#include "rotation.h"
#include "graphics.h"
#include "ran.h"

#include "image_transfer7.h"

#include "robot.h"
#include "controller.h"

const double PI = 4*atan(1.0);

#define SQR(x)	((x)*(x))

// dynamic arrays for storing DirectX vertices
double *XB, *YB, *ZB, *RB, *GB, *BB, *UB, *VB;

// maximum number of vertices in the dynamic arrays
int MAX_VERTICES = 100000;

// * default window size (image size will be the same as this)
int WINDOW_WIDTH = 640;
int WINDOW_HEIGHT = 480;

// 1 - use border, 0 - no border
int BORDER_DEFAULT = 1; 

// background colour (R,G,B)
double BACKG[3] = { 0.28, 0.47, 0.63 };

// default min and max viewing distances.
// objects closer than VMIN and farther than VMAX are not drawn (ie cannot be seen).
// note the ratio of VMAX/VMIN should be less than 60,000 for most graphics cards.
double VMIN = 1.0; // units of m (or whatever units you draw your object in)
double VMAX = 10000.0; // units of m

// ambient light color (R,G,B)
double AMBIENT[3] = { 0.3, 0.3, 0.3 };

// flag to turn directional lights on/off (note: there are 3 lights)
int LIGHT[3] = {1, 1, 1}; // 1 - on, 0 - off

// light direction
double DIR1[3] = { -1.0, -1.0, -1.0 }; // light #1
double DIR2[3] = { 1.0, 0.0, 0.0 }; // light #2
double DIR3[3] = { 0.0, 0.0, 1.0 }; // light #3

// light colour (RGB)
double COLOUR1[3] = { 0.7, 0.7, 0.7 }; // light #1
double COLOUR2[3] = { 0.5, 0.5, 0.5 }; // light #2
double COLOUR3[3] = { 0.3, 0.3, 0.3 }; // light #3

// FOV - field of view (PI/4 rad is a typical value)
double FOV = PI/4;

int VIEW_2D = 0; // 2D_view mode flag
// 0 - use 3D view
// 1 - X-Y coordinates
// 2 - X-Z coordinates
// 3 - Y-Z coordinates

// width of the 2D view window in global coordinates (m)
double WIDTH_2D = 20.0;

// height of the 2D view window in global coordinates (m)
double HEIGHT_2D = 15.0;

// make 2D correspond to pixel coord
//double WIDTH_2D = (double)WINDOW_WIDTH;
//double HEIGHT_2D = (double)WINDOW_HEIGHT;

// sync fps with screen refresh rate (0 - no sync, 1 - sync)
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

extern double X0;
extern double Y0;
extern double THETA_0;

extern double u_phi;

robot robot1(X0,Y0,THETA_0);

void draw_car(double x_c, double y_c, double z_c, double yaw_c, double pitch_c,
	double roll_c, double th_front, double th_back, double th_steer);

image b;

void draw_3D_graphics()
{
	static int init = 0; // initialization flag
	static double tc0; // initial clock time
	
	double tc; // current clock time
	double x, y, z;
	double yaw, pitch, roll;
	
	static double th_front = 0.0, th_back = 0.0, th_steer = 0.0;

	static double t_sim = 0.0;
	double dt_sim;	

	// initalization section
	if( !init ) {
		tc0 = high_resolution_time(); // initial clock time (s)

		// * activate the vision library before using it
		activate_vision();

		// * image size must correspond to 3D graphics window size
		b.width  = WINDOW_WIDTH;
		b.height = WINDOW_HEIGHT;
		b.type = RGB_IMAGE;
		allocate_image(b);

		init = 1;
	} // end of initialization section

	// calculate control inputs
	calculate_control_inputs();

	// read clock time
	tc = high_resolution_time() - tc0;
	
	dt_sim = 0.001; // simulation time step

	// real-time simulation of robot
	while( t_sim < tc ) {
		robot1.sim_step(dt_sim);
		
		t_sim += dt_sim;
	}

	th_back = robot1.x[3]; // w
	
	// v = wf * Rw -> x = theta_f * Rw
	// theta_f = x / Rw
	th_front = robot1.x[5] / robot1.Rw; 
	
	// get variables to draw car from simulation
	yaw = robot1.x[6];
	pitch = 0.0;
	roll = 0.0;
	
	x = robot1.x[7];
	y = robot1.x[8];
	z = 0.0;
	
	th_steer = u_phi;
	
	// draw car
	draw_car(x,y,z,yaw,pitch,roll,th_front,th_back,th_steer);
	
	// * you must wait until all objects are drawn in the 
	// 3D graphics window before acquiring the image
	screen_grab((unsigned char *)(b.pdata), 
				(int)(b.width), (int)(b.height));

	// * view captured image in the image_view.exe window
	view_rgb_image(b);

	// * the image_view.exe program should have the same
	// window size (or same aspect ratio at least) as 
	// the 3D graphics window / image

	// * note it's possible to get images for different views,
	// but you would have to alternate views in different
	// calls to draw_3D_graphics() for stereo, 3D vision, etc.
	
	// the 3D graphics would flicker between views, but
	// the captured images would be correct
	
	// the fps would also be lowered for each additional view
	// (ie fps_ave = fps / number_of_views)

}


void draw_car(double x_c, double y_c, double z_c, double yaw_c, double pitch_c,
	double roll_c, double th_front, double th_back, double th_steer)
{
	// note: static is used here so the constructor only gets called 
	// once and also so the mech constructor gets called after
	// winmain starts the graphics library
	
	static mesh body("porsche_body.cmo"); 
	static mesh wheel_l("porsche_wheel_left.cmo");
	static mesh wheel_r("porsche_wheel_right.cmo");	
	static mesh track("half_track.cmo");
	static mesh sky("sky.cmo");

	double yaw, pitch, roll, scale;
	double R1[3+1][3+1], R2[3+1][3+1];
	double R[3+1][3+1], rho[3+1], dP[3+1], P[3+1], T[3+1];
	int i;

	// view parameters
	static double eye_point[3+1], lookat_point[3+1], up_dir[3+1], fov;
	static double eye_point_l[3+1], lookat_point_l[3+1], up_dir_l[3+1];
	
	static int init = 0, view_flag;

	// initialization section
	if( !init ) {		
		view_flag = 1; // view ID number
		fov = 3.14159/4; // field of view
		init = 1;
	}

	// size / scale for the car
	scale = 1.537;
	
	body.Scale = scale;
	wheel_l.Scale = scale;
	wheel_r.Scale = scale;

	// position of car center T in global coord
	T[1] = x_c;
	T[2] = y_c;
	T[3] = z_c;

	// car rotation
	yaw = yaw_c;
	pitch = 0.0;
	roll = 0.0;
	euler_to_rotation(yaw,pitch,roll,R1);
	
	// set different view points depending on key input
	if( KEY('1') || ( view_flag == 1 ) ) {
		eye_point[1] = 0.0; eye_point[2] = 0.0; eye_point[3] = 140.0;
		lookat_point[1] = 0.0; lookat_point[2] = 0.0; lookat_point[3] = 0.0;
		up_dir[1] = 0.0; up_dir[2] = 1.0; up_dir[3] = 0.0;
		view_flag = 1;
	}
	
	// 1st person perspective
	if( KEY('2') || ( view_flag == 2 ) ) {
		
		// set view parameters in local coordinates
		eye_point_l[1] = 0;
		eye_point_l[2] = 0;
		eye_point_l[3] = 3;
		lookat_point_l[1] = 1; // lookat point in the local x-direction
		lookat_point_l[2] = 0;
		lookat_point_l[3] = 3;
		up_dir_l[1] = 0.0;
		up_dir_l[2] = 0.0; 
		up_dir_l[3] = 1.0; // in the local z direction

		// multiply a matrix times a vector, b = A*x (from rotation.h/cpp)
		// calculate b = R1*pl
		multiply_Ax(R1,eye_point_l,eye_point);
		multiply_Ax(R1,lookat_point_l,lookat_point);
		multiply_Ax(R1,up_dir_l,up_dir);

		// add T to R1*pl for eye_point_g and lookat_point_g
		// ie Pg = T + R1*pl
		for(i=1;i<=3;i++) eye_point[i] += T[i];
		for(i=1;i<=3;i++) lookat_point[i] += T[i];

		view_flag = 2;
	}	
	
	// set different view points depending on key input
	if( KEY('3') || ( view_flag == 3 ) ) {
		eye_point[1] = -60.0; eye_point[2] = -100.0; eye_point[3] = 60.0;
		lookat_point[1] = 0.0; lookat_point[2] = 0.0; lookat_point[3] = 0.0;
		up_dir[1] = 0.0; up_dir[2] = 0.0; up_dir[3] = 1.0;
		view_flag = 3;
	}

	// set different view points depending on key input
	if( KEY('4') || ( view_flag == 4 ) ) {
		eye_point[1] = T[1]; eye_point[2] = T[2]; eye_point[3] = T[3] + 30;
		lookat_point[1] = T[1]; lookat_point[2] = T[2]; lookat_point[3] = T[3];
		up_dir[1] = 0.0; up_dir[2] = 1.0; up_dir[3] = 0.0;
		view_flag = 4;
	}

	// set different view points depending on key input
	if( KEY('5') || ( view_flag == 5 ) ) {
		eye_point[1] = T[1]; eye_point[2] = T[2]; eye_point[3] = T[3] + 50;
		lookat_point[1] = T[1]; lookat_point[2] = T[2]; lookat_point[3] = T[3];
		up_dir[1] = 0.0; up_dir[2] = 1.0; up_dir[3] = 0.0;
		view_flag = 5;
	}

	//////////////////////////////////////

	// set view matrices
	set_view(eye_point,lookat_point,up_dir);

	// set an additional light
	// maybe looks better but is slower
//	set_light(1,1.0,-2.0,3.0,1.0,1.0,1.0,1);

	// end of set view point code ////////////////

	// draw car body
	body.draw(T[1],T[2],T[3],yaw,pitch,roll);	

	// draw back wheels ///////////////////////////////////////
	
	// back wheel rotation with respect to body

	yaw = 0.0;
	pitch = th_back;
	roll = 0.0;
	euler_to_rotation(yaw,pitch,roll,R2);

	// calculate overall rotation matrix of the wheel R = R1 * R2
	multiply_AB(R1,R2,R);

	// calculate Euler angles for R
	rotation_to_euler(R,yaw,pitch,roll);

	// rho2 - position of back left wheel in car local coord
	rho[1] = -0.675*scale;
	rho[2] = 0.5*scale;
	rho[3] = -0.3*scale;

	// calculate the wheel center position in global coord
	// P = T1 + R1*T2 = T1 + R1*pho
	multiply_Ax(R1,rho,dP);
	for(i=1;i<=3;i++) P[i] = T[i] + dP[i];

	// draw back left wheel
	wheel_l.draw(P[1],P[2],P[3],yaw,pitch,roll); 

	// rho2 - position of back right wheel in car local coord
	rho[1] = -0.675*scale;
	rho[2] = -0.5*scale;
	rho[3] = -0.3*scale;

	// calculate the wheel center position in global coord
	// P = T1 + R1*T2 = T1 + R1*pho
	multiply_Ax(R1,rho,dP);
	for(i=1;i<=3;i++) P[i] = T[i] + dP[i];

	// draw back right wheel
	wheel_r.draw(P[1],P[2],P[3],yaw,pitch,roll); 

	// draw front wheels //////////////////////////////////////

	// front wheel rotation with respect to body

	yaw = th_steer;
	pitch = th_front;
	roll = 0.0;
	euler_to_rotation(yaw,pitch,roll,R2);

	// calculate overall rotation matrix of the wheel R = R1 * R2
	multiply_AB(R1,R2,R);

	// calculate Euler angles for R
	rotation_to_euler(R,yaw,pitch,roll);

	// rho2 - position of front left wheel in car local coord
	rho[1] = 0.8*scale;
	rho[2] = 0.45*scale;
	rho[3] = -0.3*scale;

	// calculate the wheel center position in global coord
	// P = T1 + R1*T2 = T1 + R1*pho
	multiply_Ax(R1,rho,dP);
	for(i=1;i<=3;i++) P[i] = T[i] + dP[i];

	// draw front left wheel
	wheel_l.draw(P[1],P[2],P[3],yaw,pitch,roll); 

	// rho2 - position of front right wheel in car local coord
	rho[1] = 0.8*scale;
	rho[2] = -0.45*scale;
	rho[3] = -0.3*scale;

	// calculate the wheel center position in global coord
	// P = T1 + R1*T2 = T1 + R1*pho
	multiply_Ax(R1,rho,dP);
	for(i=1;i<=3;i++) P[i] = T[i] + dP[i];

	// draw front right wheel
	wheel_r.draw(P[1],P[2],P[3],yaw,pitch,roll); 

//	sky.Scale = 80.0;
//	sky.draw(0.0,0.0,-80.0,0.0,0.0,3.14159/2);

//	track.Scale = 2;
//	track.draw(0.0,0.0,-1.2,0.0,0.0,0.0);

	// draw sky
	sky.Scale = 80.0;
	sky.Roll_0 = 3.14159/2;
	sky.Y_0 = -1.0;
	sky.draw(0.0,0.0,0.0,0.0,0.0,0.0);

	// draw track
	track.Scale = 2;
	track.Z_0 = -0.35;
	track.draw(0.0,0.0,-0.5,0.0,0.0,0.0);

}

