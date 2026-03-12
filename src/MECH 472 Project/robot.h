// robot — Attacker and Defender entity (draw, later logic)

#ifndef ROBOT_H
#define ROBOT_H

#include "global_data.h"

struct robot {
	double X;              // center, inches (camera space)
	double Y;
	double theta_chassis;  // radians
	double theta_laser;    // radians
	bool laserOn;         // true = attack mode (draw laser), false = defense (no laser)
};

class world;

// Draw one robot: body (filled rect), turret circle, and if laserOn the extended laser line.
// isAttacker: true = Attacker (e.g. blue), false = Defender (e.g. green).
void Draw(robot const& r, world const& w, bool isAttacker);

#endif
