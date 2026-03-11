
#include <iostream>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <Windows.h> // also needed for the 2D graphics library

#include "2D_graphics.h" // use the 2D graphics library
#include "timer.h" // use the time / clock reading function
#include "ran.h"

#include "global_data.h"

#include "world.h"


using namespace std;


//Function Prototypes
//int transform(elemental_form &cf, elemental_form f2);
void title_sequence();
void game_over();


int main()
{	
	int i;
	double t=2.50; //animation time step
	double collision_timeout = 3.0; //intangible time after collision
	double dt, t0;
	double score = 0.0;



	initialize_graphics();
	


	world World(".\\art\\bg2.png");

	
	World.draw();
	World.background_sound->play();

	
	title_sequence();

	t0 = high_resolution_time();
	dt = 0.0;

	while (1) {
		clear();

	
		World.update(score);
		World.draw();

		//Forced reset for testing purposes
		if (KEY('O')) {		
			World.player->y = FloorLevel;
			World.player->init(0);
			for (i = 0; i < 4; i++) {
				World.power_state[i] = 1;
			}
			World.hp = 5;
			for (i = 0; i < 4; i++) {
				World.E[i]->isActive = true;
			}
		}


		if (KEY('Q')) return 0;		//Exit game


		if (dt >= collision_timeout) { //collision timeout after beind hit
			World.activeCollision = true;
			t0 = high_resolution_time();
			dt=0.0;
		}

		
		if (World.ui->HP == 0) { //Die if health at 0;
			World.game_over_sound->play();
			game_over();
			return 1;
		}

		update();

		dt = high_resolution_time() - t0;
	}

	return 0;
}


void title_sequence() {
	int id_title, id_intro;
	double x, y;

	x = WindowWidth/2.0;
	y = 300;
	
	create_sprite(".\\art\\title.png", id_title);
	create_sprite(".\\art\\intro.png", id_intro);

	draw_sprite(id_title, x, y ,0,0.75);
	draw_sprite(id_intro, x, y - 225, 0, 0.40);

	update();

	while (1) {
		if (KEY('N'))break;
	}
}

void game_over() {
	int gameOver_id;
	double x, y;

	x = WindowWidth / 2.0;
	y = 300;

	create_sprite(".\\art\\game_over.png", gameOver_id);
	Sleep(500);
	draw_sprite(gameOver_id, x, y, 0, 0.5);

	update();

	while (1) {
		if (KEY('R'))break;
	}
}


