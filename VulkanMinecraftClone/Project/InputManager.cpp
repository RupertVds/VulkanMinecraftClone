#include "InputManager.h"

void InputManager::Init(GLFWwindow* window)
{
    m_Window = window;

    int tempX, tempY{};
    glfwGetWindowSize(window, &tempX, &tempY);
    lastX = tempX / 2.f;
    lastY = tempY / 2.f;
    firstMouse = true;

    m_IsFPS = false;

    (m_IsFPS) ? glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED) :
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
            if (inputManager)
            {
                if (action == GLFW_PRESS)
                {
                    inputManager->m_KeyPressedMap[key] = true;
                    inputManager->m_KeyHeldMap[key] = true;
                    inputManager->m_KeyReleasedMap[key] = false;
                }
                else if (action == GLFW_RELEASE)
                {
                    inputManager->m_KeyPressedMap[key] = false;
                    inputManager->m_KeyHeldMap[key] = false;
                    inputManager->m_KeyReleasedMap[key] = true;
                }
            }
        }
    );

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
        {
            InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
            if (inputManager)
            {
                if (action == GLFW_PRESS)
                {
                    inputManager->m_MouseButtonPressedMap[button] = true;
                    inputManager->m_MouseButtonReleasedMap[button] = false;
                }
                else if (action == GLFW_RELEASE)
                {
                    inputManager->m_MouseButtonPressedMap[button] = false;
                    inputManager->m_MouseButtonReleasedMap[button] = true;
                }
            }
        }
    );
    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos)
        {
            InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
            if (inputManager)
            {
                inputManager->m_MouseX = xpos;
                inputManager->m_MouseY = ypos;
            }
        }
    );

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
        {
            InputManager* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
            if (inputManager)
            {
                inputManager->m_ScrollX = xoffset;
                inputManager->m_ScrollY = yoffset;
            }
        });
}

bool InputManager::IsMouseButtonPressed(int button)
{
    if (m_MouseButtonPressedMap[button])
    {
        m_MouseButtonPressedMap[button] = false;
        return true;
    }
    return false;
}

bool InputManager::IsMouseButtonReleased(int button)
{
    if (m_MouseButtonReleasedMap[button])
    {
        m_MouseButtonReleasedMap[button] = false;
        return true;
    }
    return false;
}

std::pair<double, double> InputManager::GetMousePos()
{
    return std::make_pair(m_MouseX, m_MouseY);
}
std::pair<double, double> InputManager::GetScrollOffset()
{
    return std::make_pair(m_ScrollX, m_ScrollY);
}

void InputManager::ResetScrollOffset()
{
    m_ScrollX = 0;
    m_ScrollY = 0;
}

bool InputManager::IsKeyPressed(int key)
{
    if (m_KeyPressedMap[key])
    {
        m_KeyPressedMap[key] = false;
        return true;
    }
    return false;
}

bool InputManager::IsKeyHeld(int key)
{
    return m_KeyHeldMap[key];
}

bool InputManager::IsKeyReleased(int key)
{
    if (m_KeyReleasedMap[key])
    {
        m_KeyReleasedMap[key] = false;
        return true;
    }
    return false;
}

void InputManager::SetFPSMode(bool state)
{
    if (state)
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void InputManager::ToggleFPSMode()
{
    m_IsFPS = !m_IsFPS;

    if (m_IsFPS)
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}