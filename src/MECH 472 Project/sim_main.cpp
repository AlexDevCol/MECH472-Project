// Simulation main — entry point and main loop

#include <Windows.h>
#include "Libraries\2D_graphics.h"
#include "global_data.h"
#include "world.h"

int main() {
	double dt = 0.0;

	initialize_graphics();
	world World;

	static bool prevQ = false;
	while (1) {
		clear();
		World.Update(dt);
		World.Draw();

		// Q = shuffle (one-shot: only on key press edge)
		bool q = (KEY('Q') != 0);
		if (q && !prevQ)
			World.Shuffle();
		prevQ = q;
		// Exit on Escape
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			return 0;

		update();
	}

	return 0;
}
