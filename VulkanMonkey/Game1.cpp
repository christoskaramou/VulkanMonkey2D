#include "Game1.h"
#include <chrono>
#include <random>

namespace vm {
	void windowResizedCallback(GLFWwindow* window, int width, int height);
	void keysCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	Camera *camera;
	Entity player;
	std::vector<Entity> objects;

	void Game1::load()
	{
		// set up the window (renderer)
		window.createWindow(1200, 800, "", nullptr, nullptr);
		window.setWindowUserPointer(this); // pointer for callbacks usage
		window.setWindowSizeCallback(windowResizedCallback);
		window.setKeyCallback(keysCallback);
		window.setScrollCallback(scroll_callback);

		const float SCALE = 1.f;

		auto seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::default_random_engine gen((unsigned int)seed);
		std::uniform_real_distribution<float> x(-800.0f * SCALE, 800.0f * SCALE);
		std::uniform_real_distribution<float> y(-800.0f * SCALE, 800.0f * SCALE);
		std::uniform_real_distribution<float> h(10.f, 17.0f);
		std::uniform_real_distribution<float> w(10.f, 17.0f);
		
		// SCENE

		// player
		Rect rect{ b2Vec2(0, 0), b2Vec2(15, 20) };
		player.setSprite(new Sprite(rect, "textures/stickman.png"));
		player.setDepth(0.12f); // front
		player.createBody2D(rect.pos.x, rect.pos.y);
		player.addBoxShape(rect.size.x, rect.size.y);
		player.body->SetType(b2BodyType::b2_dynamicBody);
		player.body->SetFixedRotation(true);
		player.body->GetFixtureList()->SetRestitution(0.f);
		player.body->SetGravityScale(0.f);
		//player.body->

		Rect rect1;
		for (int i = 0; i < 100 * SCALE; i++) {
			rect1 = { b2Vec2(x(gen), y(gen)), b2Vec2(w(gen), h(gen)) };
			rect1.size.y = rect1.size.x;
			objects.push_back(Entity());
			objects.back().setSprite(new Sprite(rect1, "textures/sun.png"));
			objects.back().createBody2D(rect1.pos.x, rect1.pos.y);
			objects.back().addCircleShape(rect1.size.x / 2.5f);
			if (i % 2 == 0)
				objects.back().body->SetGravityScale(-1.f);

			rect1 = { b2Vec2(x(gen), y(gen)), b2Vec2(w(gen), h(gen)) };
			rect1.size.y = rect1.size.x;
			objects.push_back(Entity());
			objects.back().setSprite(new Sprite(rect1, "textures/cd.png"));
			objects.back().createBody2D(rect1.pos.x, rect1.pos.y);
			objects.back().addCircleShape(rect1.size.x);
			if (i % 2 == 0)
				objects.back().body->SetGravityScale(-1.f);

			rect1 = { b2Vec2(x(gen), y(gen)), b2Vec2(w(gen), h(gen)) };
			rect1.size.y = rect1.size.x;
			objects.push_back(Entity());
			objects.back().setSprite(new Sprite(rect1, "textures/circle.png"));
			objects.back().createBody2D(rect1.pos.x, rect1.pos.y);
			objects.back().addCircleShape(rect1.size.x);
			if (i % 2 == 0)
				objects.back().body->SetGravityScale(-1.f);

			rect1 = { b2Vec2(x(gen), y(gen)), b2Vec2(w(gen), h(gen)) };
			rect1.size.y = rect1.size.x;
			objects.push_back(Entity());
			objects.back().setSprite(new Sprite(rect1, "textures/circle-maze.png"));
			objects.back().createBody2D(rect1.pos.x, rect1.pos.y);
			objects.back().addCircleShape(rect1.size.x);
			if (i % 2 == 0)
				objects.back().body->SetGravityScale(-1.f);

			rect1 = { b2Vec2(x(gen), y(gen)), b2Vec2(w(gen), h(gen)) };
			objects.push_back(Entity());
			objects.back().setSprite(new Sprite(rect1, "textures/default.jpg"));
			objects.back().createBody2D(rect1.pos.x, rect1.pos.y);
			objects.back().addBoxShape(rect1.size.x, rect1.size.y);
			if (i % 2 == 0)
				objects.back().body->SetGravityScale(-1.f);
		}

		// top
		rect1 = { b2Vec2(0, 850), b2Vec2(850, 5) };
		rect1 = rect1 * SCALE;
		objects.push_back(Entity());
		objects.back().setSprite(new Sprite(rect1));
		objects.back().createBody2D(rect1.pos.x, rect1.pos.y);
		objects.back().addBoxShape(rect1.size.x, rect1.size.y);
		objects.back().body->SetType(b2BodyType::b2_staticBody);
		// bot
		rect1 = { b2Vec2(0, -850), b2Vec2(850, 5) };
		rect1 = rect1 * SCALE;
		objects.push_back(Entity());
		objects.back().setSprite(new Sprite(rect1));
		objects.back().createBody2D(rect1.pos.x, rect1.pos.y);
		objects.back().addBoxShape(rect1.size.x, rect1.size.y);
		objects.back().body->SetType(b2BodyType::b2_staticBody);
		// left
		rect1 = { b2Vec2(-850, 0), b2Vec2(5, 850) };
		rect1 = rect1 * SCALE;
		objects.push_back(Entity());
		objects.back().setSprite(new Sprite(rect1));
		objects.back().createBody2D(rect1.pos.x, rect1.pos.y);
		objects.back().addBoxShape(rect1.size.x, rect1.size.y);
		objects.back().body->SetType(b2BodyType::b2_staticBody);
		//right
		rect1 = { b2Vec2(850, 0), b2Vec2(5, 850) };
		rect1 = rect1 * SCALE;
		objects.push_back(Entity());
		objects.back().setSprite(new Sprite(rect1));
		objects.back().createBody2D(rect1.pos.x, rect1.pos.y);
		objects.back().addBoxShape(rect1.size.x, rect1.size.y);
		objects.back().body->SetType(b2BodyType::b2_staticBody);

		//rotating block
		rect1 = { b2Vec2(0, 0), b2Vec2(5, 450) };
		rect1 = rect1 * SCALE;
		objects.push_back(Entity());
		objects.back().setSprite(new Sprite(rect1));
		objects.back().createBody2D(rect1.pos.x, rect1.pos.y);
		objects.back().addBoxShape(rect1.size.x, rect1.size.y);
		objects.back().body->SetType(b2BodyType::b2_kinematicBody);

		PointLight &light = pointLight[0];
		light.attachTo(player.getTranslationMat());
		light.setRadius(100.f);
		light.setColor({ 1.f, 0.3f, 1.f, 0.0f });
		light.turnOn();
		//AmbientLight::color = { 1.f, 1.f, 1.f, 0.5f };
	}

	void Game1::init()
	{
		Game::init();

		// init the main Camera
		Renderer &r = window.getRenderer();
		camera = window.getRenderer().getMainCamera();
		camera->init(r.swapchainExtent.width, r.swapchainExtent.height, r.gpu, r.device, r.gpuProperties);

		// init all sprites to the staring position
		// (this is not necessary unless the object will not get updated later)
		b2Transform t;

		t.Set(b2Vec2(P2M*player.getSprite().getRect().pos.x, P2M*player.getSprite().getRect().pos.y), player.getAngle());
		player.setTransform(t);
		camera->attachTo(player.getTranslationMat());

		for (auto &e : objects) {
			t.Set(b2Vec2(P2M*e.getSprite().getRect().pos.x, P2M*e.getSprite().getRect().pos.y), e.getAngle());
			e.setTransform(t);
		}
	}

	void Game1::update(double delta)
	{
		pointLight[0].update();
		for (auto &e : objects) {
			if (e.hasBody()) 
				e.setTransform(e.body->GetTransform());
			e.update();
		}
		if (player.hasBody())
			player.setTransform(player.body->GetTransform());
		player.update();

		camera->update();

		physics2D_Step(delta);
	}
	void Game1::draw()
	{
		Game::draw();
		for (auto &e : objects) {
			e.draw();
		}

		player.draw();

		window.getRenderer().summit(true);
	}

	void Game1::checkInput(double delta)
	{
		float _delta = static_cast<float>(delta);
		if (window.getKey(KEY_A)) {
			if (gameState == GameState::Running)
				player.body->ApplyLinearImpulseToCenter(b2Vec2(-100.f*_delta, 0), true);
		}

		if (window.getKey(KEY_D)) {
			if (gameState == GameState::Running)
				player.body->ApplyLinearImpulseToCenter(b2Vec2(100.f*_delta, 0), true);
		}

		if (window.getKey(KEY_W)) {
			if (gameState == GameState::Running)
				player.body->ApplyLinearImpulseToCenter(b2Vec2(0, 100.f*_delta), true);
		}

		if (window.getKey(KEY_S)) {
			if (gameState == GameState::Running)
				player.body->ApplyLinearImpulseToCenter(b2Vec2(0, -100.f*_delta), true);
		}
		if (window.getKey(KEY_PAGE_UP)) {
			AmbientLight::color.w = 1.f;
		}
		if (window.getKey(KEY_PAGE_DOWN)) {
			AmbientLight::color.w = 0.f;
		}

		if (window.getKey(KEY_SPACE)) {
			if (objects.back().body->GetAngularVelocity() != -1.f)
				objects.back().body->SetAngularVelocity(-1.0f);
		}
		else {
			if (objects.back().body->GetAngularVelocity() != 0.0f)
				objects.back().body->SetAngularVelocity(0.0f);
		}
		if (window.getKey(KEY_RIGHT)) {
			pointLight[0].setRadius(pointLight[0].getRadius() + 150.f *_delta);
		}
		if (window.getKey(KEY_LEFT)) {
			pointLight[0].setRadius(pointLight[0].getRadius() - 150.f *_delta);
			if (pointLight[0].getRadius() < 0.f)
				pointLight[0].setRadius(0.f);
		}

		if (window.getKey(KEY_P)) {
			gameState = GameState::Paused;
		}
		else
			gameState = GameState::Running;
	}
	void windowResizedCallback(GLFWwindow* window, int width, int height)
	{
		if (width == 0 || height == 0)
			return;

		vm::Game1* app = reinterpret_cast<vm::Game1*>(glfwGetWindowUserPointer(window));
		app->getWindow().getRenderer().reInitSwapchain();
	}
	void keysCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		vm::Game1* app = reinterpret_cast<vm::Game1*>(glfwGetWindowUserPointer(window));
		if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		if (key == GLFW_KEY_KP_ADD && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			app->setMaxFPS(app->getMaxFps() + 30);
		}
		else if (key == GLFW_KEY_KP_SUBTRACT && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
			if (app->getMaxFps() >= 30) {
				app->setMaxFPS(app->getMaxFps() - 30);
			}
			else {
				app->setMaxFPS(0);
			}
		}
	}
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		vm::Game1* app = reinterpret_cast<vm::Game1*>(glfwGetWindowUserPointer(window));
		float zoom = (float)yoffset / 20.0f;
		app->getWindow().getRenderer().mainCamera.addZoom(-zoom, app->getWindow().getRenderer().swapchainExtent.width, app->getWindow().getRenderer().swapchainExtent.height);
	}
}
