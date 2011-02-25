#include "messages.h"
#include "video.h"
#include "message.h"
#include "video.h"

Messages::Messages()
{
    m_fonts[32] = Font::get("Arial_48pt_bold");
    m_fonts[72] = Font::get("Arial_72pt_bold");
}

void Messages::update(float delta)
{
    for each_(MessageVectorsByHeight, m_buffer, iter)
    {
        MessageVector* currentHeightbuffer = &iter->second;
        MessageVector::iterator iter2 = currentHeightbuffer->begin();

        while (iter2 != currentHeightbuffer->end())
        {
            Message* message = *iter2;

            message->applyFlow(delta);

            if (message->applyDelta(delta))
            {
                delete message;
                iter2 = currentHeightbuffer->erase(iter2);
            }
            else
            {
                iter2++;
            }
        }
    }
}

void Messages::remove(Message* message)
{
    MessageVector* currentHeightbuffer = &m_buffer.find(message->m_fontSize)->second;
    MessageVector::iterator iter = currentHeightbuffer->begin();

    while (iter != currentHeightbuffer->end())
    {
        if (*iter == message)
        {
            delete *iter;
            iter = currentHeightbuffer->erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

static Vector mul(const Matrix& mx, const Vector& vec)
{
    return Vector(mx.m00*vec.x + mx.m10*vec.y + mx.m20*vec.z + mx.m30*vec.w,
                  mx.m01*vec.x + mx.m11*vec.y + mx.m21*vec.z + mx.m31*vec.w,
                  mx.m02*vec.x + mx.m12*vec.y + mx.m22*vec.z + mx.m32*vec.w,
                  mx.m03*vec.x + mx.m13*vec.y + mx.m23*vec.z + mx.m33*vec.w);
}

void Messages::add3D(Message* message)
{
    const Font* font = m_fonts.find(message->m_fontSize)->second;

    const Matrix& modelview = Video::instance->getModelViewMatrix();
    const Matrix& projection = Video::instance->getProjectionMatrix();
    int viewport_width;
    int viewport_height;
    Video::instance->getViewport(viewport_width, viewport_height);

    Vector v = mul(projection, mul(modelview, message->m_position));
    v /= v.w;
    float vx = viewport_width * (v.x + 1) * 0.5f;
    float vy = viewport_height * (v.y + 1) * 0.5f;
    float vz = (v.z + 1) * 0.5f;

    //correct positions to fit on screen
    if (vx < font->getWidth(message->getText()) / 2)
    {
        vx = (float)font->getWidth(message->getText()) / 2;
    }
    else if (vx > Video::instance->getResolution().first - font->getWidth(message->getText()) / 2)
    {
        vx = (float)Video::instance->getResolution().first - font->getWidth(message->getText()) / 2;
    }

    if (vy < 0)
    {
        vy = 0;
    }
    else if (vy > Video::instance->getResolution().second - font->getHeight(message->getText()) * 5)
    {
        vy = (float)Video::instance->getResolution().second - font->getHeight(message->getText()) * 5;
    }

    message->m_position = Vector(static_cast<float>(vx),
                                 static_cast<float>(vy),
                                 static_cast<float>(vz));

    m_buffer[message->getFontSize()].push_back(message);
}

void Messages::add2D(Message* message)
{
    m_buffer[message->getFontSize()].push_back(message);
}

void Messages::render() const
{
    for each_const(MessageVectorsByHeight, m_buffer, iter)
    {
        const Font* font = m_fonts.find(iter->first)->second;
        font->begin();
        for each_const(MessageVector, iter->second, iter2)
        {
            Message* message = *iter2;
            message->render(font);
        }
        font->end();
    }
}

Messages::~Messages()
{
    for each_const(MessageVectorsByHeight, m_buffer, iter)
    {
        MessageVector currentHeightbuffer = iter->second;
        for each_const(MessageVector, currentHeightbuffer, iter2)
        {
            delete *iter2;    
        }
    }
}
