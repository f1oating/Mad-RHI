#include "Common/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace mad::common {

Camera::Camera(glm::vec3 pos, float fov, float aspect, float near, float far)
    : m_Position(pos), m_Fov(fov), m_Aspect(aspect), m_NearPlane(near), 
      m_FarPlane(far), m_WorldUp(0, 1, 0), m_Yaw(-90.0f), m_Pitch(0.0f)
{
    UpdateVectors();
}

void Camera::UpdateVectors() 
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

    m_Front = glm::normalize(front);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

glm::vec3 Camera::GetPosition() const
{
    return m_Position;
}

glm::mat4 Camera::GetView() const 
{
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::GetProjection() const 
{
    return glm::perspective(glm::radians(m_Fov), m_Aspect, m_NearPlane, m_FarPlane);
}

void Camera::MoveForward(float dt) 
{ 
    m_Position += m_Front * m_Speed * dt; 
}

void Camera::MoveBack(float dt) 
{ 
    m_Position -= m_Front * m_Speed * dt; 
}

void Camera::MoveRight(float dt)
{ 
    m_Position += m_Right * m_Speed * dt; 
}

void Camera::MoveLeft(float dt)
{ 
    m_Position -= m_Right * m_Speed * dt; 
}

void Camera::Rotate(float dx, float dy, float sensitivity) 
{
    m_Yaw += dx * sensitivity;
    m_Pitch -= dy * sensitivity;

    m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);

    UpdateVectors();
}

}