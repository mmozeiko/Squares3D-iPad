#ifndef __MESH_H__
#define __MESH_H__

#include "common.h"
#include "vmath.h"
#include "video.h"

class Mesh
{
public:
    Mesh(GLenum mode, bool indexed);
    ~Mesh();
  
    void render() const;

protected:
    void init();

    FaceVector   m_faces; 
    UShortVector m_indices;

private:
    GLuint  m_buffers[2];

    GLenum  m_mode;
    GLsizei m_indicesCount;
    bool    m_indexed;
};

class CubeMesh : public Mesh
{
public:
    CubeMesh(const Vector& size);
};

class SphereMesh : public Mesh
{
public:
    SphereMesh(const Vector& radius, int stacks, int slices);
};

class CylinderMesh : public Mesh
{
public:
    CylinderMesh(float radius, float height, int stacks, int slices);
};

class ConeMesh : public Mesh
{
public:
    ConeMesh(float radius, float height, int stacks, int slices);
};

#endif
