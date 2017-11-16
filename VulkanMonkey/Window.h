#pragma once
#include "Renderer.h"
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>
#include <GLFW\glfw3native.h>

#define RELEASE					GLFW_RELEASE             

#define PRESS					GLFW_PRESS                 

#define REPEAT					GLFW_REPEAT                

#define KEY_UNKNOWN				GLFW_KEY_UNKNOWN           

#define KEY_SPACE				GLFW_KEY_SPACE             
#define KEY_APOSTROPHE			GLFW_KEY_APOSTROPHE        
#define KEY_COMMA				GLFW_KEY_COMMA             
#define KEY_MINUS				GLFW_KEY_MINUS             
#define KEY_PERIOD				GLFW_KEY_PERIOD            
#define KEY_SLASH				GLFW_KEY_SLASH             
#define KEY_0					GLFW_KEY_0                 
#define KEY_1					GLFW_KEY_1                 
#define KEY_2					GLFW_KEY_2                 
#define KEY_3					GLFW_KEY_3                 
#define KEY_4					GLFW_KEY_4                 
#define KEY_5					GLFW_KEY_5                 
#define KEY_6					GLFW_KEY_6                 
#define KEY_7					GLFW_KEY_7                 
#define KEY_8					GLFW_KEY_8                 
#define KEY_9					GLFW_KEY_9                 
#define KEY_SEMICOLON			GLFW_KEY_SEMICOLON         
#define KEY_EQUAL				GLFW_KEY_EQUAL             
#define KEY_A					GLFW_KEY_A                 
#define KEY_B					GLFW_KEY_B                 
#define KEY_C					GLFW_KEY_C                 
#define KEY_D					GLFW_KEY_D                 
#define KEY_E					GLFW_KEY_E                 
#define KEY_F					GLFW_KEY_F                 
#define KEY_G					GLFW_KEY_G                 
#define KEY_H					GLFW_KEY_H                 
#define KEY_I					GLFW_KEY_I                 
#define KEY_J					GLFW_KEY_J                 
#define KEY_K					GLFW_KEY_K                 
#define KEY_L					GLFW_KEY_L                 
#define KEY_M					GLFW_KEY_M                 
#define KEY_N					GLFW_KEY_N                 
#define KEY_O					GLFW_KEY_O                 
#define KEY_P					GLFW_KEY_P                 
#define KEY_Q					GLFW_KEY_Q                 
#define KEY_R					GLFW_KEY_R                 
#define KEY_S					GLFW_KEY_S                 
#define KEY_T					GLFW_KEY_T                 
#define KEY_U					GLFW_KEY_U                 
#define KEY_V					GLFW_KEY_V                 
#define KEY_W					GLFW_KEY_W                 
#define KEY_X					GLFW_KEY_X                 
#define KEY_Y					GLFW_KEY_Y                 
#define KEY_Z					GLFW_KEY_Z                 
#define KEY_LEFT_BRACKET		GLFW_KEY_LEFT_BRACKET      
#define KEY_BACKSLASH			GLFW_KEY_BACKSLASH         
#define KEY_RIGHT_BRACKET		GLFW_KEY_RIGHT_BRACKET     
#define KEY_GRAVE_ACCENT		GLFW_KEY_GRAVE_ACCENT      
#define KEY_WORLD_1				GLFW_KEY_WORLD_1           
#define KEY_WORLD_2				GLFW_KEY_WORLD_2           

#define KEY_ESCAPE				GLFW_KEY_ESCAPE          
#define KEY_ENTER				GLFW_KEY_ENTER           
#define KEY_TAB					GLFW_KEY_TAB             
#define KEY_BACKSPACE			GLFW_KEY_BACKSPACE       
#define KEY_INSERT				GLFW_KEY_INSERT          
#define KEY_DELETE				GLFW_KEY_DELETE          
#define KEY_RIGHT				GLFW_KEY_RIGHT           
#define KEY_LEFT				GLFW_KEY_LEFT            
#define KEY_DOWN				GLFW_KEY_DOWN            
#define KEY_UP					GLFW_KEY_UP              
#define KEY_PAGE_UP				GLFW_KEY_PAGE_UP         
#define KEY_PAGE_DOWN			GLFW_KEY_PAGE_DOWN       
#define KEY_HOME				GLFW_KEY_HOME            
#define KEY_END					GLFW_KEY_END             
#define KEY_CAPS_LOCK			GLFW_KEY_CAPS_LOCK       
#define KEY_SCROLL_LOCK			GLFW_KEY_SCROLL_LOCK     
#define KEY_NUM_LOCK			GLFW_KEY_NUM_LOCK        
#define KEY_PRINT_SCREEN		GLFW_KEY_PRINT_SCREEN    
#define KEY_PAUSE				GLFW_KEY_PAUSE           
#define KEY_F1					GLFW_KEY_F1              
#define KEY_F2					GLFW_KEY_F2              
#define KEY_F3					GLFW_KEY_F3              
#define KEY_F4					GLFW_KEY_F4              
#define KEY_F5					GLFW_KEY_F5              
#define KEY_F6					GLFW_KEY_F6              
#define KEY_F7					GLFW_KEY_F7              
#define KEY_F8					GLFW_KEY_F8              
#define KEY_F9					GLFW_KEY_F9              
#define KEY_F10					GLFW_KEY_F10             
#define KEY_F11					GLFW_KEY_F11             
#define KEY_F12					GLFW_KEY_F12             
#define KEY_F13					GLFW_KEY_F13             
#define KEY_F14					GLFW_KEY_F14             
#define KEY_F15					GLFW_KEY_F15             
#define KEY_F16					GLFW_KEY_F16             
#define KEY_F17					GLFW_KEY_F17             
#define KEY_F18					GLFW_KEY_F18             
#define KEY_F19					GLFW_KEY_F19             
#define KEY_F20					GLFW_KEY_F20             
#define KEY_F21					GLFW_KEY_F21             
#define KEY_F22					GLFW_KEY_F22             
#define KEY_F23					GLFW_KEY_F23             
#define KEY_F24					GLFW_KEY_F24             
#define KEY_F25					GLFW_KEY_F25             
#define KEY_KP_0				GLFW_KEY_KP_0            
#define KEY_KP_1				GLFW_KEY_KP_1            
#define KEY_KP_2				GLFW_KEY_KP_2            
#define KEY_KP_3				GLFW_KEY_KP_3            
#define KEY_KP_4				GLFW_KEY_KP_4            
#define KEY_KP_5				GLFW_KEY_KP_5            
#define KEY_KP_6				GLFW_KEY_KP_6            
#define KEY_KP_7				GLFW_KEY_KP_7            
#define KEY_KP_8				GLFW_KEY_KP_8            
#define KEY_KP_9				GLFW_KEY_KP_9            
#define KEY_KP_DECIMAL			GLFW_KEY_KP_DECIMAL      
#define KEY_KP_DIVIDE			GLFW_KEY_KP_DIVIDE       
#define KEY_KP_MULTIPLY			GLFW_KEY_KP_MULTIPLY     
#define KEY_KP_SUBTRACT			GLFW_KEY_KP_SUBTRACT     
#define KEY_KP_ADD				GLFW_KEY_KP_ADD          
#define KEY_KP_ENTER			GLFW_KEY_KP_ENTER        
#define KEY_KP_EQUAL			GLFW_KEY_KP_EQUAL        
#define KEY_LEFT_SHIFT			GLFW_KEY_LEFT_SHIFT      
#define KEY_LEFT_CONTROL		GLFW_KEY_LEFT_CONTROL    
#define KEY_LEFT_ALT			GLFW_KEY_LEFT_ALT        
#define KEY_LEFT_SUPER			GLFW_KEY_LEFT_SUPER      
#define KEY_RIGHT_SHIFT			GLFW_KEY_RIGHT_SHIFT     
#define KEY_RIGHT_CONTROL		GLFW_KEY_RIGHT_CONTROL   
#define KEY_RIGHT_ALT			GLFW_KEY_RIGHT_ALT       
#define KEY_RIGHT_SUPER			GLFW_KEY_RIGHT_SUPER     
#define KEY_MENU				GLFW_KEY_MENU            

#define KEY_LAST				GLFW_KEY_LAST       

//#define MOD_SHIFT				GLFW_MOD_SHIFT      

//#define MOD_CONTROL				GLFW_MOD_CONTROL    

//#define MOD_ALT					GLFW_MOD_ALT        

#define MOD_SUPER				GLFW_MOD_SUPER      

#define MOUSE_BUTTON_1			GLFW_MOUSE_BUTTON_1     
#define MOUSE_BUTTON_2			GLFW_MOUSE_BUTTON_2     
#define MOUSE_BUTTON_3			GLFW_MOUSE_BUTTON_3     
#define MOUSE_BUTTON_4			GLFW_MOUSE_BUTTON_4     
#define MOUSE_BUTTON_5			GLFW_MOUSE_BUTTON_5     
#define MOUSE_BUTTON_6			GLFW_MOUSE_BUTTON_6     
#define MOUSE_BUTTON_7			GLFW_MOUSE_BUTTON_7     
#define MOUSE_BUTTON_8			GLFW_MOUSE_BUTTON_8     
#define MOUSE_BUTTON_LAST		GLFW_MOUSE_BUTTON_LAST  
#define MOUSE_BUTTON_LEFT		GLFW_MOUSE_BUTTON_LEFT  
#define MOUSE_BUTTON_RIGHT		GLFW_MOUSE_BUTTON_RIGHT 
#define MOUSE_BUTTON_MIDDLE		GLFW_MOUSE_BUTTON_MIDDLE

#define JOYSTICK_1				GLFW_JOYSTICK_1         
#define JOYSTICK_2				GLFW_JOYSTICK_2         
#define JOYSTICK_3				GLFW_JOYSTICK_3         
#define JOYSTICK_4				GLFW_JOYSTICK_4         
#define JOYSTICK_5				GLFW_JOYSTICK_5         
#define JOYSTICK_6				GLFW_JOYSTICK_6         
#define JOYSTICK_7				GLFW_JOYSTICK_7         
#define JOYSTICK_8				GLFW_JOYSTICK_8         
#define JOYSTICK_9				GLFW_JOYSTICK_9         
#define JOYSTICK_10				GLFW_JOYSTICK_10        
#define JOYSTICK_11				GLFW_JOYSTICK_11        
#define JOYSTICK_12				GLFW_JOYSTICK_12        
#define JOYSTICK_13				GLFW_JOYSTICK_13        
#define JOYSTICK_14				GLFW_JOYSTICK_14        
#define JOYSTICK_15				GLFW_JOYSTICK_15        
#define JOYSTICK_16				GLFW_JOYSTICK_16        
#define JOYSTICK_LAST			GLFW_JOYSTICK_LAST      

namespace vm {
	class Window
	{
	public:
		Window();
		~Window();

		void createWindow(int width, int height, std::string title = "", GLFWmonitor *monitor = nullptr, GLFWwindow *share = nullptr); /*glfwGetPrimaryMonitor()*/
		Renderer& getRenderer();
		void setWindowUserPointer(void *pointer);
		void* getWindowUserPointer();
		void setWindowSizeCallback(GLFWwindowsizefun callback);
		void setKeyCallback(GLFWkeyfun callback);
		void setScrollCallback(GLFWscrollfun callback);
		void windowHint(int hint, int value);
		GLFWwindow* getWindow();
		bool shouldClose();
		void setWindowShouldClose(int value);
		void setWindowTitle(std::string title);
		void pollEvents();
		bool getKey(int key);
	private:
		GLFWwindow *window;
		int width;
		int height;
		std::string title;
		Renderer *renderer;

	};
}

