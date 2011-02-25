#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "common.h"
#include "vmath.h"

class Camera : public NoCopy
{
public:
    Camera(const Vector& pos, float angleX, float angleY);
    ~Camera();  

    void control(int rotateX, int rotateY);
    void update(float delta);
    void prepare();
    void render() const;

    float angleY() const;
    const Matrix& getModelViewMatrix() const;

private:
    Vector m_targetRotation;
    Vector m_targetDirection;

    Vector m_pos;
    float  m_angleX;
    float  m_angleY;

    const Matrix m_strafeRotation;
    const Matrix m_scaleMatrix;
    Matrix m_matrix;

    const Vector m_defPos;
    const float  m_defAngleX;
    const float  m_defAngleY;
};

#endif
