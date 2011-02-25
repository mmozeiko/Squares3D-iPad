#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "common.h"
#include "video.h"

class Texture;

class SkyBox : public NoCopy
{
public:
    SkyBox(const string& name);

    void render(const Matrix& modelviewMatrix) const;

private:
    Texture*      m_texture;
    vector<float> m_data;
};

#endif
