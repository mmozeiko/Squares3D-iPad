#include "video.h"
#include "config.h"
#include "file.h"
#include "level.h"
#include "material.h"
#include "texture.h"
#include "geometry.h"
#include "collision.h"
#include "input.h"
#include "zlib.h"
#include "file.h"

static const int CIRCLE_DIVISIONS = 12;

template <class Video> Video* System<Video>::instance = NULL;

void Video::resize(int width, int height)
{
    glViewport(0, 0, height, width);
    Video::instance->setViewport(width, height);

    const float fov = 45.0f;
    const float aspect = static_cast<float>(height)/static_cast<float>(width);
    const float t = std::tan(fov/2 * DEG_IN_RAD) * 0.1f;
    const float b = -t;
    const float l = aspect * b;
    const float r = aspect * t;

    float n = 0.1f;
    float f = 80.0f;

    Matrix projection;
    projection.m00 = 2*n/(r-l);
    projection.m10 = 0;
    projection.m20 = (r+l)/(r-l);
    projection.m30 = 0;
    
    projection.m01 = 0;
    projection.m11 = 2*n/(t-b);
    projection.m21 = (t+b)/(t-b);
    projection.m31 = 0;

    projection.m02 = 0;
    projection.m12 = 0;
    projection.m22 = -(f+n)/(f-n);
    projection.m32 = -2*f*n/(f-n);

    projection.m03 = 0;
    projection.m13 = 0;
    projection.m23 = -1;
    projection.m33 = 0;
    Video::instance->setProjectionMatrix(projection);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection.m);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.0f, -(r-l), 0.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    Video::instance->setModelViewMatrix(Matrix::identity());
}

Video::Video()
{
    setInstance(this);

    clog << "Initializing video." << endl;

    clog << " * Version    : " << glGetString(GL_VERSION) << endl
         << " * Vendor     : " << glGetString(GL_VENDOR) << endl
         << " * Renderer   : " << glGetString(GL_RENDERER) << endl
         << " * Extensions : " << glGetString(GL_EXTENSIONS) << endl;

    resize(1024, 768);
    
    m_resolution = make_pair(1024, 768);

    for (int i=CIRCLE_DIVISIONS; i>=0; i--)
    {
        m_circleSin.push_back(std::sin(i*2.0f*M_PI/CIRCLE_DIVISIONS));
        m_circleCos.push_back(std::cos(i*2.0f*M_PI/CIRCLE_DIVISIONS));
    }
}

Video::~Video()
{
    clog << "Closing video." << endl;

    unloadTextures();
}

void Video::init()
{
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY); 
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
    glEnableClientState(GL_NORMAL_ARRAY); 
}

void Video::renderFace(const FaceVector& faces) const
{
    glTexCoordPointer(2, GL_FLOAT, sizeof(Face), &faces[0].tc[0]);
    glNormalPointer(GL_FLOAT, sizeof(Face), &faces[0].n[0]);
    glVertexPointer(3, GL_FLOAT, sizeof(Face), &faces[0].v[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(faces.size()));
}

void Video::renderFace(const FaceVector& faces, int first, int count) const
{
    glTexCoordPointer(2, GL_FLOAT, sizeof(Face), &faces[0].tc[0]);
    glNormalPointer(GL_FLOAT, sizeof(Face), &faces[0].n[0]);
    glVertexPointer(3, GL_FLOAT, sizeof(Face), &faces[0].v[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, first, count);
}

void Video::setViewport(int width, int height)
{
    m_width = width;
    m_height = height;
}

void Video::getViewport(int& width, int& height) const
{
    width = m_width;
    height = m_height;
}

void Video::setModelViewMatrix(const Matrix& matrix)
{
    m_modelview = matrix;
}

void Video::setProjectionMatrix(const Matrix& matrix)
{
    m_projection = matrix;
}

const Matrix& Video::getProjectionMatrix() const
{
    return m_projection;
}

const Matrix& Video::getModelViewMatrix() const
{
    return m_modelview;
}

void Video::setColor(const Vector& color)
{
    m_color = color;
    glColor4f(color.x, color.y, color.z, color.w);
}

const Vector& Video::getColor() const
{
    return m_color;
}

void Video::begin() const
{
    glPushMatrix();
}

void Video::begin(const Matrix& matrix) const
{
    glPushMatrix();
    glMultMatrixf(matrix.m);
}

void Video::end() const
{
    glPopMatrix();
}

void Video::bind(const Material* material) const
{
    if (material != NULL)
    {
        material->bind();
    }
}

Texture* Video::loadTexture(const string& name, Texture::WrapType wrap)
{
    TextureMap::iterator iter = m_textures.find(name);
    if (iter != m_textures.end())
    {
        return iter->second;
    }
    
    Texture* texture = new Texture(name, wrap);
    return m_textures.insert(make_pair(name, texture)).first->second;
}

IntPair Video::getResolution() const
{
    return m_resolution;
}

void Video::unloadTextures()
{
    for each_const(TextureMap, m_textures, iter)
    {
        delete iter->second;
    }
    m_textures.clear();
}

void Video::renderRoundRect(const Vector& lower, const Vector& upper, float r) const
{
    glDisable(GL_TEXTURE_2D);

    glDisableClientState(GL_NORMAL_ARRAY); 
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    std::vector<float> buf;
    buf.reserve(2*(1+9*4));

    buf.push_back((lower + upper).x / 2);
    buf.push_back((lower + upper).y / 2);

    for (int i=0; i<9; i++)
    {
        buf.push_back(lower.x - r*std::sin(i*M_PI_2/8));
        buf.push_back(lower.y - r*std::cos(i*M_PI_2/8));
    }
    for (int i=0; i<9; i++)
    {
        buf.push_back(lower.x - r*std::sin((8-i)*M_PI_2/8));
        buf.push_back(upper.y + r*std::cos((8-i)*M_PI_2/8));
    }
    for (int i=0; i<9; i++)
    {
        buf.push_back(upper.x + r*std::sin(i*M_PI_2/8));
        buf.push_back(upper.y + r*std::cos(i*M_PI_2/8));
    }
    for (int i=0; i<9; i++)
    {
        buf.push_back(upper.x + r*std::sin((8-i)*M_PI_2/8));
        buf.push_back(lower.y - r*std::cos((8-i)*M_PI_2/8));
    }
    
    buf.push_back(lower.x);
    buf.push_back(lower.y - r);
    
    glVertexPointer(2, GL_FLOAT, 0, &buf[0]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, buf.size() / 2);

    glEnableClientState(GL_NORMAL_ARRAY); 
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_TEXTURE_2D);
}

void Video::renderSimpleShadow(float r, const Vector& pos, const Collision* level, const Vector& color) const
{
    float y = 0.01f;

    if (isPointInRectangle(pos, Vector(-3, 0, -3), Vector(3, 0, 3)))
    {
        y += 0.035f;
    }

    glPushMatrix();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableClientState(GL_COLOR_ARRAY); 
    glDisableClientState(GL_NORMAL_ARRAY); 
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    float buf[(CIRCLE_DIVISIONS+2) * 7];
    float* ptr = buf;

    float yy = level->getHeight(pos.x, pos.z) + y;

    *ptr++ = color.x;
    *ptr++ = color.y;
    *ptr++ = color.z;
    *ptr++ = color.w;

    *ptr++ = pos.x;
    *ptr++ = yy;
    *ptr++ = pos.z;

    for (int i=0; i<=CIRCLE_DIVISIONS; i++)
    {
        float xx = r * m_circleCos[i];
        float zz = r * m_circleSin[i];
        float yy = level->getHeight(pos.x+xx, pos.z+zz) + y;

        *ptr++ = color.x;
        *ptr++ = color.y;
        *ptr++ = color.z;
        *ptr++ = color.w - 0.1f;

        *ptr++ = pos.x + xx;
        *ptr++ = yy;
        *ptr++ = pos.z + zz;
    }

    glColorPointer(4, GL_FLOAT, 7 * sizeof(float), buf + 0);
    glVertexPointer(3, GL_FLOAT, 7 * sizeof(float), buf + 4);
    glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_DIVISIONS + 2);

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
}

unsigned char* loadImg(const string& fname, int& w, int& h, int& t)
{
    File::Reader f(fname);
    if (f.is_open() == false)
    {
        Exception("File " + fname + " not found");
    }
    size_t bufSize = f.size() - 2 - 2 - 4;
    unsigned short width = *((unsigned short*)f.pointer());
    unsigned short height = *((unsigned short*)(f.pointer() + 2));
    unsigned int type = *((unsigned short*)(f.pointer() + 4));
    unsigned char* buf = f.pointer() + 8;

    w = width;
    h = height;
    t = type;
    
    unsigned char* res = new unsigned char[width*height*type];
    uLongf resSize = width*height*type;
    uncompress(res, &resSize, buf, bufSize);

    return res;
}
