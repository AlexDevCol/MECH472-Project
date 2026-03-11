
#include <iostream>
#include <cstdio>
#include <cmath>
#include <Windows.h> 
#include "2D_graphics.h" 
#include "platform.h"
#include "global_data.h"

using namespace std;

void platform::init(double x, double y, char Psize) {
	
	char* file_name;
	startX = x;
	startY = y;
	size = Psize;
	isActive = true;
	//width = 0.0;

	//width of the platform is determined by size (s/m/l)
	if (size == 's') {
		width = 553;
		file_name = ".\\art\\plat_small.png";
		//file_name = 'plat_small.png';
	}
	else if (size == 'm') {
		width = 585;
		file_name = ".\\art\\plat_med.png";
		//file_name = 'plat_med.png';
	}
	else {
		width = 743;
		file_name = ".\\art\\plat_long.png";
		//file_name = 'plat_long.png';
	}

	height = 131* PlatformScale;
	width *= PlatformScale;

	if (startY <= FloorLevel) {
		double dy = FloorLevel-startY;
		startY += 2.0*dy;
	}

	endX = startX + width;
	endY = startY;

	create_sprite(file_name, id_plat);
	//cout << "Platform type: " << size << endl;
	//cout << "StartX = " << startX << endl;
	//cout << "StartY = " << startY << endl;
}

platform::platform(double x, double y, char Psize) {
	init(x, y, Psize);
}

void platform::print() {

	//cout << "\nstartX = " << startX;
	//cout << "\tstatY =" << startY;
	//cout<<"\tSize = " << Psize;
}

void platform::draw() {
	int nl = 2;	//number of points on the line
	//double xl[2], yl[2];
	double x, y;
	double scale = PlatformScale;

	/*
	xl[0] = startX;
	xl[1] = endX;
	yl[0] = startY;
	yl[1] = endY;

	line(xl, yl, nl, 1, 0, 0); //red line
	*/

	x = ((endX - startX) / 2.0)+startX;
	y = startY - height / 2.0;
	
	draw_sprite(id_plat, x,y, 0, scale);	//assume sprite creation happens before in a diff function

}

void platform::update_pl(double offset) {
	startX += offset;
	endX = startX + width;
}
