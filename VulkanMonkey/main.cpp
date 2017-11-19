#include "Game1.h"
#include <thread>

int main()
{
	{
		vm::Game1 game;

		std::thread t([&] { game.run(); });
		t.join();
	}

	return 0;
}