#pragma once

#include <unordered_map>
#include <GLFW\glfw3.h>

class InputManager final
{
public:
    static InputManager& GetInstance()
    {
        static InputManager instance;
        return instance;
    }

    //InputManager(GLFWwindow* window);
    InputManager(const InputManager& other) = delete;
    InputManager& operator=(const InputManager& other) = delete;
    InputManager(InputManager&& other) = delete;
    InputManager& operator=(InputManager&& other) = delete;
    ~InputManager() = default;
public:
    void Init(GLFWwindow* window);

    bool IsMouseButtonPressed(int button);
    bool IsMouseButtonReleased(int button);

    std::pair<double, double> GetMousePos();
    std::pair<double, double> GetScrollOffset();
    
    // must be called after every GetScrollEvent()
    void ResetScrollOffset();

    bool IsKeyPressed(int key);
    bool IsKeyHeld(int key);
    bool IsKeyReleased(int key);

    void SetFPSMode(bool state);
    void ToggleFPSMode();

    float lastX;
    float lastY;
    bool firstMouse;
    bool m_IsFPS;
private:
    InputManager() = default;

    GLFWwindow* m_Window;

    double m_MouseX{};
    double m_MouseY{};

    double m_ScrollX{};
    double m_ScrollY{};

    std::unordered_map<int, bool> m_MouseButtonPressedMap;
    std::unordered_map<int, bool> m_MouseButtonReleasedMap;

    std::unordered_map<int, bool> m_KeyPressedMap;
    std::unordered_map<int, bool> m_KeyHeldMap;
    std::unordered_map<int, bool> m_KeyReleasedMap;
};

