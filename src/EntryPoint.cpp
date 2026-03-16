#include "EntryPoint.h"

EntryPoint::EntryPoint()
{
    
}

EntryPoint::~EntryPoint()
{
    
}

void EntryPoint::run()
{
    while (!glfwWindowShouldClose(window.getWindowPointer()))
    {
        window.PollEvents();

        tilemapRender.update();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        tilemapRender.draw();

        window.SwapBuffer();
    }
}