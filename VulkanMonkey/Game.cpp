#include "Game.h"
#include <sstream>
#include "ErrorAndLog.h"
#include "ResourceManager.h"

namespace vm {
	Game::Game()
	{
		timeFactor = 1.0;
		gameState = GameState::Running;
		limitedFps = 0;
		limitedSeconds = 0;
	}

	Game::~Game()
	{
		
	}

	void vm::Game::run()
	{
		load();
		init();
		if (!window.getWindow()) {
			LOG("window not created\n");
			exit(-1);
		}
		window.getRenderer().pushSpritesToBuffers();
		int frame = 0;
		delta = 0;
		double deltaTemp = 1;
		while (!window.shouldClose()) {

			double startTime = glfwGetTime();

			window.pollEvents();

			//--------------------------------
			checkInput(delta);

			// update and draw
			if (gameState == GameState::Paused)	{
				update(0);
				draw();
			}
			else if (gameState == GameState::Running) {
				update(delta * timeFactor);
				draw();
			}
			else if (gameState == GameState::Exit) {
				window.setWindowShouldClose(true);
			}
			//--------------------------------

			float fps = calculateFPS();
			if (deltaTemp > 1) {
				std::stringstream ss;
				ss << window.getRenderer().getGpuName() << "    Max FPS limit: " << (limitedFps == 0 ? "MAX" : std::to_string((int)limitedFps).c_str()) << "  -  AVRG FPS: " << (int)fps;
				window.setWindowTitle(ss.str());
				deltaTemp = 0;
			}
			else
				deltaTemp += delta;

			double stopTime = glfwGetTime();
			delta = stopTime - startTime;

			// limit fps
			if (limitedFps > 0 && delta < limitedSeconds) {
				while (glfwGetTime() - startTime < limitedSeconds) {}
				delta = glfwGetTime() - startTime;
			}
		}
	}

	void Game::load()
	{
	}

	void Game::init()
	{
	}

	void Game::update(double delta)
	{
	}

	void vm::Game::draw()
	{
		Entity::drawList.clear();
	}

	void vm::Game::checkInput(double delta)
	{
	}

	float vm::Game::calculateFPS()
	{
		static const int SAMPLES = 20;
		static float frameTimes[SAMPLES];
		static int currentFrame = 0;
		static double prevSeconds = glfwGetTime();
		double currentSeconds = glfwGetTime();

		frameTimes[currentFrame % SAMPLES] = (float)(currentSeconds - prevSeconds);
		prevSeconds = currentSeconds;

		int count;
		currentFrame++;
		if (currentFrame < SAMPLES)
			count = currentFrame;
		else
			count = SAMPLES;

		double frameTimeAVRG = 0;
		for (int i = 0; i < count; i++) {
			frameTimeAVRG += frameTimes[i];
		}
		frameTimeAVRG /= count;

		if (frameTimeAVRG > 0)
			return 1.0f / (float)frameTimeAVRG;
		else
			return 1.0f;

	}
	double Game::getDelta()
	{
		return delta;
	}
	void Game::bulletTime(double timeFactor)
	{
		this->timeFactor = timeFactor;
	}
	void Game::setMaxFPS(unsigned int fps)
	{
		limitedFps = fps;
		limitedSeconds = limitedFps ? 1 / (double)limitedFps : 0;
	}
	void Game::setGameState(GameState state)
	{
		gameState = state;
	}
	Window & Game::getWindow()
	{
		return window;
	}
	unsigned int Game::getMaxFps()
	{
		return limitedFps;
	}
}