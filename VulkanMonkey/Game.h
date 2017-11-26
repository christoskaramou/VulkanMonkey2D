#pragma once
#include "Renderer.h"
#include "Window.h"
#include "Light.h"
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
		virtual void init();
		virtual void load();
		virtual void update(double delta);
		virtual void checkInput(double delta);
		virtual void draw();

		float calculateFPS() const;
		double getDelta() const;
		void setTimeScale(double timeScale);
		double getTimeScale() const;
		unsigned int getMaxFps() const;
		void setMaxFPS(unsigned int fps);
		void setGameState(GameState state);
		Window& getWindow();
		void setAmbientColor(glm::vec4 color) const;
		void physics2D_Step(double delta) const;


	protected:
		GameState gameState;
		Window window;
		PointLight pointLight[MAX_POINT_LIGHTS]; // must have well defined pointLights here and at the shader, because lights are passed as one big uniform block array in every draw call;

	private:
		double delta;
		double timeScale;
		unsigned int limitedFps;
		double limitedSeconds;
	};
}

