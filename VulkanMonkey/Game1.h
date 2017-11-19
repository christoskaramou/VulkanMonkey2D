#pragma once
#include "Game.h"

namespace vm {
	class Game1 : public Game
	{
	public:
		Game1() {};
		~Game1() {};
		void init() override;
		void load() override;
		void update(double delta) override;
		void draw() override;
		void checkInput(double delta) override;
	};
}