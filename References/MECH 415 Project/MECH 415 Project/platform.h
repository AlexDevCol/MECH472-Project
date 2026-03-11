using namespace std;

class platform
{
private:
	char size;			//platfrom size (s,m,l)

	//char* file_name;


public:
	int id_plat;
	double startX, startY, endX, endY;	//start and end points for the platfrom
	double width, height;		//platfrom width based on size
	bool isActive;


	platform(double x, double y, char Psize);//constructor
	void init(double x, double y, char Psize);

	void draw();						//draws platform as lines
	void print();						//prints platform variables
	void update_pl(double offset);	//updates platforms position
};

