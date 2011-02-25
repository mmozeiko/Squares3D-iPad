#ifndef __GRASS_H__
#define __GRASS_H__

#include "common.h"
#include "vmath.h"
#include "video.h"

class Level;
class Texture;

class Grass : public NoCopy
{
public:
    Grass(const Level* level);
    ~Grass();

    void render() const;

private:
    float        m_time;

    size_t       m_count;
    unsigned int m_buffer;
    FaceVector   m_faces;
    Texture*     m_grassTex;  
};

#endif
