// Simulation main — entry point and main loop

#include <Windows.h>
#include "Libraries\2D_graphics.h"
#include "global_data.h"
#include "world.h"

int main() {
	double dt = 0.0;

	initialize_graphics();
	world World;

	while (1) {
		clear();
		World.Update(dt);
		World.Draw();

		// Q = shuffle (no action in Phase 1)
		(void)KEY('Q');
		// Exit on Escape
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			return 0;

		update();
	}

	return 0;
}
