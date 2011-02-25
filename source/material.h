#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "common.h"
#include "vmath.h"

class Level;
class Texture;
class XMLnode;

class Material : public NoCopy
{
    friend class Level;
public:
    string  m_id;        // ""
    Vector  m_cAmbient;  // (0.2f, 0.2f, 0.2f, 1.0f)
    Vector  m_cDiffuse;  // (0.8f, 0.8f, 0.8f, 1.0f)
    Vector  m_cSpecular; // (0.0f, 0.0f, 0.0f)
    Vector  m_cEmission; // (0.0f, 0.0f, 0.0f, 1.0f)
    float   m_cShine;    // 0.0f

    void bind() const;
private: 
    Material(const XMLnode& node);

    Texture* m_texture;
};

#endif
