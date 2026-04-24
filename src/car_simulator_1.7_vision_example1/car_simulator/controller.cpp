
#include <cmath>   // math functions
#include <cstdio>  // standard I/O functions
#include <cstring> // string manipulation functions

#include <iostream>  // console stream I/O
#include <fstream>   // file stream I/O
#include <strstream> // string stream I/0
#include <iomanip>   // I/O manipulators

#include <windows.h> // for keyboard input

// user defined functions
#include "timer.h" // for measuring time
#include "rotation.h" // for computing rotation functions
#include "3D_graphics.h" // user functions for DirectX 3D graphics

#include "graphics.h"

#include "robot.h"

#include "controller.h"

using namespace std;

const double PI = 4*atan(1.0);

// output file for testing / debugging purposes since 
// console output is not easy in a graphics program
ofstream fout("output.txt");

// initial location of car
// -> you can start the car on any spot of the track
double X0 = -90.0;
double Y0 = 11;
double THETA_0 = 0.0;

extern robot robot1;

// make global since u_phi is needed by the graphics program
// to draw the angle of the front wheels when they steer
double u_s = 0.0, u_phi = 0.0;	

void reset_ICs();

// note that the following views are availble in the simulation
// by pressing the number keys

// 1 - overhead view (3rd person perspective)
// 2 - driver view (1st person perspective)
// 3 - diagonal view (3rd person perspective)
// 4 - close up overhead driver view (1st person perspective)
// 5 - overhead driver view (1st person perspective)

// note calculate_control_inputs() is analogous to loop()
// in an Arduino program -- it get's exectuted as fast as possible.
// -- ie this isn't like a main

void calculate_control_inputs()
{	
	// set state variables and outputs each time in the control loop
	
	double im = robot1.x[1]; // motor current im (A)
	double wm = robot1.x[2]; // motor speed wm (rad/s)
	double theta = robot1.x[3]; // motor angle, theta (rad)	
	double vf = robot1.x[4]; // forward velocity vf (m/s)
	double xf = robot1.x[5]; // forward position xf (m)
	double theta_c = robot1.x[6]; // robot angle theta_c (rad)
	double xc = robot1.x[7]; // robot x position xc (m)
	double yc = robot1.x[8]; // robot y position yc (m)	

	double wb = robot1.y[1]; // back wheel velocity wb (rad/s)
	double wf = robot1.y[2]; // front wheel velocity wf (rad/s)
	double r = robot1.y[3]; // slip ratio r
	double mu = robot1.y[4]; // coefficient of tire friction mu

	double t = robot1.t; // simulation time

	// max steering angle of typical car 
	double phi_max = 35.0/180*3.14159;
	
	// maximum car voltage (speed input) (V)
	double us_max = 12; // 12 V -> 26.3 m/s -> 95 hm/h, 60 mph !	
	
	double ds, dphi;

	static double t_reset = 30;

	static int init = 0;
	
	// initialization section -- gets exectuted once at the beginning
	if( !init ) {
		
		fout << scientific;
		
		init = 1;
	}
	
	// read inputs from the keyboard ////////////
	
	ds = 0.01; // V
	dphi = 0.003; // rad
	
	if( KEY(VK_UP) ) {
		if( u_s < us_max ) u_s += ds;		
	}
		
	if( KEY(VK_DOWN) ) {
		if( u_s > -us_max ) u_s -= ds;
	}
		
	if( KEY(VK_LEFT) ) {
		if( u_phi < phi_max ) u_phi += dphi;
	}

	if( KEY(VK_RIGHT) )	{
		if( u_phi > -phi_max ) u_phi -= dphi;	
	}
	
	// set inputs in the robot model
	robot1.u[1] = u_s; // motor voltage V(t)
	robot1.u[2] = 0.0; // disturbance torque Td(t)
	robot1.u[3] = u_phi; // steering angle phi (rad)
	
	// file output
	// note: too much output might slow down the controller and simulation
	fout << t << "," << xc << "," << yc << "," << u_s << "," << u_phi << "\n";

	// how to periodically reset the ICs
	// -- in case you want to perform some repeated tests, etc.
	if( t > t_reset ) {
		reset_ICs();
		t_reset += 30; // reset 30 seconds later
	}

}


void reset_ICs()
{
	// reset inputs
	
	u_s = 0.0;
	u_phi = 0.0;		
	
	robot1.u[1] = 0.0; // motor voltage V(t)
	robot1.u[2] = 0.0; // disturbance torque Td(t)
	robot1.u[3] = 0.0; // steering angle phi (rad)
	
	// reset states
	
	robot1.x[1] = 0.0; // motor current im (A)
	robot1.x[2] = 0.0; // motor speed wm (rad/s)
	robot1.x[3] = 0.0; // motor angle, theta (rad)	
	robot1.x[4] = 0.0; // forward velocity vf (m/s)
	robot1.x[5] = 0.0; // forward position xf (m)
	robot1.x[6] = THETA_0; // robot angle theta_c (rad)
	robot1.x[7] = X0; // robot x position xc (m)
	robot1.x[8] = Y0; // robot y position yc (m)	
	
	// reset outputs
	
	robot1.y[1] = 0.0; // back wheel velocity wb (rad/s)
	robot1.y[2] = 0.0; // front wheel velocity wf (rad/s)
	robot1.y[3] = 0.0; // slip ratio r
	robot1.y[4] = 0.0; // coefficient of tire friction mu
	
}
