#include "skybox.h"
#include "texture.h"

SkyBox::SkyBox(const string& name)
{      
    static const float vertices[][3] = {
        { -0.5f, -0.5f, -0.5f }, // 0
        {  0.5f, -0.5f, -0.5f }, // 1
        {  0.5f, -0.5f,  0.5f }, // 2
        { -0.5f, -0.5f,  0.5f }, // 3
        
        { -0.5f, 0.5f, -0.5f },  // 4
        {  0.5f, 0.5f, -0.5f },  // 5
        {  0.5f, 0.5f,  0.5f },  // 6
        { -0.5f, 0.5f,  0.5f },  // 7
    };
    
    static const int faces[][4] = {
        { 5, 6, 1, 2 }, // front
        { 7, 4, 3, 0 }, // back
        { 4, 5, 0, 1 }, // right
        { 6, 7, 2, 3 }, // left
    };
    
    static const float uv[][2] = {
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
    };
    
    static const float translate[][2] = {
        { 0.5f, 0.5f },
        { 0.5f, 0.0f },
        { 0.0f, 0.5f },
        { 0.0f, 0.0f },
    };

    m_data.reserve(4 * 2 * 3 * (3 + 2));
    for (int i = 0; i < 4; i++)
    {
        static const int idx[] = { 0, 1, 2, 1, 2, 3 };
        
        for (int k = 0; k < 6; k++)
        {
            const float* v = vertices[faces[i][idx[k]]];
            
            m_data.push_back(90.0f * v[0]);
            m_data.push_back(90.0f * v[1]);
            m_data.push_back(90.0f * v[2]);
            
            float uu = uv[idx[k]][0] * 0.5f + translate[i][0];
            float vv = uv[idx[k]][1] * 0.5f + translate[i][1];
            if (uu > 0.9f) uu = 1.0f - 1.5f / 1024.0f;
            if (uu < 0.1f) uu = 1.5f / 1024.0f; 
            
            m_data.push_back(uu);
            m_data.push_back(vv);
        }
    }
    
    m_texture = Video::instance->loadTexture("cubemap/" + name, Texture::ClampToEdge);
}

void SkyBox::render(const Matrix& modelviewMatrix) const
{
    Matrix modelview =  modelviewMatrix;
    
    glPushMatrix();
    
    modelview.m30 = modelview.m31 = modelview.m32 = 0.0f;
    glLoadMatrixf(modelview.m);
    glRotatef(-40.0f, 0.0f, 1.0f, 0.0f);
    
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    
    Video::instance->setColor(Vector::One);
    
    m_texture->bind();
    
    glDisableClientState(GL_NORMAL_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, (3+2)*sizeof(float), &m_data[3]);
    glVertexPointer(3, GL_FLOAT, (3+2)*sizeof(float), &m_data[0]);
    glDrawArrays(GL_TRIANGLES, 0, 4 * 2 * 3);
    glEnableClientState(GL_NORMAL_ARRAY);

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
}
