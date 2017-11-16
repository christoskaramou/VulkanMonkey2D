
// use only in cpp files //

#pragma once

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <Windows.h>
#ifdef _DEBUG
#define LOG( s )								\
	{												\
		std::wostringstream os_;					\
		os_ << s;									\
		OutputDebugStringW( os_.str().c_str() );	\
		std::cout << s;								\
	}
#else
#define LOG( s )								\
	{												\
		std::wostringstream os_;					\
		os_ << s;									\
	}
#endif
#define errCheck( x )								\
	{												\
		if (x != vk::Result::eSuccess)				\
			exit(-1);								\
	}