
// robot simulation class

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>

#include "robot.h"

using namespace std;

// sign macro function
#define SIGN(a) ( (a) >= 0.0 ? 1.0 : -1.0 )

robot::robot(double x0, double y0, double theta0)
{
	// model parameters //////////////////
	
	NS = 8; // number of state variables (order)
	MS = 3; // number of inputs
	PS = 4; // number of outputs	

	L = 0.003; ///
	R = 0.141; ///
	// note due to energy conservation kb = km always
	kb = km = 0.00574; /// 
	J = 0.0001; /// this might also include the load inertia (wheel, gears, etc.)
	b = 3.97e-6; ///
	fc = 0.0; /// 0.008 max
		
	// new parameters for traction model
	m = 1.136; /// (kg) total car mass
	Rw = 3.2e-2; /// (m) 3.2 cm tire radius
	g = 9.8; /// g (m/s^2)
	Q = 0.37; /// rear weight distribution
	GR = 2.5; /// gear ratio

	// new parameters for simple car model

	// length between forward and rear wheel axes (m)	
	Lc = 2.27; // wheel base of a Porsche 911 (1994)

	// initial conditions //////////////////
	
	t = 0.0; // initial time
	
	x[1] = 0.0; // initial current, i (A)
	x[2] = 0.0; // initial motor velocity, w (rad/s)
	x[3] = 0.0; // initial angle, theta (rad)	

	// new states for the traction model
	x[4] = 0.0; // initial forward velocity v (m/s)
	x[5] = 0.0; // initial x position (m)

	// new states for simple car model
	x[6] = theta0; // theta_c (rad)
	x[7] = x0; // xc (m)
	x[8] = y0; // yc (m)
	
	// initial control inputs
	u[1] = 0.0; // motor voltage V(t)
	u[2] = 0.0; // disturbance torque Td(t)
	u[3] = 0.0; // steering angle phi (rad)
	
	// default value for outputs
	y[1] = 0.0;
	y[2] = 0.0;
	y[3] = 0.0;
	y[4] = 0.0;
	
}


void robot::sim_step(double dt)
{
	int i;
	
	// parameters for traction model
	double mu, Ft, Fn, wm, v, r, wb, wf;
	// tolerance for slip ratio calculation
	double tol = 1.0e-10;

	// parameters for simple car model
	double u_s, u_phi;	
			
	// 4. calculate the derivative vector xd at time t
		
	Fn = m*g*Q; /// normal force on back wheels
	
	// gear equations ////////
	/// wb = 1 / GR * wm
	/// tau_b = GR * tau_m	
	
	// calculate slip ratio r

	wm = x[2]; /// motor angular velocity, wm (rad/s)
	wb = (1/GR)*wm; /// back wheel angular velocity, wb (rad/s)
	
	v = x[4]; // forward velocity v (m/s)

	// checking for / 0 is not needed because of tol
	r = ( wb * Rw - v ) / ( fabs(v) + tol ); ///
	
	// calculate friction coefficient
	mu = calculate_mu(r); ///
	
	// outputs of interest for plotting
	
	y[1] = wb; // back wheel velocity (rad/s)
	
	// calculate front wheel angular velocity wf
	// v = wf * Rw -> wf = v / Rw
	wf = v / Rw;
	
	y[2] = wf; // front wheel velocity (rad/s) 	
	
	y[3] = r;
	
	y[4] = mu;

	// calculate tire force
	Ft = mu * Fn;
	
	/// tau_b = GR * tau_m -> tau_m = tau_b / GR
	
	// DC motor equations (modified for tire torque Rw*Ft)
	xd[1] = (-x[1]*R - kb*x[2] + u[1])/L; // di/dt
	xd[2] = (km*x[1] - b*x[2] - fc*SIGN(x[2]) - (Rw*Ft)/GR - u[2])/J; /// dw/dt
	xd[3] = x[2]; // dth/dt = w

	// note that combining state variable equation models
	// normally requires exchange / sharing of coupling 
	// terms / variables between both sets of equations
	// -- in this case the tire force Ft
	
	// algebraic constrants may also occur with 
	// resulting coupling forces (lagrange multipliers, etc.)
	// -- requires differential algebraic equation (DAE) solvers 

	// new state-variable equations for the traction model
	xd[4] = Ft / m; // dv/dt
	xd[5] = x[4]; // dx/dt = v

	// new state-variable equations for simple car model

	u_s = v; // forward speed input (m/s)
	u_phi = u[3]; // steering angle input (rad)
	
	// d(theta_c)/dt	
	xd[6] = (u_s / Lc) * tan( u_phi ); 
	
	// d(xc)/dt
	xd[7] = u_s * cos(x[6]);
	
	// d(yc)/dt
	xd[8] = u_s * sin(x[6]);
	
	// 5. apply Euler's equation, x = x + dx, note x is a vector
	// this part is always the same
	// but calculating xd will normally be different
	for(i=1;i<=NS;i++) x[i] = x[i] + xd[i]*dt; 
		
	t = t + dt; // increment time		
}


// magic formula for tire friction

// F = N*D*sin(C*atan(B*r-E*(B*r−atan(B*r))))

// coefficient table from
// https://www.mathworks.com/help/physmod/sdl/ref/tiremagicformula.html
// https://www.mathworks.com/help/physmod/sdl/ref/tireroadinteractionmagicformula.html

// Surface	B		C		D		E
// Dry		10		1.9		1		0.97
// Wet		12		2.3		0.82	1
// Snow		5		2		0.3		1
// Ice		4		2		0.1		1

double calculate_mu(double r)
{
	double mu;
	double B, C, D, E, rmax;
	
//	B=10; C=1.9; D=1; E=0.97; // dry
	B=12; C=2.3; D=0.82; E=1; // wet	
//	B=5; C=2; D=0.3; E=1; // snow
//	B=4; C=2; D=0.1; E=1; // ice
	
	// maximum slip ratio for model -- prevents too much extrapolation
	rmax = 10.0;

	// limit range of r
	if( r > rmax )  r = rmax;
	if( r < -rmax ) r = -rmax;

	mu = D*sin( C*atan( B*r-E*(B*r-atan(B*r)) ) );

	return mu;
}


double calculate_mu_bw(double r)
// BW model
// similar to Burckhardt model but with added term a2 for dip in curve
{
	double mu;
	double a1, a2, a3, c1, c2, rmax;

	// note a2 is mainly for wet road dip in curve

	// the ai parameters can be readily estimated off-line or on-line
	a1 = 1.63; a2 = -0.9; a3 = -0.1; rmax = 1.0; // wet
	
	// rmax is the maximum slip ratio for model -- prevents too much 
	// extrapolation from the a3 term

	// normally c1 and c2 should be constant for different road conditions
	c1 = -27;
	c2 = -15; 

	// limit range of r
	if( r > rmax )  r = rmax;
	if( r < -rmax ) r = -rmax;
	
	if( r < 0 ) {
		r = fabs(r);
		mu = -( a1 * (1-exp(c1*r)) + a2 * (1-exp(c2*r)) + a3 * r );
	} else {
		mu = a1 * (1-exp(c1*r)) + a2 * (1-exp(c2*r)) + a3 * r;	
	}
	
	return mu;
}
