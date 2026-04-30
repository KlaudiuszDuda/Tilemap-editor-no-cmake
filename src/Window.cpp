#include "Window.h"
#include "Viewport.h"

Callback callback;
Input input;
Window window;
ProgramTime Time;

void GLAPIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204
        || id == 131222
        ) return;
    if (type == GL_DEBUG_TYPE_PERFORMANCE) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;

}

void enableReportGlErrors()
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}

Input::Input()
{
    MapKeyToAction(MappedInput::W, GLFW_KEY_W);
    MapKeyToAction(MappedInput::S, GLFW_KEY_S);
    MapKeyToAction(MappedInput::A, GLFW_KEY_A);
    MapKeyToAction(MappedInput::D, GLFW_KEY_D);
    MapKeyToAction(MappedInput::E, GLFW_KEY_E);
    MapKeyToAction(MappedInput::Q, GLFW_KEY_Q);
    MapKeyToAction(MappedInput::Z, GLFW_KEY_Z);
    MapKeyToAction(MappedInput::X, GLFW_KEY_X);
    MapKeyToAction(MappedInput::R, GLFW_KEY_R);
    MapKeyToAction(MappedInput::T, GLFW_KEY_T);
    MapKeyToAction(MappedInput::F, GLFW_KEY_F);
    MapKeyToAction(MappedInput::C, GLFW_KEY_C);
    MapKeyToAction(MappedInput::CAPS_LOCK, GLFW_KEY_CAPS_LOCK);
    MapKeyToAction(MappedInput::ESC, GLFW_KEY_ESCAPE);
    MapKeyToAction(MappedInput::LSHIFT, GLFW_KEY_LEFT_SHIFT);
    MapKeyToAction(MappedInput::SPACE, GLFW_KEY_SPACE);
}

void Input::pollInputs()
{
    for (i32 i = 0; i < GLFW_KEY_LAST + 1 + GLFW_MOUSE_BUTTON_LAST + 1; i++)
    {
        i32 input = callback.m_InputData.digital[i];
        if (input == GLFW_PRESS)
        {
            callback.m_InputData.digital[i] = GLFW_REPEAT;
        }
    }
}

void Callback::updateKeyboardCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
{
    Callback* ev = static_cast<Callback*>(glfwGetWindowUserPointer(window));
    ev->m_InputData.digital[key] = action;
}

void Callback::updateMouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 mods)
{
    Callback* ev = static_cast<Callback*>(glfwGetWindowUserPointer(window));
    ev->m_InputData.digital[button + GLFW_KEY_LAST + 1] = action;
}

void Callback::updateScrollWheelCallback(GLFWwindow* window, f64 xoffset, f64 yoffset)
{
    Callback* ev = static_cast<Callback*>(glfwGetWindowUserPointer(window));
    ev->m_InputData.analog[GLFW_KEY_LAST + 1 + GLFW_MOUSE_BUTTON_LAST + 1] = yoffset;
}

void Callback::windowResizeEvent(GLFWwindow* window, i32 xposIn, i32 yposIn)
{
    glViewport(0, 0, xposIn, yposIn);
    Callback* ev = static_cast<Callback*>(glfwGetWindowUserPointer(window));
    ev->m_WindowSize.x = xposIn;
    ev->m_WindowSize.y = yposIn;
}

void Callback::windowMousePositionEvent(GLFWwindow* window, f64 xposIn, f64 yposIn)
{
    Callback* ev = static_cast<Callback*>(glfwGetWindowUserPointer(window));
    ev->m_WindowMousePosition.x = (f32)xposIn;
    ev->m_WindowMousePosition.y = (f32)yposIn;
}

i32 Input::getKeyboard(u32 index)
{
    return callback.m_InputData.digital[index];
}

i32 Input::getMouseButton(u32 index)
{
    i32 input = callback.m_InputData.digital[GLFW_KEY_LAST + 1 + index];
    return input;
}

f32 Input::getScrollWheel()
{
    return callback.m_InputData.analog[GLFW_KEY_LAST + 1 + GLFW_MOUSE_BUTTON_LAST + 1];
}

void Input::MapKeyToAction(MappedInput mapping, u32 index)
{
    m_ActionMapping[static_cast<u32>(mapping)] = index;
}
void Input::MapMouseButtonToAction(MappedInput mapping, u32 index)
{
    m_ActionMapping[static_cast<u32>(mapping)] = GLFW_KEY_LAST + 1 + index;
}
void Input::MapScrollWheelToAction(MappedInput mapping)
{
    m_ActionMapping[static_cast<u32>(mapping)] = GLFW_KEY_LAST + 1 + GLFW_MOUSE_BUTTON_LAST + 1;
}

void Input::UnmapInputFromAction(MappedInput mapping)
{
    m_ActionMapping[static_cast<u32>(mapping)] = -1;
}

bool Input::isActionActive(MappedInput mappedInput)
{
    if (callback.m_InputData.digital[m_ActionMapping[static_cast<u32>(mappedInput)]] != 0)
    {
        return true;
    }
    return false;
}

bool Input::isActionActiveDigital(MappedInput mappedInput, u32 eventType)
{
    if (callback.m_InputData.digital[m_ActionMapping[static_cast<u32>(mappedInput)]] == eventType)
    {
        return true;
    }
    return false;
}

bool Input::isActionActiveAnalog(MappedInput mappedInput, f32 value)
{
    if (callback.m_InputData.analog[m_ActionMapping[static_cast<u32>(mappedInput)]] == value)
    {
        return true;
    }
    return false;
}

Window::Window()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    p_Window = glfwCreateWindow(callback.m_WindowSize.x, callback.m_WindowSize.y, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(p_Window);
    glfwSetInputMode(p_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glewInit();

    glfwSetWindowUserPointer(p_Window, &callback);
    for (i32 i = 0; i <= GLFW_JOYSTICK_3; ++i)
    {
        glfwSetJoystickUserPointer(i, this);
        if (glfwJoystickPresent(i))
        {
            callback.m_GamepadsConnected++;
        }
    }
    glfwSetKeyCallback(p_Window, Callback::updateKeyboardCallback);
    glfwSetMouseButtonCallback(p_Window, Callback::updateMouseButtonCallback);
    glfwSetScrollCallback(p_Window, Callback::updateScrollWheelCallback);

    glfwSetCursorPosCallback(p_Window, Callback::windowMousePositionEvent);
    glfwSetFramebufferSizeCallback(p_Window, Callback::windowResizeEvent);

    glfwSetCursorPos(p_Window, 0.0, 0.0);
    glfwSetInputMode(p_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    enableReportGlErrors();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glCullFace(GL_BACK);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
}

Window::~Window()
{
    glfwDestroyWindow(p_Window);
    glfwTerminate();
}

glm::vec2& Window::getWindowSize()
{
    return callback.m_WindowSize;
}

glm::vec2 Window::getMousePosition()
{
    f64 mouseX, mouseY;
    glfwGetCursorPos(p_Window, &mouseX, &mouseY);
    return glm::vec2((f32)mouseX, (f32)mouseY);
}

glm::vec3 Window::getWorldMousePosition(Viewport& viewport)
{
    f32 my;
    my = window.getWindowSize().y - window.getMousePosition().y;

    float x = (2.0f * window.getMousePosition().x) / window.getWindowSize().x - 1.0f;
    float y = (2.0f * my) / window.getWindowSize().y - 1.0f;

    glm::vec4 startNDC(x, y, -1.0f, 1.0f);
    glm::vec4 endNDC(x, y, 1.0f, 1.0f);

    glm::mat4 invVP = glm::inverse(viewport.getProjection() * viewport.getView());

    glm::vec4 startW = invVP * startNDC;
    startW /= startW.w;
    glm::vec4 endW = invVP * endNDC;
    endW /= endW.w;

    glm::vec3 rayOrigin = glm::vec3(startW);
    glm::vec3 rayDir = glm::normalize(glm::vec3(endW - startW));

    // Intersect with Z=0 plane
    float t = -rayOrigin.z / rayDir.z;
    glm::vec3 worldPos = rayOrigin + t * rayDir;

    return worldPos;
}

void Window::SwapBuffer()
{
    glfwSwapBuffers(p_Window);
}

void Window::PollEvents()
{
    input.pollInputs();
    glfwPollEvents();
    Time.update();
    if (input.isActionActive(MappedInput::ESC))
    {
        glfwSetWindowShouldClose(p_Window, true);
    }
}

GLFWwindow*& Window::getWindowPointer()
{
    return p_Window;
}


GLFWmonitor* Window::getCurrentMonitor()
{
    int nmonitors, i;
    int wx, wy, ww, wh;
    int mx, my, mw, mh;
    int overlap, bestoverlap;
    GLFWmonitor* bestmonitor;
    GLFWmonitor** monitors;
    const GLFWvidmode* mode;

    bestoverlap = 0;
    bestmonitor = NULL;

    glfwGetWindowPos(p_Window, &wx, &wy);
    glfwGetWindowSize(p_Window, &ww, &wh);
    monitors = glfwGetMonitors(&nmonitors);

    for (i = 0; i < nmonitors; i++)
    {
        mode = glfwGetVideoMode(monitors[i]);
        glfwGetMonitorPos(monitors[i], &mx, &my);
        mw = mode->width;
        mh = mode->height;

        overlap =
            std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx)) *
            std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));

        if (bestoverlap < overlap)
        {
            bestoverlap = overlap;
            bestmonitor = monitors[i];
        }
    }

    return bestmonitor;
}

void Window::close()
{
    glfwSetWindowShouldClose(p_Window, true);
}