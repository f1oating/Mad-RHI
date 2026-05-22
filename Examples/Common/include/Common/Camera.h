#pragma once

#include <glm/glm.hpp>

namespace mad::common {

class Camera 
{
public:
    Camera(glm::vec3 pos, float fov, float aspect, float near, float far);

    glm::vec3 GetPosition() const;
    glm::mat4 GetView() const;
    glm::mat4 GetProjection() const;

    void MoveForward(float dt);
    void MoveBack(float dt);
    void MoveRight(float dt);
    void MoveLeft(float dt);
    void Rotate(float dx, float dy, float sensitivity = 0.1f);

    void SetAspectRatio(float aspect);

private:
    glm::vec3 m_Position;
    glm::vec3 m_Front, m_Right, m_Up;
    glm::vec3 m_WorldUp = {0, 1, 0};

    float m_Yaw = -90.0f;
    float m_Pitch = 0.0f;
    float m_Speed = 0.005f;

    float m_Fov, m_Aspect, m_NearPlane, m_FarPlane;

private:
    void UpdateVectors();

};


}