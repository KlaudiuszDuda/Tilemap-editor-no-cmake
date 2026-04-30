#pragma once

#include <iostream>
#include <chrono>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "types.h"

class Viewport;

enum class MappedInput
{
	W,
	S,
	A,
	D,
	E,
	Q,
	Z,
	X,
	R,
	T,
	F,
	C,
	CAPS_LOCK,
	ESC,
	LSHIFT,
	SPACE
};

struct Callback
{
	static void updateKeyboardCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);
	static void updateMouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 mods);
	static void updateScrollWheelCallback(GLFWwindow* window, f64 xoffset, f64 yoffset);

	static void windowResizeEvent(GLFWwindow* window, i32 xposIn, i32 yposIn);
	static void windowMousePositionEvent(GLFWwindow* window, f64 xposIn, f64 yposIn);

	union InputData
	{
		f32 analog[GLFW_KEY_LAST + 1 + GLFW_MOUSE_BUTTON_LAST + 1 + 1];
		i32 digital[GLFW_KEY_LAST + 1 + GLFW_MOUSE_BUTTON_LAST + 1 + 1];
	};
	InputData m_InputData{};
	glm::vec2 m_WindowMousePosition;
	glm::vec2 m_WindowSize = { 800.f, 600.f };
	u32 m_GamepadsConnected = 0;
};

extern Callback callback;

class Input
{
public:
	Input();

	void pollInputs();

	i32 getKeyboard(u32 index);
	i32 getMouseButton(u32 index);
	f32 getScrollWheel();

	void MapKeyToAction(MappedInput mapping, u32 index);
	void MapMouseButtonToAction(MappedInput mapping, u32 index);
	void MapScrollWheelToAction(MappedInput mapping);

	void UnmapInputFromAction(MappedInput mapping);

	bool isActionActive(MappedInput mappedInput);
	bool isActionActiveDigital(MappedInput mappedInput, u32 eventType);
	bool isActionActiveAnalog(MappedInput mappedInput, f32 value);

public:

	u32 m_ActionMapping[20];
};

class ProgramTime
{
public:
	f64 dt = 0.0;
	f64 elapsed = 0.0;

private:
	std::chrono::steady_clock::time_point last;

public:
	inline ProgramTime()
	{
		last = std::chrono::steady_clock::now();
	}

	inline void update()
	{
		auto now = std::chrono::steady_clock::now();
		dt = std::chrono::duration<f64>(now - last).count();
		elapsed += dt;
		last = now;
	}
};

class Window
{
public:
    Window();
    ~Window();

	glm::vec2& getWindowSize();
	glm::vec2 getMousePosition();
	glm::vec3 getWorldMousePosition(Viewport& viewport);
	void SwapBuffer();
	void PollEvents();
	GLFWwindow*& getWindowPointer();

    GLFWmonitor* getCurrentMonitor();
    void close();

    GLFWwindow* p_Window;
};

extern Input input;
extern Window window;
extern ProgramTime Time;