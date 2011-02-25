#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#include "common.h"
#include "vmath.h"
#include "system.h"
#include "texture.h"

class Material;
class Collision;

struct UV
{
    UV(float u = 0.0f, float v = 0.0f) : u(u), v(v) {}

    float u, v;
};

struct Face
{
    Face(const UV& uv, const Vector& normal, const Vector& vertex)
    {
        tc[0] = uv.u;
        tc[1] = uv.v;
        n[0] = normal.x;
        n[1] = normal.y;
        n[2] = normal.z;
        v[0] = vertex.x;
        v[1] = vertex.y;
        v[2] = vertex.z;
    }

    float tc[2];
    float n[3];
    float v[3];
};

typedef vector<Face> FaceVector;

typedef map<string, Texture*> TextureMap;

class Video : public System<Video>, public NoCopy
{
public:
    Video();
    ~Video();

    void init();
    void unloadTextures();

    void renderFace(const FaceVector& faces) const;
    void renderFace(const FaceVector& faces, int first, int count) const;
    void renderRoundRect(const Vector& lower, const Vector& upper, float r) const;
    void renderSimpleShadow(float r, const Vector& pos, const Collision* level, const Vector& color) const;

    void resize(int width, int height);
    void setColor(const Vector& color);
    const Vector& getColor() const;

    void setViewport(int width, int height);
    void getViewport(int& width, int& height) const;
    
    void setModelViewMatrix(const Matrix& matrix);
    void setProjectionMatrix(const Matrix& matrix);
    const Matrix& getProjectionMatrix() const;
    const Matrix& getModelViewMatrix() const;

    void begin() const;
    void begin(const Matrix& matrix) const;
    void end() const;

    void bind(const Material* material) const;

    IntPair getResolution() const;

    Texture* loadTexture(const string& name, Texture::WrapType wrap = Texture::Repeat);

private:
    TextureMap    m_textures;
    IntPair       m_resolution;    
    Vector        m_color;
    int           m_width;
    int           m_height;

    Matrix        m_modelview;
    Matrix        m_projection;

    vector<float> m_circleSin;
    vector<float> m_circleCos;
};

unsigned char* loadImg(const string& fname, int& w, int& h, int& type);

#endif
