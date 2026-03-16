#pragma once

#include "types.h"
#include "Window.h"
#include <chrono>
#include "TilemapRender.h"

class EntryPoint
{
public:
	EntryPoint();
	~EntryPoint();

	void run();
private:
	TilemapRender tilemapRender;
};