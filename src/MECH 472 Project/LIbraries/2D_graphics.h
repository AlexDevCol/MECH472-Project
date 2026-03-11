
// DirectX 2D graphics window library

// note x, y refer to global coordinates whereas xp, xp refer 
// to local (screen) coordinates.
// x, y in general can be outside the current view -- they can then 
// come into view depending on the view(...) function parameters.

void initialize_graphics();
// initialize the DirectX graphics window
// -- you should run DirectX_window.exe before calling this function

void clear();
// clear the graphics window to start a new drawing / frame

void update();
// update the graphics window with the current drawings
// (ie those made since the last call to the clear() function)

void line(double x[], double y[], int n, double R, double G, double B);
// draw a n vertex (n-1 segment) line
// x[n] - 1D array holding x coordinates of the line
// y[n] - 1D array holding y coordinates of the line
// n - number of vertices for the line
// R, G, B - line colour (0.0 <= R <= 1.0, etc.)

void triangle(double x[], double y[], double R[], double G[], double B[]);
// draw a single triangle given 3 (x,y) coordinates / vertices
// x[3] - 1D array holding x coordinates of the triangle
// y[3] - 1D array holding y coordinates of the triangle
// R[3], G[3], B[3] - colour values for each coordinate of 
// the triangle (0.0 <= R[i] <= 1.0, etc.)

void text(char str[], double xp, double yp, double scale = 1.0);
// draw text at pixel location (xp,yp)
// str - array of characters (C-string) holding the text 
// xp - x pixel location of text
// yp - y pixel location of text
// scale (default = 1.0)

void text(double value, double xp, double yp, double scale = 1.0);
// draw text at pixel location (xp,yp)
// value - number to print to the screen
// xp - x pixel location of text
// yp - y pixel location of text
// scale (default = 1.0)

void view(double x0, double y0, double theta);
// set a 1st person perspective view window with 
// origin x0, y0 and rotation of theta

void create_sprite(char file_name[], int &id);
// create a sprite from a *.png image file and assign it
// a unique identification number (id)

void draw_sprite(int id, double x, double y, double theta, double scale);
// draw a sprite given its identification number (id)
// x - x coordinate of sprite center
// y - y coordinate of sprite center
// theta - angle of rotation (around the image center)
// scale (use scale < 0 to stretch the image to the screen)


#define KEY(c) ( GetAsyncKeyState((int)(c)) & (SHORT)0x8000 )
// check if key c is currently pressed
// note that multiple keys can be pressed at once

