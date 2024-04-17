#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class InputManager;

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 5.f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
const float MAX_MOVE_SPEED = 100.f;
const float MIN_MOVE_SPEED = 1.f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices
class Camera final
{
public:
    static Camera& GetInstance()
    {
        static Camera instance;
        return instance;
    }
public:
    void Init(InputManager* inputManager, glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
    void Update(float deltaTime);

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix();

public:
    // camera Attributes
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;
    // euler Angles
    float m_Yaw;
    float m_Pitch;
    // camera options
    float m_MovementSpeed;
    float m_MouseSensitivity;
    float m_Zoom;

private:
    Camera() = default;
    // Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
    enum Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

private:
    // processes input received from any keyboard-like input system. 
    // Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset);
    // calculates the front vector from the Camera's (updated) Euler Angles
    void UpdateCameraVectors();

private:
    InputManager* m_InputManager;
};