#include "grass.h"
#include "level.h"
#include "texture.h"
#include "material.h"
#include "random.h"
#include "geometry.h"
#include "config.h"

Grass::Grass(const Level* level) : m_time(0.0f), m_count(0), m_grassTex(NULL)
{
    const float grass_density = 0.5f;
    
    //size of grass face
    static const float SIZE = 0.4f;

    const Vector lower(-3.2f, 0.0f, -3.2f);
    const Vector upper(3.2f, 0.0f, 3.2f);
  
    for each_const(TriangleVector, level->m_triangles, iter)
    {
        const Triangle& triangle = *iter;
        
        Vector side1 = Vector(triangle.f1->v) - Vector(triangle.f0->v);
        Vector side2 = Vector(triangle.f2->v) - Vector(triangle.f0->v);
        Vector areaV = side1 ^ side2;
        float area = areaV.magnitude();
        areaV.norm();
        float count = grass_density * area;

        if (count >= 1.0f || (count < 1.0f && Randoms::getFloat() < count))
        {
            for (int n = 0; n < count; n++)
            {
                float s = Randoms::getFloat();
                float t = Randoms::getFloat();
                
                // mapping from [0,1]x[0,1] square to triangle
                t = sqrt(t);
                float a = 1 - t;
                float b = (1 - s)*t;
                float c = s * t;

                Vector v(a * Vector(triangle.f0->v) + b * Vector(triangle.f1->v) + c * Vector(triangle.f2->v));

                if (!(isPointInRectangle(v, lower, upper)))
                {
                    const Matrix trM = Matrix::translate(v) * Matrix::rotateY(Randoms::getFloatN(2*M_PI));

                    m_faces.push_back(Face( UV(0.0f, 0.0f), areaV, trM * Vector(-SIZE/2, 0.0f, 0.0f)) );
                    m_faces.push_back(Face( UV(1.0f, 0.0f), areaV, trM * Vector(+SIZE/2, 0.0f, 0.0f)) );
                    m_faces.push_back(Face( UV(1.0f, 1.0f), areaV, trM * Vector(+SIZE/2, SIZE, 0.0f)) );

                    m_faces.push_back(Face( UV(0.0f, 0.0f), areaV, trM * Vector(-SIZE/2, 0.0f, 0.0f)) );
                    m_faces.push_back(Face( UV(1.0f, 1.0f), areaV, trM * Vector(+SIZE/2, SIZE, 0.0f)) );
                    m_faces.push_back(Face( UV(0.0f, 1.0f), areaV, trM * Vector(-SIZE/2, SIZE, 0.0f)) );

                    m_faces.push_back(Face( UV(0.0f, 0.0f), areaV, trM * Vector(0.0f, 0.0f, -SIZE/2)) );
                    m_faces.push_back(Face( UV(1.0f, 0.0f), areaV, trM * Vector(0.0f, 0.0f, +SIZE/2)) );
                    m_faces.push_back(Face( UV(1.0f, 1.0f), areaV, trM * Vector(0.0f, SIZE, +SIZE/2)) );

                    m_faces.push_back(Face( UV(0.0f, 0.0f), areaV, trM * Vector(0.0f, 0.0f, -SIZE/2)) );
                    m_faces.push_back(Face( UV(1.0f, 1.0f), areaV, trM * Vector(0.0f, SIZE, +SIZE/2)) );
                    m_faces.push_back(Face( UV(0.0f, 1.0f), areaV, trM * Vector(0.0f, SIZE, -SIZE/2)) );
                }

            }
        }
    }

    m_count = m_faces.size();

    m_grassTex = Video::instance->loadTexture("grassThingy", Texture::ClampToEdge);
}

Grass::~Grass()
{
}

void Grass::render() const
{
    if (m_count == 0)
    {
        return;
    }

    m_grassTex->bind();

    glDisable(GL_CULL_FACE);
    glAlphaFunc(GL_GEQUAL, 0.3f);
    glEnable(GL_ALPHA_TEST);

    glTexCoordPointer(2, GL_FLOAT, sizeof(Face), &m_faces[0].tc[0]);
    glNormalPointer(GL_FLOAT,      sizeof(Face), &m_faces[0].n[0]);
    glVertexPointer(3, GL_FLOAT,   sizeof(Face), &m_faces[0].v[0]);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_count));

    glEnable(GL_CULL_FACE);
    glDisable(GL_ALPHA_TEST);
}
