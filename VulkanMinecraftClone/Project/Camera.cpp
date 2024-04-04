#include "Camera.h"
#include "InputManager.h"
#include <GLFW\glfw3.h>
#include <iostream>

void Camera::Init(InputManager* inputManager, glm::vec3 position, glm::vec3 up, float yaw, float pitch)
{
    m_InputManager = inputManager;
    m_Position = position;
    m_Front = glm::vec3{ 0.0f, 0.0f, -1.0f };
    m_MovementSpeed = SPEED;
    m_MouseSensitivity = SENSITIVITY;
    m_Zoom = ZOOM;
    m_WorldUp = up;
    m_Yaw = yaw;
    m_Pitch = pitch;
    UpdateCameraVectors();
}

void Camera::Update(float deltaTime)
{

    if (m_InputManager->IsKeyHeld(GLFW_KEY_W))
    {
        ProcessKeyboard(FORWARD, deltaTime);
    }
    if (m_InputManager->IsKeyHeld(GLFW_KEY_S))
    {
        ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (m_InputManager->IsKeyHeld(GLFW_KEY_A))
    {
        ProcessKeyboard(LEFT, deltaTime);
    }
    if (m_InputManager->IsKeyHeld(GLFW_KEY_D))
    {
        ProcessKeyboard(RIGHT, deltaTime);
    }
    if (m_InputManager->IsKeyHeld(GLFW_KEY_E))
    {
        m_Position.y += m_MovementSpeed * deltaTime;;
    }
    if (m_InputManager->IsKeyHeld(GLFW_KEY_Q))
    {
        m_Position.y -= m_MovementSpeed * deltaTime;;
    }
    if (m_InputManager->IsKeyPressed(GLFW_KEY_F))
    {
        m_MovementSpeed += 1.f;
        if (m_MovementSpeed > 20.f)
            m_MovementSpeed = 20.f;
        if (m_MovementSpeed < 1.f)
            m_MovementSpeed = 1.f;
    }
    if (m_InputManager->IsKeyPressed(GLFW_KEY_G))
    {
        m_MovementSpeed -= 1.f;
        if (m_MovementSpeed > 20.f)
            m_MovementSpeed = 20.f;
        if (m_MovementSpeed < 1.f)
            m_MovementSpeed = 1.f;
    }

    float xpos = static_cast<float>(m_InputManager->GetMousePos().first);
    float ypos = static_cast<float>(m_InputManager->GetMousePos().second);

    if (m_InputManager->firstMouse)
    {
        m_InputManager->lastX = xpos;
        m_InputManager->lastY = ypos;
        m_InputManager->firstMouse = false;
    }

    float xoffset = xpos - m_InputManager->lastX;
    float yoffset = m_InputManager->lastY - ypos; // reversed since y-coordinates go from bottom to top

    m_InputManager->lastX = xpos;
    m_InputManager->lastY = ypos;

    if (m_InputManager->m_IsFPS)
    {
        ProcessMouseMovement(xoffset, yoffset);
    }

    ProcessMouseScroll(static_cast<float>(m_InputManager->GetScrollOffset().second));
    
    std::cout << "pos x:" << m_Position.x;
    std::cout << ", pos y:" << m_Position.y;
    std::cout << ", pos z:" << m_Position.z << '\n';
    std::cout << "pitch:" << m_Pitch;
    std::cout << ", yaw:" << m_Yaw << '\n';
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = m_MovementSpeed * deltaTime;
    if (direction == FORWARD)
        m_Position += m_Front * velocity;
    if (direction == BACKWARD)
        m_Position -= m_Front * velocity;
    if (direction == LEFT)
        m_Position -= m_Right * velocity;
    if (direction == RIGHT)
        m_Position += m_Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= m_MouseSensitivity;
    yoffset *= m_MouseSensitivity;

    //m_Yaw += xoffset;
    m_Yaw = glm::mod(m_Yaw + xoffset, 360.0f);
    m_Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    m_Zoom -= yoffset;
    if (m_Zoom < 1.0f)
        m_Zoom = 1.0f;
    if (m_Zoom > 45.0f)
        m_Zoom = 45.0f;
    m_InputManager->ResetScrollOffset();
}

void Camera::UpdateCameraVectors()
{
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}
