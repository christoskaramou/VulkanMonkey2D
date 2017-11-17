#pragma once
#include "Renderer.h"
#include "Window.h"
namespace vm {
	enum class GameState {
		Paused,
		Running,
		Exit
	};
	class Game
	{
	public:
		Game();
		~Game();

		void run(); // the game loop

	public:
		unsigned int getMaxFps();
		void setMaxFPS(unsigned int fps);
		void setGameState(GameState state);
		Window& getWindow();

		virtual void init();
		virtual void load();
		virtual void update(double delta);
		virtual void checkInput(double delta);
		virtual void draw();
		float calculateFPS();
		double getDelta();
		void bulletTime(double timeFactor);

	protected:
		GameState gameState;
		Window window;

	private:
		double delta;
		double timeFactor;
		unsigned int limitedFps;
		double limitedSeconds;
	};
}

