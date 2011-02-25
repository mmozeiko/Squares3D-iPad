#include "mesh.h"

Mesh::Mesh(GLenum mode, bool indexed) :
    m_mode(mode),
    m_indexed(indexed)
{
}

void Mesh::init()
{
    m_indicesCount = static_cast<GLsizei>(m_indexed ? m_indices.size() : m_faces.size());
}

Mesh::~Mesh()
{
}

void Mesh::render() const
{
    glTexCoordPointer(2, GL_FLOAT, sizeof(Face), &m_faces[0].tc[0]);
    glNormalPointer(GL_FLOAT,      sizeof(Face), &m_faces[0].n[0]);
    glVertexPointer(3, GL_FLOAT,   sizeof(Face), &m_faces[0].v[0]);

    if (m_indexed)
    {
        glDrawElements(m_mode, m_indicesCount, GL_UNSIGNED_SHORT, &m_indices[0]);
    }
    else
    {
        glDrawArrays(m_mode, 0, m_indicesCount);
    }
}

CubeMesh::CubeMesh(const Vector& size) : Mesh(GL_TRIANGLES, false)
{
    // -0.5 .. 0.5
    static const float vertices[][3] = {
        /* 0 */ { -0.5, -0.5, -0.5 },
        /* 1 */ {  0.5, -0.5, -0.5 },
        /* 2 */ {  0.5, -0.5,  0.5 },
        /* 3 */ { -0.5, -0.5,  0.5 },

        /* 4 */ { -0.5,  0.5, -0.5 },
        /* 5 */ {  0.5,  0.5, -0.5 },
        /* 6 */ {  0.5,  0.5,  0.5 },
        /* 7 */ { -0.5,  0.5,  0.5 },
    };

    static const int faces[][6] = {
        { 0, 1, 2, 3, 0, 2 }, // bottom
        { 4, 7, 6, 5, 4, 6 }, // up
        { 4, 5, 1, 0, 4, 1 }, // front
        { 6, 7, 3, 2, 6, 3 }, // back
        { 7, 4, 0, 3, 7, 0 }, // left
        { 5, 6, 2, 1, 5, 2 }, // right
    };
    
    static const float normals[][3] = {
        {  0.0, -1.0,  0.0 }, // bottom
        {  0.0,  1.0,  0.0 }, // up
        {  0.0,  0.0, -1.0 }, // front
        {  0.0,  0.0,  1.0 }, // back
        { -1.0,  0.0,  0.0 }, // left
        {  1.0,  0.0,  0.0 }, // right
    };

    static const float uv[][2] = {
        { 1.0, 0.0 },
        { 0.0, 0.0 },
        { 0.0, 1.0 },
        { 1.0, 1.0 },
        { 1.0, 0.0 },
        { 0.0, 1.0 },
    };

    for (size_t i = 0; i < sizeOfArray(faces); i++)
    {
        for (int k=0; k<6; k++)
        {
            m_faces.push_back(Face(UV(uv[k][0], uv[k][1]),
                                   Vector(normals[i][0], normals[i][1], normals[i][2]),
                                   size * Vector(vertices[faces[i][k]][0], vertices[faces[i][k]][1], vertices[faces[i][k]][2]))
                             );
        }
    }

    init();
}

SphereMesh::SphereMesh(const Vector& radius, int stacks, int slices) : Mesh(GL_TRIANGLE_STRIP, true)
{   
    for (int stackNumber = 0; stackNumber <= stacks; ++stackNumber)
    {
        for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
        {
            float theta = stackNumber * M_PI / stacks;
            float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
            float sinTheta = std::sin(theta);
            float sinPhi = std::sin(phi);
            float cosTheta = std::cos(theta);
            float cosPhi = std::cos(phi);

            float s = static_cast<float>(sliceNumber)/static_cast<float>(slices);
            float t = 1.0f-static_cast<float>(stackNumber)/static_cast<float>(stacks);

            m_faces.push_back(Face(UV(s, t),
                                   Vector(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta),
                                   radius * Vector(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta))
                             );
        }
    }

    for (int stackNumber = 0; stackNumber < stacks; ++stackNumber)
    {
        for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
        {
            m_indices.push_back( stackNumber*(slices+1) + sliceNumber);
            m_indices.push_back( (stackNumber+1)*(slices+1) + sliceNumber);
        }
    }

    init();
}

CylinderMesh::CylinderMesh(float radius, float height, int stacks, int slices) : Mesh(GL_TRIANGLE_STRIP, true)
{
    // bottom disc
    m_faces.push_back(Face(UV(0.5f, 0.5f), -Vector::X, Vector::Zero));
    
    for (int sliceNumber = slices; sliceNumber >= 0; --sliceNumber)
    {
        float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        m_faces.push_back(Face(UV(cosPhi, sinPhi),
                               -Vector::X,
                               Vector(0.0f, radius * cosPhi, radius * sinPhi))
                         );
    }

    // cylinder
    for (int stackNumber = 0; stackNumber <= stacks; ++stackNumber)
    {
        for (int sliceNumber = slices; sliceNumber >=0 ; --sliceNumber)
        {
            float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            float s = static_cast<float>(sliceNumber)/static_cast<float>(slices);
            float t = 1.0f-static_cast<float>(stackNumber)/static_cast<float>(stacks);

            m_faces.push_back(Face(UV(s, t),
                                   Vector(0.0f, cosPhi, sinPhi),
                                   Vector(height * stackNumber / stacks, radius * cosPhi, radius * sinPhi))
                             );
        }
    }

    // upper disc
    for (int sliceNumber = slices; sliceNumber >= 0; --sliceNumber)
    {
        float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        m_faces.push_back(Face(UV(cosPhi, sinPhi),
                               Vector::X,
                               Vector(height, radius * cosPhi, radius * sinPhi))
                         );
    }
    m_faces.push_back(Face(UV(0.5f, 0.5f), Vector::X, Vector(height, 0.0f, 0.0f)));


    // bottom disc
    for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
    {
        m_indices.push_back( 0 );
        m_indices.push_back( 1 + sliceNumber );
    }

    // cylinder
    for (int stackNumber = 0; stackNumber < stacks; ++stackNumber)
    {
        for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
        {
            m_indices.push_back( (1+slices+1) + stackNumber*(slices+1) + sliceNumber);
            m_indices.push_back( (1+slices+1) + (stackNumber+1)*(slices+1) + sliceNumber);
        }
    }

    // upper disc
    for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
    {
        m_indices.push_back( (1+slices+1) + (stacks+1)*(slices+1) + sliceNumber );
        m_indices.push_back( (1+slices+1) + (stacks+1)*(slices+1) + (slices+1) );
    }

    init();
}

ConeMesh::ConeMesh(float radius, float height, int stacks, int slices) : Mesh(GL_TRIANGLE_STRIP, true)
{
    // bottom disc
    m_faces.push_back(Face(UV(0.5f, 0.5f), -Vector::X, Vector::Zero));
    
    for (int sliceNumber = slices; sliceNumber >= 0; --sliceNumber)
    {
        float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        m_faces.push_back(Face(UV(cosPhi, sinPhi),
                               -Vector::X,
                               Vector(0.0f, radius * cosPhi, radius * sinPhi))
                         );
    }

    // cone
    for (int stackNumber = 0; stackNumber <= stacks; ++stackNumber)
    {
        for (int sliceNumber = slices; sliceNumber >=0 ; --sliceNumber)
        {
            float phi = sliceNumber * 2 * M_PI / slices + M_PI_2;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);
            float s = static_cast<float>(sliceNumber)/static_cast<float>(slices);
            float t = 1.0f-static_cast<float>(stackNumber)/static_cast<float>(stacks);

            m_faces.push_back(Face(UV(s, t),
                                   Vector(0.0f, cosPhi, sinPhi),
                                   Vector(height * (1.0f - t), radius * cosPhi * t, radius * sinPhi * t))
                             );
        }
    }

    // bottom disc
    for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
    {
        m_indices.push_back( 0 );
        m_indices.push_back( 1 + sliceNumber );
    }

    // cone
    for (int stackNumber = 0; stackNumber < stacks; ++stackNumber)
    {
        for (int sliceNumber = 0; sliceNumber <= slices; ++sliceNumber)
        {
            m_indices.push_back( (1+slices+1) + stackNumber*(slices+1) + sliceNumber);
            m_indices.push_back( (1+slices+1) + (stackNumber+1)*(slices+1) + sliceNumber);
        }
    }

    init();
}
