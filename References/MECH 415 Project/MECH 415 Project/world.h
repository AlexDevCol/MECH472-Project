
// File by Luis Alejandro Gonzalez///////////////////////////////////////////////

#include <fstream>
#include "platform.h"
#include "elemental_form.h"

const int N_PLAT_MAX = 100;
const int N_FRUIT_MAX = 10;
const int N_ENEMY_MAX = 30;


class UI {
private:
public:
	double X, Y;
	int N;
	double Scale;
	int id_water_fruit, id_wind_fruit, id_time_fruit, id_earth_fruit;
	int id_water_fruit_dark, id_wind_fruit_dark, id_time_fruit_dark, id_earth_fruit_dark;
	int id_heart, id_heart_dark;
	double Score;

	int HP;

	bool F[4];

	UI();
	~UI();
	void update(double score, bool f[],int hp);
	void draw();
};

class Audio {
private:
public:
	char* p_buffer; //dynamic memory busser of N_buffer bytes for binary file
	int N_buffer;	//number of bytes for memory buffer
	char* p_data; //pointer to audio data in p_buffer
	int N_data; //the number of bytes for audio data
	bool isPlaying;

	short int NumChannels; //number of audio channels (1- mono, 2- stereo) (2 byte int)
	int SampleRate; //number of samples per second for the audio data (Hz)

	Audio(char* file_name); //initiliaze using wave file
	~Audio();

	void play(); //play sound with Windows PlaySoundA function to play the wave file in p_buffer

};

class world {

public:

	int N_P, N_E; // the number of platforms

	platform* P[N_PLAT_MAX]; // array of platform object pointers
	fruit* F[N_FRUIT_MAX]; //array of fruit object pointers
	enemy* E[N_ENEMY_MAX]; //array of enemy object pointers
	elemental_form *player;
	Audio* damage_sound, * fruit_sound, * background_sound, *enemy_death_sound, *game_over_sound;
	UI* ui;

	int id; // sprite id number of the world background
	double x_bg1, x_bg2, x_bg3;
	double x_player;
	bool activeCollision;
	bool power_state[4];
	int hp;
	long int s1, s2, s3;

	world(char* background_file);
	~world();

	void draw();

	void sim_step(double dt);

	void update(double& score);
	void shuffle(double score);

	// sets robot inputs to identical values using the arrow keys
	bool CheckCollision(double& fp);
	bool FruitCollision();
	bool EnemyCollision();
};


