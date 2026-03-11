
// File by Luis Alejandro Gonzalez///////////////////////////////////////////////

#include <iostream>
#include <cstdio>
#include <cmath>
#include <fstream>
#include "2D_graphics.h" // use the 2D graphics library
#include <Windows.h>
#include "timer.h"

#include "elemental_form.h"

#include "global_data.h"
//#include "platform.h"

using namespace std;

void elemental_form::power() {
}

elemental_form::elemental_form(int m, double X) {
	this->x = X;
	this->y = FloorLevel;
	this->scale = CharacterScale;

	Speed = 2.0;
	Vy = 0.0;
	
	create_sprite(".\\art\\normal.png", id_normal);
	create_sprite(".\\art\\earth.png",id_earth);
	create_sprite(".\\art\\water.png", id_water);
	create_sprite(".\\art\\wind.png", id_wind);
	create_sprite(".\\art\\time.png", id_time);

	id = id_normal;
	
	init(m);
}

void elemental_form::init(int m) {
	mode = m;


	if (mode == 1) {				//Earth form
		id = id_earth;
		Gravity = 1.0;
		jumpVel = 5.0;
	}
	else if (mode == 3) {			//wind form
		id = id_wind;
		Gravity = 0.075;
		jumpVel = 7.5;
	}
	else if (mode == 2) {			//Water form
		id = id_water;
		Gravity = 0.1;
		jumpVel = 6.5;
	}
	else if (mode == 4) {			//Time form
		id = id_time;
		Gravity = 0.1;
		jumpVel = 6.5;
	}
	else {							//Regular Jump
		id = id_normal;
		Gravity = 0.1;
		jumpVel = 6.5;		//time to top of jump
	}

	w = 480.0 - 400.0;
	h = 400.0;

	//create_sprite(file_name, id_form);


	//cout << "mode = " << mode<<endl;
	//cout << "g = " << g << endl;
	//cout << "jump_top_time = " << jump_top_time << endl;
}


void elemental_form::Update(double dt){//, platform pl[]){
	double velocity = 0.0;
	double wb, hb;// fp;
	
	//if (KEY('D')) velocity = Speed * dt; // Move Right
	//if (KEY('A')) velocity = -Speed*dt;	// Move Left
	if (KEY('W') && isGrounded == true) {
		isGrounded = false;
		Vy = jumpVel;
	}
	
	wb = w * scale / 2.0;
	hb = h * scale / 2.0;

	bXL = x - wb;
	bXR = x + wb;
	bBottom = y - hb;
	bTop = y + hb;

	Vy -= Gravity * dt;
	//y += Vy * dt;

	if (Vy < 0.0) {
		/*
		if (CheckCollision(pl,fp) == true) {
			Vy = 0.0;
			isGrounded = true;
			y = fp+hb;
		}
		*/
		if (y <= FloorLevel) {
			Vy = 0.0;
			isGrounded = true;
			y = FloorLevel;
		}
		else isGrounded = false;
	}
	x += velocity;
	y += Vy * dt;

}

void elemental_form::Draw(bool box) {
	double cg = y;
	double theta = 0.0;
	draw_sprite(id, x, y, theta, scale);



	if (box == true) {
		double x[5] = { bXL,bXR,bXR,bXL,bXL }, y[5] = {bTop,bTop,bBottom,bBottom,bTop};
		double xcg[2] = { 0,800 }, ycg[2] = { cg,cg };

		line(x, y, 5, 0.0, 0.0, 1.0);
		line(xcg, ycg, 2, 0.0, 0.0, 1.0);
	}
}


fruit::fruit(double x, double y, int mode){
	char* file_name;
	this->mode = mode;
	Scale = FruitScale;
	R = 350/2.0 * Scale;
	this->x = x;
	this->y = y+R;
	isActive = true;


	if (mode == 4) {
		file_name = ".\\art\\time_fruit.png";
	}
	else if (mode == 3) {
		file_name = ".\\art\\wind_fruit.png";
	}
	else if (mode == 2) {
		file_name = ".\\art\\water_fruit.png";
	}
	else{
		file_name = ".\\art\\earth_fruit.png";
	}


	create_sprite(file_name, id);
}

fruit::~fruit() {

}

void fruit::draw() {
	draw_sprite(id, x, y, 0.0, Scale);
}

void fruit::update_f(double offset) {
	x += offset;
}

enemy::enemy(double x, double y, int mode) {
	char* file_name;
	this->mode = mode;
	Scale = EnemyScale;
	R = 350 / 2.0 * Scale;
	this->x = x;
	this->y = y + R;

	isActive = true;


	if (mode == 4) {
		file_name = ".\\art\\time_enemy.png";
	}
	else if (mode == 3) {
		file_name = ".\\art\\wind_enemy.png";
	}
	else if (mode == 2) {
		file_name = ".\\art\\fire_enemy.png";
	}
	else {
		file_name = ".\\art\\earth_enemy.png";
	}


	create_sprite(file_name, id);
}

enemy::~enemy() {

}

void enemy::draw() {
	draw_sprite(id, x, y, 0.0, Scale);
}

void enemy::update_f(double offset) {
	x += offset;
}