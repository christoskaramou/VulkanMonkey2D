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
	void Window::createWindow(int width, int height, std::string title, GLFWmonitor * monitor, GLFWwindow * share)
	{
		if (!glfwInit()) exit(-1);
		if (!glfwVulkanSupported()) exit(-1);
		windowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(width, height, title.c_str(), monitor, share);

		renderer = new Renderer(getWindow());
	}

	Renderer& Window::getRenderer()
	{
		return *renderer;
	}
	void Window::setWindowUserPointer(void *pointer)
	{
		glfwSetWindowUserPointer(window, pointer);
	}
	void * Window::getWindowUserPointer()
	{
		return glfwGetWindowUserPointer(window);
	}
	void Window::setWindowSizeCallback(GLFWwindowsizefun callback)
	{
		glfwSetWindowSizeCallback(window, callback);
	}
	void Window::setKeyCallback(GLFWkeyfun callback)
	{
		glfwSetKeyCallback(window, callback);
	}
	void Window::setScrollCallback(GLFWscrollfun callback)
	{
		glfwSetScrollCallback(window, callback);
	}
	void Window::windowHint(int hint, int value)
	{
		glfwWindowHint(hint, value);
	}
	GLFWwindow * Window::getWindow()
	{
		return window;
	}
	bool Window::shouldClose()
	{
		return glfwWindowShouldClose(window);
	}
	void Window::setWindowShouldClose(int value)
	{
		glfwSetWindowShouldClose(window, value);
	}
	void Window::setWindowTitle(std::string title)
	{
		glfwSetWindowTitle(window, title.c_str());
	}
	void Window::pollEvents()
	{
		glfwPollEvents();
	}
	bool Window::getKey(int key)
	{
		return glfwGetKey(window, key) == GLFW_PRESS;
	}
}
