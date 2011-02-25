#include "camera.h"
#include "world.h"
#include "level.h"
#include "body.h"

static const float LOOK_SPEED = 0.05f;
static const float MOVE_SPEED = 10.0f;

Camera::Camera(const Vector& pos, float angleX, float angleY) :
    m_targetRotation(),
    m_targetDirection(),
    m_pos(-pos), 
    m_angleX(angleX * DEG_IN_RAD), 
    m_angleY(angleY * DEG_IN_RAD),
    m_strafeRotation(Matrix::rotateY(-M_PI/2)),
    m_scaleMatrix(Matrix::scale(Vector(1.0f, 1.0f, -1.0f))),
    m_matrix(),
    m_defPos(-pos),
    m_defAngleX(angleX * DEG_IN_RAD),
    m_defAngleY(angleY * DEG_IN_RAD)
{
}

Camera::~Camera()
{
}

void Camera::control(int rotateX, int rotateY)
{
    m_targetRotation.y = (float)rotateX / 5.0f;
    m_targetRotation.x = (float)rotateY / 5.0f;

    m_targetDirection = Vector::Zero;

    // iTODO: zoom
    //m_targetDirection.z = static_cast<float>(mouse.z);
}

void Camera::update(float delta)
{
    //m_targetRotation *= delta;
    m_targetDirection *= delta;

    float t = 50.0f * LOOK_SPEED * m_targetDirection.z;
    if (t > 0)
    {
        m_pos *= 1.0f - t;
    }
    else if (t < 0)
    {
        m_pos *= 1.0f - t;

    }
    if (m_pos.z > 0.0f)
    {
        m_pos.z = -m_pos.z;
    }
    if (m_pos.magnitude2() < 4.0f*4.0f)
    {
        m_pos.norm();
        m_pos *= 4.0f;
    }
    else if (m_pos.magnitude2() > 17.0f*17.0f)
    {
        m_pos.norm();
        m_pos *= 17.0f;
    }

    m_angleY += LOOK_SPEED * m_targetRotation.y;
    m_angleX += LOOK_SPEED * m_targetRotation.x;
    
    if (m_angleX < 9.0f * DEG_IN_RAD)
    {
        m_angleX = 9.0f * DEG_IN_RAD;
    }
    else if (m_angleX > 60.0f * DEG_IN_RAD)
    {
        m_angleX = 60.0f * DEG_IN_RAD;
    }
}

void Camera::prepare()
{
    m_matrix = Matrix::translate(m_pos)  * 
               Matrix::rotateX(m_angleX) *
               Matrix::rotateY(m_angleY) *
               m_scaleMatrix;
}

void Camera::render() const
{
    glLoadMatrixf(m_matrix.m);
}

float Camera::angleY() const
{
    return m_angleY;
}

const Matrix& Camera::getModelViewMatrix() const
{
    return m_matrix;
}
