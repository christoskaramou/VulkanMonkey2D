#include "Game.h"
#include <sstream>
#include "ErrorAndLog.h"
#include "ResourceManager.h"

namespace vm {
	Game::Game()
	{
		timeScale = 1.0;
		gameState = GameState::Running;
		limitedFps = 0;
		limitedSeconds = 0;
	}

	Game::~Game()
	{
		for (auto &s : Sprite::sprites) {
			delete s;
			s = nullptr;
		}

		Sprite::sprites.clear();
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
				update(delta * timeScale);
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

	float vm::Game::calculateFPS() const
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
	double Game::getDelta() const
	{
		return delta;
	}
	void Game::setTimeScale(double timeScale)
	{
		this->timeScale = timeScale;
	}
	double Game::getTimeScale() const
	{
		return timeScale;
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
	void Game::setAmbientColor(glm::vec4 color) const
	{
		AmbientLight::color = color;
	}
	void Game::physics2D_Step(double delta) const
	{
		ResourceManager::getInstance().world->Step(static_cast<float>(delta), 8, 3);
	}
	unsigned int Game::getMaxFps() const
	{
		return limitedFps;
	}
}