#include "Window.h"
#include <stdlib.h>

namespace vm {

	Window::Window()
	{
		window = nullptr;
		width = 0;
		height = 0;
		title = "";
	}


	Window::~Window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		window = nullptr;
		delete renderer;
	}
	void Window::createWindow(int width, int height, std::string title, bool fullscreen)
	{
		if (!glfwInit()) exit(-1);
		if (!glfwVulkanSupported()) exit(-1);
		windowHint(GLFW_CLIENT_API, GLFW_NO_API);

		GLFWmonitor * monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;

		window = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);

		renderer = new Renderer(getWindow());
	}

	Renderer& Window::getRenderer() const
	{
		return *renderer;
	}
	void Window::setWindowUserPointer(void *pointer) const
	{
		glfwSetWindowUserPointer(window, pointer);
	}
	void * Window::getWindowUserPointer() const
	{
		return glfwGetWindowUserPointer(window);
	}
	void Window::setWindowSizeCallback(GLFWwindowsizefun callback) const
	{
		glfwSetWindowSizeCallback(window, callback);
	}
	void Window::setKeyCallback(GLFWkeyfun callback) const
	{
		glfwSetKeyCallback(window, callback);
	}
	void Window::setScrollCallback(GLFWscrollfun callback) const
	{
		glfwSetScrollCallback(window, callback);
	}
	void Window::windowHint(int hint, int value) const
	{
		glfwWindowHint(hint, value);
	}
	GLFWwindow * Window::getWindow() const
	{
		return window;
	}
	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(window);
	}
	void Window::setWindowShouldClose(int value) const
	{
		glfwSetWindowShouldClose(window, value);
	}
	void Window::setWindowTitle(std::string title) const
	{
		glfwSetWindowTitle(window, title.c_str());
	}
	void Window::pollEvents() const
	{
		glfwPollEvents();
	}
	bool Window::getKey(int key) const
	{
		return glfwGetKey(window, key) == GLFW_PRESS;
	}
}
