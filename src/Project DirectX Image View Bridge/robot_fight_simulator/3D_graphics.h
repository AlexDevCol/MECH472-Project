
// A mesh object can draw a 3D rigid body object stored in a mesh file.
// you can translate and rotate this rigid body.
// this type of graphics is useful for animating machines

// or multiple machines.
class mesh {

	// for internal use
	int Mesh_number;

public:

	// initial transformation (default = 0.0).
	// this allows the object to be given some
	// initial position and orientation.
	double Roll_0, Pitch_0, Yaw_0;
	double X_0, Y_0, Z_0;

	// scaling/size of mesh (default = 1.0).
	// this allows the size of the mesh to be changed.
	// note: scaling occurs around the origin
	// after the initial transformation.
	double Scale;

	// a constructor that loads an mesh file
	mesh(char file_name[]);

	// draw the mesh in the 3D view port
	// with a global translation of (Px,Py,Pz) - in a global frame of ref.
	// a rotation of (yaw,pitch,roll)
	// - yaw is rotation along the compass direction (NSEW)
	// - pitch is the nose rotation up or down
	// - roll is a barell roll
	void draw(double Px, double Py, double Pz, double yaw, double pitch, double roll);

};

///////

// maximum number of mesh models 
const int N_MODEL_MAX = 1000;

// number of lighting effects
const int N_EFFECTS = 2;

// dynamic arrays for drawing DirectX vertices
extern double *XB, *YB, *ZB, *RB, *GB, *BB, *UB, *VB;

// size of the dynamic arrays
extern int MAX_VERTICES;

// maximum number of textures / sprites 
const int N_TEXTURE_MAX = 100;

///////

// check if key c is currently pressed
// note that multiple keys can be pressed at once
#define KEY(c) ( GetAsyncKeyState((int)(c)) & (SHORT)0x8000 )

//////////////// related graphics functions /////////////////

// user defined graphics startup function
int graphics_startup();

// user defined graphics shutdown function
void graphics_shutdown();

void screen_grab(unsigned char *pb, int width, int height);

// draw one frame of animation
void draw_3D_graphics();

void draw_XYZ(double len);
// draw the XYZ Axes 
// len - length of the axes

void set_view(double *eye_point, double *lookat_point, double *up_dir);
// sets the view and perspective matrices
// eye_point[1] - x component of eye location
// eye_point[2] - y component of eye location
// eye_point[3] - z component of eye location
// lookat_point[1] - x component of look at point
// lookat_point[2] - y component of look at point
// lookat_point[3] - z component of look at point
// up_dir[1] - x component of up direction
// up_dir[2] - y component of up direction
// up_dir[3] - z component of up direction


void text_xy(char* str, double xp, double yp, double scale = 1.0);
// place 2D text at pixel location (x,y)
// str - text string
// xp - x pixel location of text
// yp - y pixel location of text
// scale (default = 1.0)

void text_xy(double value, double xp, double yp, double scale = 1.0);
// place 2D text at pixel location (x,y)
// value - number to print to the screen
// xp - x pixel location of text
// yp - y pixel location of text
// scale (default = 1.0)


// void draw_point_list(double *x, double *y, double *z, double *R, double *G, double *B, int n, double ptsize);
// x[n] - 1D array holding x components of the vertices
// y[n] - 1D array holding y components of the vertices
// z[n] - 1D array holding z components of the vertices
// R[n] - 1D array holding Red colour components (0<=R[i]<=1) of the vertices
// G[n] - 1D array holding Green colour components (0<=G[i]<=1) of the vertices
// B[n] - 1D array holding Blue colour components (0<=B[i]<=1) of the vertices
// n - number of vertices in the point list
// ptsize - size of the points in pixels


void draw_line_list(double *x, double *y, double *z, double *R, double *G, double *B, int n);
// x[n] - 1D array holding x components of the vertices
// y[n] - 1D array holding y components of the vertices
// z[n] - 1D array holding z components of the vertices
// R[n] - 1D array holding Red colour components (0<=R[i]<=1) of the vertices
// G[n] - 1D array holding Green colour components (0<=G[i]<=1) of the vertices
// B[n] - 1D array holding Blue colour components (0<=B[i]<=1) of the vertices
// n - number of vertices in the line list

void draw_triangle_list(double *x, double *y, double *z, double *R, double *G, double *B, int n);
// x[n] - array holding x components of the vertices
// y[n] - array holding y components of the vertices
// z[n] - array holding z components of the vertices
// R[n] - array holding red colour components (0<=R<=1) of vertices
// G[n] - array holding green colour components (0<=G<=1) of vertices
// B[n] - array holding blue colour components (0<=B<=1) of vertices
// n - number of vertices in the triangle list

void draw_vertex_list(double *x, double *y, double *z, double *R, double *G, double *B, int n, int flag, int effect);
// define the size of the vertex buffers 
// (maximum number of vertices).
// for very complex objects you might have 
// to increase these values

void view_2D(double x0, double y0, double theta);
// set the view window -- 1st person perspective with origin x0, y0
// and rotation of theta

void create_sprite_2D(char file_name[], int id);

void draw_sprite_2D(int id, double x, double y, double theta, double scale);
// scale < 0 -- stretch to screen
