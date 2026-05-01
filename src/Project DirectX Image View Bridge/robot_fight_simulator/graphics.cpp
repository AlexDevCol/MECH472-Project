
// graphics functions examples

#include <cstdio>  // standard I/O functions
#include <cstring> // string manipulation functions

#include <iostream>  // console stream I/O
#include <fstream>   // file stream I/O
#include <strstream> // string stream I/0
#include <iomanip>   // I/O manipulators

#include <Windows.h>

#include "3D_graphics.h"

#include "rotation.h"
#include "graphics.h"

// maximum buffer size for drawing triangle and textured triangle lists
// -> number of triangles = 63000 / 3 = 21000 (10500 squares -- 100x100 mesh)
const int TB_SIZE = MAX_VERTICES - MAX_VERTICES % 3;

// maximum buffer size for drawing line and point lists
const int LB_SIZE = MAX_VERTICES - MAX_VERTICES % 2;

const double PI = 4*atan(1.0);

void draw_line(double *x, double *y, double *z, double *R, double *G, 
		double *B, int n)
// x[n] - 1D array holding x components of the vertices, i = 0 to n-1
// y[n] - 1D array holding y components of the vertices
// z[n] - 1D array holding z components of the vertices
// R[n] - 1D array holding Red colour components (0<=R[i]<=1) of the vertices
// G[n] - 1D array holding Green colour components (0<=G[i]<=1) of the vertices
// B[n] - 1D array holding Blue colour components (0<=B[i]<=1) of the vertices
// n - number of points in the line
{
	int i; // index of line vertices
	int k; // index of vertex array

	// only draw what is possible
	if(n > LB_SIZE/2) n = LB_SIZE/2;
	
	k = 0;
	for(i=0;i<(n-1);i++) {
		XB[k] = x[i];
		YB[k] = y[i];
		ZB[k] = z[i];
		RB[k] = R[i];
		GB[k] = G[i];
		BB[k] = B[i];		
		k++;
		
		XB[k] = x[i+1];
		YB[k] = y[i+1];
		ZB[k] = z[i+1];
		RB[k] = R[i+1];
		GB[k] = G[i+1];
		BB[k] = B[i+1];	
		k++;
	}
	
	draw_line_list(XB,YB,ZB,RB,GB,BB,k);
	
}


void draw_points(double *x, double *y, double *z, double *R, double *G, 
		double *B, int n)
// x[n] - 1D array holding x components of the vertices, i = 0 to n-1
// y[n] - 1D array holding y components of the vertices
// z[n] - 1D array holding z components of the vertices
// R[n] - 1D array holding Red colour components (0<=R[i]<=1) of the vertices
// G[n] - 1D array holding Green colour components (0<=G[i]<=1) of the vertices
// B[n] - 1D array holding Blue colour components (0<=B[i]<=1) of the vertices
// n - number of points
{
	int ptsize = 3; // size of the points in pixels
	
	// only draw what is possible
	if(n > LB_SIZE) n = LB_SIZE;	
	
//	draw_point_list(x,y,z,R,G,B,n,ptsize);
}


void draw_polygon(double *x, double *y, double *z, double R, double G, 
		double B, int n)
// draw a polygon surface (assumes the vertices are in a 2D plane)
// x[n] - 1D array holding x components of the vertices, i = 0 to n-1
// y[n] - 1D array holding y components of the vertices
// z[n] - 1D array holding z components of the vertices
// n - number of points in the polygon 
// note: if you want the polygon to be closed then set
// x[n-1]=x[0], y[n-1]=y[0], z[n-1]=z[0]
// note the vertices should proceed in the counter clockwise in
// the viewing direction or it won't be visible.
{
	int i, k;
	double xc,yc,zc; // centroid of the polygon
	
	// only draw what is possible (3 vertices/triangle for each point)
	if(n > TB_SIZE/3) n = TB_SIZE/3;	
	
	// calculate centroid
	xc = yc = zc = 0.0;
	for(i=0;i<n;i++) {
		xc += x[i];
		yc += y[i];
		zc += z[i];
	}
	
	xc /= n;
	yc /= n;
	zc /= n;
	
	k = 0;
	for(i=0;i<(n-1);i++) {
		
		// pt 1 - center
		XB[k] = xc;
		YB[k] = yc;
		ZB[k] = zc;
		RB[k] = R;
		GB[k] = G;
		BB[k] = B;		
		k++;		
		
		// pt 2
		XB[k] = x[i];
		YB[k] = y[i];
		ZB[k] = z[i];
		RB[k] = R;
		GB[k] = G;
		BB[k] = B;		
		k++;
		
		// pt 3
		XB[k] = x[i+1];
		YB[k] = y[i+1];
		ZB[k] = z[i+1];
		RB[k] = R;
		GB[k] = G;
		BB[k] = B;	
		k++;	
	}	
	
	draw_triangle_list(XB,YB,ZB,RB,GB,BB,k);
	
}


void draw_circle(double x, double y, double z, int n, double radius, 
		double R, double G, double B, int flag)
// draw a circle centered at (x,y,z) with n line segments and 
// colour (R,G,B), flag: 
// 1 - X-Y coordinates
// 2 - X-Z coordinates
// 3 - Y-Z coordinates
{
	int i, k;
	double dx, dy, dth;
	
	// only draw what is possible
	if(n > LB_SIZE/2) n = LB_SIZE/2;	
	
	dth = 2*PI/n;
	
	k = 0;
	for(i=0;i<n;i++) {
		
		// pt 1
		dx = radius * cos(dth*i);
		dy = radius * sin(dth*i);		
		XB[k] = x + dx;
		YB[k] = y + dy;
		ZB[k] = z;
		RB[k] = R;
		GB[k] = G;
		BB[k] = B;		
		k++;
		
		// pt 2
		dx = radius * cos(dth*(i+1));
		dy = radius * sin(dth*(i+1));
		XB[k] = x + dx;
		YB[k] = y + dy;
		ZB[k] = z;
		RB[k] = R;
		GB[k] = G;
		BB[k] = B;	
		k++;	
	}	
	
	draw_line_list(XB,YB,ZB,RB,GB,BB,k);
}

void draw_disc(double x, double y, double z, double radius, int n,
		double R, double G, double B, int flag)
// draw a disc centered at (x,y,z) with n segments and 
// colour (R,G,B), flag: 
// 1 - X-Y coordinates
// 2 - X-Z coordinates
// 3 - Y-Z coordinates
{
	int i, k;
	double dx, dy, dth;

	// only draw what is possible	
	if(n > TB_SIZE/3) n = TB_SIZE/3;	
	
	dth = 2*PI/n;
	
	k = 0;
	for(i=0;i<n;i++) {
		
		// pt 1 - center
		XB[k] = x;
		YB[k] = y;
		ZB[k] = z;
		RB[k] = R;
		GB[k] = G;
		BB[k] = B;		
		k++;		
		
		// pt 2
		dx = radius * cos(dth*i);
		dy = radius * sin(dth*i);		
		XB[k] = x + dx;
		YB[k] = y + dy;
		ZB[k] = z;
		RB[k] = R;
		GB[k] = G;
		BB[k] = B;		
		k++;
		
		// pt 3
		dx = radius * cos(dth*(i+1));
		dy = radius * sin(dth*(i+1));
		XB[k] = x + dx;
		YB[k] = y + dy;
		ZB[k] = z;
		RB[k] = R;
		GB[k] = G;
		BB[k] = B;	
		k++;	
	}	
	
	draw_triangle_list(XB,YB,ZB,RB,GB,BB,k);
	
}


void draw_rectangle(double x, double y, double z, double theta, 
	double width, double height, double R, double G, double B, 
	int flag)
// draw a rectangle centered at (x,y,z) and angle theta with 
// colour (R,G,B), flag: 
// 1 - X-Y coordinates (width-height)
// 2 - X-Z coordinates (width-height) 
// 3 - Y-Z coordinates (width-height)
{	
	int k;
	double R1[3+1][3+1],rc[3+1],yaw,pitch,roll;

	// order of points:
	// 3 4
	// 1 2
	// --> two counter clockwise sequences
	// 1-2-4 and 1-4-3
	k = 0;
	
	// pt 1		
	XB[k] = x - width/2;
	YB[k] = y - height/2;
	ZB[k] = z;
	RB[k] = R;
	GB[k] = G;
	BB[k] = B;		
	k++;
	
	// pt 2		
	XB[k] = x + width/2;
	YB[k] = y - height/2;
	ZB[k] = z;
	RB[k] = R;
	GB[k] = G;
	BB[k] = B;		
	k++;	
	
	// pt 4		
	XB[k] = x + width/2;
	YB[k] = y + height/2;
	ZB[k] = z;
	RB[k] = R;
	GB[k] = G;
	BB[k] = B;		
	k++;	
	
	// pt 1		
	XB[k] = x - width/2;
	YB[k] = y - height/2;
	ZB[k] = z;
	RB[k] = R;
	GB[k] = G;
	BB[k] = B;		
	k++;	
	
	// pt 4		
	XB[k] = x + width/2;
	YB[k] = y + height/2;
	ZB[k] = z;
	RB[k] = R;
	GB[k] = G;
	BB[k] = B;		
	k++;

	// pt 3	
	XB[k] = x - width/2;
	YB[k] = y + height/2;
	ZB[k] = z;
	RB[k] = R;
	GB[k] = G;
	BB[k] = B;		
	k++;	
	
	// set the translation vector
	rc[1] = x;
	rc[2] = y;
	rc[3] = z;

	yaw = theta;
	pitch = 0;
	roll = 0;
	
	// calculate the rotation matrix
	euler_to_rotation(yaw,pitch,roll,R1);

	// transform the points to global coord
	// -> rotatate an translate the triangles
	rt_transform(R1,rc,XB,YB,ZB,XB,YB,ZB,k);
	
	draw_triangle_list(XB,YB,ZB,RB,GB,BB,k);
}


void colour_map(double x, double y, double z, double width, 
		double height, double **R, double **G, double **B, 
		int ni, int nj, int flag)
// draw a rectangle map centered at (x,y,z) with colour
// at the vertices given by 2D arrays R[i][j], G[i][j], B[i][j]
// flag: 
// 1 - X-Y (i-j) coordinates
// 2 - X-Z (i-j) coordinates
// 3 - Y-Z (i-j) coordinates
{
	int i, j, k;
	int i1, i2, i3, i4, j1, j2, j3, j4;
	double dw, dh, dx, dy, q;
	
	// only draw what is possible	
	k = TB_SIZE/3/2; // maximum number of squares possible
	if( ni*nj > k ) {
		q = (double) k / (ni*nj);
		nj *= (int)q; // shrink the rectangular domain
	}
	
	// order of points:
	// 3 4
	// 1 2
	// --> two counter clockwise sequences
	// 1-2-4 and 1-4-3
	// (i,j) -> (x,y)
	
	dw = width / (ni-1);
	dh = height / (nj-1);
	
	k = 0;	
	for(j=0;j<(nj-1);j++) {
		for(i=0;i<(ni-1);i++) {
			i1 = i;		j1 = j;
			i2 = i+1;	j2 = j;
			i3 = i;		j3 = j+1;
			i4 = i+1;	j4 = j+1;			
		
			// pt 1		
			dx = i1*dw - width/2;
			dy = j1*dh - height/2;	
			XB[k] = x + dx;
			YB[k] = y + dy;
			ZB[k] = z;
			RB[k] = R[i1][j1];
			GB[k] = G[i1][j1];
			BB[k] = B[i1][j1];		
			k++;
	
			// pt 2	
			dx = i2*dw - width/2;
			dy = j2*dh - height/2;	
			XB[k] = x + dx;
			YB[k] = y + dy;
			ZB[k] = z;
			RB[k] = R[i2][j2];
			GB[k] = G[i2][j2];
			BB[k] = B[i2][j2];		
			k++;	
			
			// pt 4	
			dx = i4*dw - width/2;
			dy = j4*dh - height/2;	
			XB[k] = x + dx;
			YB[k] = y + dy;
			ZB[k] = z;
			RB[k] = R[i4][j4];
			GB[k] = G[i4][j4];
			BB[k] = B[i4][j4];		
			k++;	
			
			// pt 1		
			dx = i1*dw - width/2;
			dy = j1*dh - height/2;	
			XB[k] = x + dx;
			YB[k] = y + dy;
			ZB[k] = z;
			RB[k] = R[i1][j1];
			GB[k] = G[i1][j1];
			BB[k] = B[i1][j1];		
			k++;

			// pt 4	
			dx = i4*dw - width/2;
			dy = j4*dh - height/2;	
			XB[k] = x + dx;
			YB[k] = y + dy;
			ZB[k] = z;
			RB[k] = R[i4][j4];
			GB[k] = G[i4][j4];
			BB[k] = B[i4][j4];		
			k++;	
			
			// pt 3	
			dx = i3*dw - width/2;
			dy = j3*dh - height/2;	
			XB[k] = x + dx;
			YB[k] = y + dy;
			ZB[k] = z;
			RB[k] = R[i3][j3];
			GB[k] = G[i3][j3];
			BB[k] = B[i3][j3];		
			k++;
		}
	}	
	
	draw_triangle_list(XB,YB,ZB,RB,GB,BB,k);	
}


void set_view()
// change the eye_point using the keyboard
{
	static int init=0;
	double delta = 0.15;
	static double eye_point[3+1], lookat_point[3+1], up_dir[3+1];
	static double fov;
	int T;

	if(!init) {
		// eye point
		eye_point[1] = 3.0;
		eye_point[2] = 3.0;
		eye_point[3] = 3.0;

		// look at origin
		lookat_point[1] = 0.0;
		lookat_point[2] = 0.0;
		lookat_point[3] = 0.0;

		// up direction
		up_dir[1] = 0.0;
		up_dir[2] = 0.0;
		up_dir[3] = 1.0;

		init = 1;
	} // end of initialization code

	// read the keyboard and change the eye_point accordingly

	// small delay after pressing a key so it doesn't move too fast
	T = 3;
	
	if( KEY('Q') ) {
		eye_point[1] += delta; // x
		Sleep(T);
	}
	
	if( KEY('A') ) {
		eye_point[1] -= delta; // x
		Sleep(T);
	}

	if( KEY('W') ) {
		eye_point[2] += delta; // y
		Sleep(T);		
	}
	
	if( KEY('S') ) {
		eye_point[2] -= delta; // y
		Sleep(T);		
	}

	if( KEY('E') ) {
		eye_point[3] += delta; // z
		Sleep(T);		
	}
	
	if( KEY('D') ) {
		eye_point[3] -= delta; // z
		Sleep(T);		
	}

	if( KEY('R') ) {
		eye_point[1] += delta; // x
		eye_point[2] += delta; // y	
		eye_point[3] += delta; // z
		Sleep(T);		
	}
	
	if( KEY('F') ) {
		eye_point[1] -= delta; // x
		eye_point[2] -= delta; // y
		eye_point[3] -= delta; // z
		Sleep(T);		
	}

	// set view matrices
	set_view(eye_point,lookat_point,up_dir);

	// set an additional light
	// maybe looks better but is slower
//	set_light(1,1.0,-2.0,3.0,1.0,1.0,1.0,1);

}


void draw_plane(double Px, double Py, double Pz, double yaw, double pitch, 
   double roll, double scale)
{
	int i,n;
	double R1[3+1][3+1], rc[3+1];
	
	// construct a list of vertices ///////////////////
	i = 0;
	// point #1
	XB[i] = -1.0*scale; // x
	YB[i] = -1.0*scale; // y
	ZB[i] = 0.01; // z
	RB[i] = 1.0; // red component varies from 0 to 1
	GB[i] = 0.0; // green component varies from 0 to 1
	BB[i] = 1.0; // blue component varies from 0 to 1

	// point #2
	i++;
	XB[i] = 2.0*scale; // x
	YB[i] = 0.0; // y
	ZB[i] = 0.01; // z
	RB[i] = 1.0; // red component varies from 0 to 1
	GB[i] = 1.0; // green component varies from 0 to 1
	BB[i] = 0.0; // blue component varies from 0 to 1

	// point #3
	i++;
	XB[i] = -1.0*scale; // x
	YB[i] = 1.0*scale; // y
	ZB[i] = 0.01; // z
	RB[i] = 1.0; // red component varies from 0 to 1
	GB[i] = 0.0; // green component varies from 0 to 1
	BB[i] = 1.0; // blue component varies from 0 to 1

	// point #4
	i++;
	XB[i] = -1.0*scale; // x
	YB[i] = -1.0*scale; // y
	ZB[i] = 0.0; // z
	RB[i] = 0.0; // red component varies from 0 to 1
	GB[i] = 0.0; // green component varies from 0 to 1
	BB[i] = 1.0; // blue component varies from 0 to 1

	// point #5
	i++;
	XB[i] = -1.0*scale; // x
	YB[i] = 1.0*scale; // y
	ZB[i] = 0.0; // z
	RB[i] = 0.0; // red component varies from 0 to 1
	GB[i] = 0.0; // green component varies from 0 to 1
	BB[i] = 1.0; // blue component varies from 0 to 1

	// point #6
	i++;
	XB[i] = 2.0*scale; // x
	YB[i] = 0.0; // y
	ZB[i] = 0.0; // z
	RB[i] = 0.0; // red component varies from 0 to 1
	GB[i] = 1.0; // green component varies from 0 to 1
	BB[i] = 1.0; // blue component varies from 0 to 1

	n = 6; // set number of verticies

	// set the translation vector
	rc[1] = Px;
	rc[2] = Py;
	rc[3] = Pz;

	// calculate the rotation matrix
	euler_to_rotation(yaw,pitch,roll,R1);	
	
	// transform the points to global coord
	// -> rotatate an translate the triangles
	rt_transform(R1,rc,XB,YB,ZB,XB,YB,ZB,n);
	
	draw_triangle_list(XB,YB,ZB,RB,GB,BB,n);
}


void draw_box(double Px, double Py, double Pz, double yaw, double pitch, double roll,
	double Lx, double Ly, double Lz, double R, double G, double B, int draw_edges)
{
	int n,i;
	double R1[3+1][3+1], rc[3+1];

	// input the 8 vertices for the cube
	const double x1 = 0.0, y1 = 0.0, z1 = 1.0;
	const double x2 = 1.0, y2 = 0.0, z2 = 1.0;
	const double x3 = 0.0, y3 = 1.0, z3 = 1.0;
	const double x4 = 1.0, y4 = 1.0, z4 = 1.0;
	const double x5 = 0.0, y5 = 0.0, z5 = 0.0;
	const double x6 = 1.0, y6 = 0.0, z6 = 0.0;
	const double x7 = 0.0, y7 = 1.0, z7 = 0.0;
	const double x8 = 1.0, y8 = 1.0, z8 = 0.0;

	// construct a vertex list for a cube with size 1
	static double xc[36] = {
	 x1,x2,x4,x1,x4,x3,x2,x6,x8,x2,x8,x4,x3,x4,x8,x3,x8,x7,
	 x1,x6,x2,x1,x5,x6,x5,x8,x6,x5,x7,x8,x1,x7,x5,x1,x3,x7
	};

	static double yc[36] = { 
	 y1,y2,y4,y1,y4,y3,y2,y6,y8,y2,y8,y4,y3,y4,y8,y3,y8,y7,
	 y1,y6,y2,y1,y5,y6,y5,y8,y6,y5,y7,y8,y1,y7,y5,y1,y3,y7
	};

	static double zc[36] = {
	 z1,z2,z4,z1,z4,z3,z2,z6,z8,z2,z8,z4,z3,z4,z8,z3,z8,z7,
	 z1,z6,z2,z1,z5,z6,z5,z8,z6,z5,z7,z8,z1,z7,z5,z1,z3,z7
	};
		
	// vertices for lined edges

	// z = 1
	// 34
    // 12
	// z = 0
    // 78
	// 56

	static double xe[48] = {
		x1,x2,x2,x4,x4,x3,x3,x1,
		x5,x6,x6,x8,x8,x7,x7,x5,
		x2,x4,x4,x8,x8,x6,x6,x2,
		x1,x3,x3,x7,x7,x5,x5,x1,
		x1,x2,x2,x6,x6,x5,x5,x1,
		x3,x4,x4,x8,x8,x7,x7,x3
	};

	static double ye[48] = { 
		y1,y2,y2,y4,y4,y3,y3,y1,
		y5,y6,y6,y8,y8,y7,y7,y5,
		y2,y4,y4,y8,y8,y6,y6,y2,
		y1,y3,y3,y7,y7,y5,y5,y1,
		y1,y2,y2,y6,y6,y5,y5,y1,
		y3,y4,y4,y8,y8,y7,y7,y3
	};

	static double ze[48] = {
		z1,z2,z2,z4,z4,z3,z3,z1,
		z5,z6,z6,z8,z8,z7,z7,z5,
		z2,z4,z4,z8,z8,z6,z6,z2,
		z1,z3,z3,z7,z7,z5,z5,z1,
		z1,z2,z2,z6,z6,z5,z5,z1,
		z3,z4,z4,z8,z8,z7,z7,z3
	};

	// process the vertex list
	n = 36;
	for(i=0;i<n;i++) {
		// make the colour green
		RB[i] = R;
		GB[i] = G;
		BB[i] = B;
		// scale and translate the vertices
		XB[i] = Lx*(xc[i]-0.5);
		YB[i] = Ly*(yc[i]-0.5);
		ZB[i] = Lz*(zc[i]-0.5);
	}

	// set the translation vector
	rc[1] = Px;
	rc[2] = Py;
	rc[3] = Pz;

	// calculate the rotation matrix
	euler_to_rotation(yaw,pitch,roll,R1);

	// transform the points to global coord
	// -> rotatate an translate the triangles
	rt_transform(R1,rc,XB,YB,ZB,XB,YB,ZB,n);
	draw_triangle_list(XB,YB,ZB,RB,GB,BB,n);

	// draw box edges if true
	if( draw_edges ) {
		// process the vertex list
		n = 48;
		for(i=0;i<n;i++) {
			// make the colour green
			RB[i] = 0;
			GB[i] = 0;
			BB[i] = 0;
			// scale and translate the vertices
			XB[i] = Lx*(xe[i]-0.5)*1.005;
			YB[i] = Ly*(ye[i]-0.5)*1.005;
			ZB[i] = Lz*(ze[i]-0.5)*1.005;
		}

		rt_transform(R1,rc,XB,YB,ZB,XB,YB,ZB,n);	
		draw_line_list(XB,YB,ZB,RB,GB,BB,n);
	}

}

