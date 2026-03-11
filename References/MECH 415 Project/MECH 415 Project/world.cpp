
// File by Luis Alejandro Gonzalez///////////////////////////////////////////////

//#define COLLISION // if defined, the collision box for the objects are drawn
#include <iostream>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <Windows.h>

#include "2D_graphics.h" // use the 2D graphics library
#include "timer.h"
#include "ran.h"

#include "world.h"
#include "global_data.h"


// for Playsound 
#include <MMSystem.h>
#pragma comment(lib,"winmm.lib") // links a windows library

using namespace std;


world::world(char* background_file) {
	int i, ground_yn;
	char size;
	double x, y;
	
	//I feel like these should not be memeber variables, I tried to move them as local variables
	//in the world::shuffle(double score) function but it gave me a runtime error,
	// I dont think they are used anywhere else tho
	s1 = -7;
	s2 = -13;
	s3 = -133;


	this->N_P = 14;
	this->N_E = 12;

	//Random Spawning of Platforms
	long int sp1 = -15, sp2 = -5, sp3 = -14;
	double ran_x, ran_y, rand;
	double map_x = round(WindowWidth / 5.0);
	int sector = 0;
	//double offset_x = 150;
	double map_y = WindowHeight - FloorLevel - 50;
	int size_num;
	

	//Character x location
	x = 0.25 * WindowWidth;

	player = new elemental_form(0, x);

	this->x_bg1 = WindowWidth / 2.0;
	this->x_bg2 = x_bg1 + WindowWidth;
	this->x_bg3 = x_bg1 - WindowWidth;
	this->x_player = 0.0;
	this->activeCollision = true;

	for (i = 0; i < 4; i++) {
		this->power_state[i] = false;
	}

	damage_sound = new Audio(".\\sounds\\damage.wav");
	fruit_sound = new Audio(".\\sounds\\fruit.wav");
	background_sound = new Audio(".\\sounds\\background.wav");
	game_over_sound = new Audio(".\\sounds\\game_over.wav");
	enemy_death_sound = new Audio(".\\sounds\\enemy_death.wav");

	ui = new UI();

	this->hp = ui->HP;

	create_sprite(background_file, id);
	

	for (i = 0; i < N_P; i++) {
		//cout << "creating platform #" << i+1 << " \n";

		rand = round(2*(ran(sp1)));
		//cout << "SIZE\n";

		size_num = (int)(rand);
		size_num++;
		//cout << "\tfinal # is: " << size_num << endl;

		//plat size generator (s/m/l)
		if (size_num == 3) {
			size = 'l';
		}
		else if (size_num == 2) {
			size = 'm';
		}
		else {
			size = 's';
		}

		//Random X position
		rand = round(map_x*(ran(sp2)));
		//cout << "X position\n";

		ran_x = rand + map_x * sector;
		//cout << "\tfinal # is: " << ran_x << endl << endl;


		//RANDOM Y Position
		rand = round(map_y*(ran(sp3)));
		//cout << "Y position\n";

		ran_y = rand + FloorLevel;
		//cout << "\tfinal # is: " << ran_y << endl << endl;

		sector++;

		x = ran_x;
		y = ran_y;


		cout << " Platform #"<<i+1<<"Read values: x = " << x << ", y = " << y << ", size = " << size << "\n";
		P[i] = new platform(x, y, size);
		
		if (P[i] == NULL) {
			cout << "\nallocation error in world - Platform Class";
			return;
		}

		//FRUITS///////////////
		if (i < 4) {
			x = (P[i]->startX)+((P[i]->width)*0.25);
			y = P[i]->startY;
			F[i] = new fruit(x, y, i + 1);
			
			if (F[i] == NULL) {
				cout << "\nallocation error in world - Fruit Class";
				return;
			}
		}

		//ENEMIES////////////////
		if (i < N_E) {
			x = P[i]->startX + ((P[i]->width)*0.75);

			ground_yn = round(ran(s3));
			if (ground_yn == 0) y = FloorLevel-20;		//to have some enemies on the ground
			else y = P[i]->startY;						//else enemy is on the platform
			

			if(i<4)E[i] = new enemy(x, y, i + 1);
			else if (i>4&&i<8) E[i] = new enemy(x, y, (i + 1)-4);
			else E[i] = new enemy(x, y, (i + 1) - 8);
			
			if (E[i] == NULL) {
				cout << "\nallocation error in world - Enemy Class";
				return;
			}
		}//END OF ENEMY INIT
	}//END OF PLATFORM FOR LOOP
}//END OF world::world

world::~world() {
	int i;

	for (i = 0; i < N_P; i++) {
		// safe delete
		if (P[i] == NULL) {
			cout << "\nerror in ~World";
			return;
		}
		else {
			delete P[i];
		}
	}
	for (i = 0; i < 4; i++) {
		// safe delete
		if (F[i] == NULL) {
			cout << "\nerror in ~World";
			return;
		}
		else {
			delete F[i];
		}
	}

	for (i = 0; i < N_E; i++) {
		// safe delete
		if (E[i] == NULL) {
			cout << "\nerror in ~World";
			return;
		}
		else {
			delete E[i];
		}
	}

	if (player == NULL) {
		cout << "\nerror in ~World";
		return;
	}
	else {
		delete player;
	}

	/*
	damage_sound = new Audio(".\\sounds\\damage.wav");
	fruit_sound = new Audio(".\\sounds\\fruit.wav");
	background_sound = new Audio(".\\sounds\\background.wav");
	game_over_sound = new Audio(".\\sounds\\game_over.wav");
	enemy_death_sound = new Audio(".\\sounds\\enemy_death.wav");
	*/
}

void world::draw() {
	int i;
	double x, y, q, scale;

	bool collision_box_status = false;

#ifdef COLLISION
	collision_box_status = true;
#endif

	// to draw a background set x=y=q=0 and scale=-1
	x = x_bg1;
	y = WindowHeight / 2.0 - 2.5;
	q = 0.0;
	scale = BgScale;

	// draw background
	draw_sprite(id, x, y, q, scale);

	x = x_bg2;
	draw_sprite(id, x, y, q, scale);

	x = x_bg3;
	draw_sprite(id, x, y, q, scale);

	// draw each platform
	for (i = 0; i < N_P; i++) {
		if(P[i]->isActive == true)P[i]->draw();
	}

	for (i = 0; i < 4; i++) {
		if (power_state[i] == false && F[i]->isActive == true) F[i]->draw();
	}
	for (i = 0; i < N_E; i++) {
		if (E[i]->isActive == true)E[i]->draw();
	}

	player->Draw(collision_box_status);

	ui->draw();

}

void world::sim_step(double dt) {

}

void world::update(double& score) {
	int i;
	double offset_x = ScrollingSpeed;
	double x_temp;
	double xi, xMax;
	double dt = 2.50;
	double hb, fp;

	//bool f[] = { 0,1,0,1 }; //for UI testing

	xMax = score * 100.0;

	FruitCollision();
	EnemyCollision();
	shuffle(score);
	//xi = xMax;

	//background scrolling	////////////////////////////////
	if (KEY('D')) { //Moving Forward
		x_bg1 -= ScrollingSpeed;
		x_bg2 -= ScrollingSpeed;
		x_bg3 -= ScrollingSpeed;
		x_player += ScrollingSpeed;
		//this is for moving the platforms
		//updating platform positions
		for (i = 0; i < N_P; i++) {
			P[i]->update_pl(-offset_x);
		};
		for (i = 0; i < 4; i++) {
			F[i]->update_f(-offset_x);
		};
		for (i = 0; i < N_E; i++) {
			E[i]->update_f(-offset_x);
		};
	}

	if (KEY('A')) { //Moving Backwards
		x_bg1 += ScrollingSpeed;
		x_bg2 += ScrollingSpeed;
		x_bg3 += ScrollingSpeed;
		x_player -= ScrollingSpeed;
		//updating platform positions
		for (i = 0; i < N_P; i++) {
			P[i]->update_pl(offset_x);
		};
		for (i = 0; i < 4; i++) {
			F[i]->update_f(offset_x);
		};
		for (i = 0; i < N_E; i++) {
			E[i]->update_f(offset_x);
		};
	};

	for (i = 0; i < N_P; i++) {
		if (P[i]->isActive == true) {
			if (P[i]->startX < (-1.5*WindowWidth)) {
				P[i]->isActive = false;
				//cout << "Platform #: " << i << " inactive\n";
			}
		}
	};
	for (i = 0; i < N_E; i++) {
		if (E[i]->isActive == true) {
			if (E[i]->x < (-1.5*WindowWidth)) {
				E[i]->isActive = false;
				//cout << "Enemy #: " << i << " inactive\n";
			}
		}
	};
	for (i = 0; i < 4; i++) {
		if (F[i]->isActive == true) {
			if (F[i]->x < (-1.5*WindowWidth)) {
				F[i]->isActive = false;
				//cout << "Fruit #: " << i << " inactive\n";
			}
		}
	};

	//checking background sprite positions	////////////////
	if (x_bg1 < WindowWidth / 2.0) { //Moving Forward   1 2 3
		x_bg3 = x_bg2 + WindowWidth;
		if (x_bg1 - WindowWidth / 2.0 < 0.0) { //bg_1 is out of sight
			x_temp = x_bg1;
			x_bg1 = x_bg2;
			x_bg2 = x_bg3;
			x_bg3 = x_bg1;
		}
	}
	if (x_bg1 > WindowWidth / 2.0) { //Moving Backwards  2 3 1
		x_bg2 = x_bg3 - WindowWidth;
		if (x_bg1 > WindowWidth) {  //bg_1 is out sight
			x_temp = x_bg1;
			x_bg1 = x_bg2;
			x_bg2 = x_bg3;
			x_bg3 = x_bg1;
		}
	}


	xi = x_player;
	if (xi > xMax) {
		xMax = xi;
	}
	score = xMax / 100.0;

	//Jumping Logic
	if (player->Vy < 0.0 && CheckCollision(fp) == true) {
		player->Vy = 0.0;
		player->isGrounded = true;
		hb = player->h * player->scale / 2.0;
		player->y = fp + hb;
	}

	player->Update(dt);
	ui->update(score, power_state, hp);

	//Transforming Elements
	if (KEY('H') && ui->F[0] == 1 && player->mode != 1) { //Transform to earth
		//transform(player, earth_form);
		player->init(1);
		F[0]->isActive = true;
		power_state[0] = 0;
	}
	else if (KEY('J') && ui->F[1] == 1 && player->mode != 2) {	//Transform to water
		//transform(player, water_form);
		player->init(2);
		F[1]->isActive = true;
		power_state[1] = 0;
	}
	else if (KEY('K') && ui->F[2] == 1 && player->mode != 3) {	//Transform to wind
		//transform(player, wind_form);
		player->init(3);
		F[2]->isActive = true;
		power_state[2] = 0;
	}
	else if (KEY('L') && ui->F[3] == 1 && player->mode != 4) {	//Transform to time
		//transform(player, time_form);
		player->init(4);
		F[3]->isActive = true;
		power_state[3] = 0;
	}
	else if (KEY('N')) {	//Transform to Normal
		player->init(0);
		F[3]->isActive = true;
		//power_state[3] = 0;
	}


	//if (background_sound->isPlaying == false)background_sound->play();
}


bool world::CheckCollision(double& fp) {
	int i;
	double initialX, finalX, ypos;

	for (i = 0; i < N_P; i++) {
		if (P[i]->isActive == true) {
			initialX = P[i]->startX;
			finalX = P[i]->startX + P[i]->width;
			ypos = P[i]->startY;
			if (initialX <= player->bXL && finalX >= player->bXR) {
				if (ypos >= player->bBottom && ypos <= player->bTop) {// && isGrounded == false) {
					//cout << "contact with platform, if falling: " << i << endl;
					//cout << endl << i << endl;
					fp = ypos - 5.0;
					return true;//character hitting with top boundary
				}
				else {
					//cout << "within platform's range: " << i << endl;
					return false;
				}
			}
		}
	}
	return false;
}

bool world::FruitCollision() {
	int i;
	double R1 = player->w / 2.0;

	//int ic[4] = { 0 }, N_orig;
	//N_orig = 4;

	double x1, y1;
	double x2, y2, R2;
	double d12;

	x1 = player->x;
	y1 = player->y;


	for (i = 0; i < 4; i++) {
		if (F[i]->isActive == true && power_state[i]==false) {
			x2 = F[i]->x;
			y2 = F[i]->y;
			R2 = F[i]->R;

			d12 = sqrt((x2 - x1) * (x2 - x1) +
				(y2 - y1) * (y2 - y1));

			if (d12 <= (R1 + R2)) {

				// * pick one collision response and comment
				// out the other one to run the example

				// collision response #1 -- reverse velocities
				// P[i]->v *= -1;
				// P[j]->v *= -1;

				// collision response #2 -- indicate each
				// robot that needs to be removed later
				//ic[i] = 1;
				//j = i;
				//cout << "collided with fruit #" << i << endl;
				F[i]->isActive = false;
				fruit_sound->play();
				power_state[i] = true;
				background_sound->isPlaying = false;
				return true;
			}
		}
	}


	return false;
}

bool world::EnemyCollision() {
	int i;
	double R1 = player->w / 2.0;

	//int ic[4] = { 0 }, N_orig;
	//N_orig = 4;

	double x1, y1;
	double x2, y2, R2;
	double d12;

	x1 = player->x;
	y1 = player->y;


	for (i = 0; i < N_E; i++) {
		if (E[i]->isActive == true && activeCollision == true) {
			x2 = E[i]->x;
			y2 = E[i]->y;
			R2 = E[i]->R*0.75;

			d12 = sqrt((x2 - x1) * (x2 - x1) +
				(y2 - y1) * (y2 - y1));


			if (d12 <= (R1 + R2)) {

				// * pick one collision response and comment
				// out the other one to run the example

				// collision response #1 -- reverse velocities
				// P[i]->v *= -1;
				// P[j]->v *= -1;

				// collision response #2 -- indicate each
				// robot that needs to be removed later
				//ic[i] = 1;
				if (E[i]->mode == player->mode) {
					cout << "Enemy Defeated!\n";
					E[i]->isActive = false;
					enemy_death_sound->play();
					background_sound->isPlaying = false;
				}
				else {
					cout << "Damage Taken!\n";
					if (hp > 0 )hp--; // && player->mode==0) hp--; //loose health only if not powered up
					player->init(0);
					activeCollision = false;
					damage_sound->play();
					background_sound->isPlaying = false;
				}
				//cout << "collided with fruit #" << i << endl;
				return true;
			}
		}
	}


	/*
	// collision response #2
	// -- remove the robots that have collided
	for (i = 0; i < N_orig; i++) {
		if (ic[i] == 1) {
			remove(i);
			N--;
		}
	}
	*/
	return false;
}

void world::shuffle(double score) {
	int i,j, ground_yn;
	double rand;// , mapped_rand;
	//int new_plat;
	double map_x = round(WindowWidth / 5.0);
	int sector = 0;
	double ran_x;

	for (i = 0; i < N_P; i++) {

		if (P[i]->isActive == false && x_player > score) {

			//Random X position
			rand = round(150*(ran(s2)));
			if (i == 0) ran_x = P[N_P - 1]->endX + rand;
			else ran_x = P[i - 1]->endX + 50 + rand;

			//cout << "\tfinal # is: " << ran_x << endl << endl;

			sector++;
			P[i]->startX = ran_x;

			P[i]->isActive = true;
			//cout << "Platform shuffle for #: " << i << endl;

			if (sector > 5)sector = 0;

//NOTE: IT WAS EASIER TO SPAWN THE NEW ENEMY AND FRUIT ON THE PLATFORM BEING SHUFFLED
//ELSE THEY WOULD SPAWN ON ANY PLATFORM IN PLAY, ICLUDING THOSE ALREADY PASSED


			//SHUFFLING ENEMIES/////////////////////////
			for (j = 0; j < N_E; j++) {
				if (E[j]->isActive == false) {
					E[j]->x = P[i]->startX + ((P[i]->width)*0.75);

					ground_yn = round(ran(s3));
					if (ground_yn == 0)	E[j]->y = FloorLevel;// -(E[j]->R / 2.0);	//to have some enemies on the ground
					else E[j]->y = P[i]->startY + E[j]->R;						//else enemy is on the platform
					
					E[j]->isActive = true;
					//cout << "value of seed2: " << s1 << endl;
					break;
				}
				//if (E[j]->isActive == true) break;
			} //END OF ENEMY SHUFFLE

	//FRUIT SHUFFLE////////////////////////////////
			for (j = 0; j < 4; j++) {

				if (F[j]->isActive == false) {

					F[j]->x = P[i]->startX + F[j]->R;
					F[j]->y = P[i]->startY + F[j]->R;
					F[j]->isActive = true;
					//cout << "value of seed1: " << s1 << endl;
					break;
				}
				//if (F[j]->isActive == true) break;
			}//END OF FRUIT SHUFFLE




		} //CHECK IF PLATFORM IS OUT OF SIGHT
	} //END OF PLATFORM FOR LOOP


	
	
}

UI::UI() {
	int i;
	N = 4;
	X = 250.0;
	Y = WindowHeight - 50.0;
	Scale = PowerIconsScale;
	Score = 0.0;
	HP = 5;

	for (i = 0; i < 4; i++) {
		F[i] = 0;
	}

	//Colored Fruits
	create_sprite(".\\art\\water_fruit.png", id_water_fruit);
	create_sprite(".\\art\\wind_fruit.png", id_wind_fruit);
	create_sprite(".\\art\\earth_fruit.png", id_earth_fruit);
	create_sprite(".\\art\\time_fruit.png", id_time_fruit);
	//Dead Fruits
	create_sprite(".\\art\\deadwater_fruit.png", id_water_fruit_dark);
	create_sprite(".\\art\\deadwind_fruit.png", id_wind_fruit_dark);
	create_sprite(".\\art\\deadearth_fruit.png", id_earth_fruit_dark);
	create_sprite(".\\art\\deadtime_fruit.png", id_time_fruit_dark);
	//HP
	create_sprite(".\\art\\heart.png", id_heart);
	create_sprite(".\\art\\deadheart.png", id_heart_dark);
}

UI::~UI() {

}

void UI::update(double score, bool f[], int hp) {
	int i;
	Score = score;

	for (i = 0; i < 4; i++) {
		F[i] = f[i];
	}

	if (hp >= 0 && hp <= 5) {
		HP = hp;
	}


}

void UI::draw() {
	int i;
	double xhp, yhp, gap;
	double xScore = 10.0;
	double yScore = WindowHeight - 30.0;
	double Fruit_tscale = 0.5;
	xhp = X;
	yhp = Y;
	gap = 371.45 * PowerIconsScale;


	text("Score: ", xScore, yScore, 0.75);
	xScore += 105;
	text((int)Score, xScore, yScore, 0.75);

	if (F[0] == 1) {
		draw_sprite(id_earth_fruit, xhp, yhp, 0, Scale);
	}
	else draw_sprite(id_earth_fruit_dark, xhp, yhp, 0, Scale);
	text("H", xhp-10, yhp - 40, Fruit_tscale);

	xhp += gap;
	if (F[1] == 1) {
		draw_sprite(id_water_fruit, xhp, yhp, 0, Scale);
	}
	else draw_sprite(id_water_fruit_dark, xhp, yhp, 0, Scale);
	text("J", xhp-10, yhp - 40, Fruit_tscale);

	xhp += gap;
	if (F[2] == 1) {
		draw_sprite(id_wind_fruit, xhp, yhp, 0, Scale);
	}
	else draw_sprite(id_wind_fruit_dark, xhp, yhp, 0, Scale);
	text("K", xhp-10, yhp - 40, Fruit_tscale);

	xhp += gap;
	if (F[3] == 1) {
		draw_sprite(id_time_fruit, xhp, yhp, 0, Scale);
	}
	else draw_sprite(id_time_fruit_dark, xhp, yhp, 0, Scale);
	text("L", xhp-10, yhp - 40, Fruit_tscale);


	xhp = WindowWidth - 5 * gap;
	for (i = 1; i <= HP; i++) {
		draw_sprite(id_heart, xhp, yhp, 0, Scale * 0.5);
		xhp += gap;
	}
	for (i = HP; i < 5; i++) {
		draw_sprite(id_heart_dark, xhp, yhp, 0, Scale * 0.5);
		xhp += gap;
	}

}

Audio::Audio(char* file_name) {
	// use beam wav file in the canonical format
	// so the bytes format is the same as described in
	// "WAVE_soundfile_format.pdf"
	fstream fin;

	isPlaying = false;

	fin.open(file_name, ios::in | ios::binary);

	//cout << "File name: " << file_name;

	if (!fin) {
		cout << "\nfile open error";
		exit(0);
	}

	// we need to know n the size of the binary file
	// in order to allocate the buffer and read it
	fin.seekg(0, ios::end); // move to the end of the file

	N_buffer = fin.tellg(); // get the current position -> size of file

	fin.seekg(0, ios::beg); // move to the beginning of the file (rewind)

	// need to dynamically allocate an array of size n for p_buffer
	p_buffer = new char[N_buffer];

	if (p_buffer == NULL) {
		cout << "\ndynamic memory allocation error";
		exit(0);
	}

	fin.read(p_buffer, N_buffer);

	fin.close();

	p_data = p_buffer + 44;
	N_data = N_buffer - 44;

	// get sound info from header / beginning of the file buffer //

	// number of channels (a 2 byte int) is at byte 22
	// -- see "WAVE_soundfile_format.pdf"

	NumChannels = *(short int*)(p_buffer + 22);

	// sample rate (a 4 byte int) is at byte 24
	int* pi = (int*)(p_buffer + 24);

	SampleRate = *pi;

	SampleRate = *(int*)(p_buffer + 24);

	//cout << "\n\nNumChannels = " << NumChannels;
	//cout << "\nSampleRate (Hz) = " << SampleRate << " Hz";
}

void Audio::play() {
	isPlaying = true;
	// play a wav sound file stored in memory (faster than a file)
	// p_buffer is a memory buffer read from a binary wave file
	// p_buffer is a character pointer to a memory buffer for the file
	//cout << "\nplaying sound ...";
	PlaySoundA(p_buffer, NULL, SND_MEMORY | SND_ASYNC);
	// note: it takes about 200 ms for PlaySound function to respond
	//Sleep(1000);
}

Audio::~Audio() {
	delete[] p_buffer;
}