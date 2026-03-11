
// File by Luis Alejandro Gonzalez///////////////////////////////////////////////


class elemental_form {
private:
public:
	int mode;
	double Speed, jumpVel, Vy, Gravity;
	double x, y;
	double scale;
	int id, id_normal, id_earth, id_water, id_wind, id_time;
	double h, w;
	double bXR, bXL, bBottom, bTop;
	bool isGrounded;

	void init(int m);
	elemental_form(int m, double X); //Constructor
	void power();

	void Update(double dt);//, platform pl[]);
	void Draw(bool box);
	//bool CheckCollision(platform pl[],double &fp);

	//void transform(double t);
};


class fruit {
private:
	int mode;
public:
	double x, y, Scale;
	int id;
	double R;
	bool isActive;

	fruit(double x, double y, int mode);
	~fruit();

	void draw();
	void update_f(double offset);

};

class enemy {
private:
public:
	double x, y, Scale;
	bool isActive;
	int mode;
	int id;
	double R;

	enemy(double x, double y, int mode);
	~enemy();

	void draw();
	void update_f(double offset);

};

