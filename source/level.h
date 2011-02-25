#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <Newton.h>
#include "common.h"
#include "vmath.h"

class XMLnode;
class Material;    
class Properties;
class Collision;
class Body;
class Profile;
class Music;

struct Face;

struct Triangle
{
    const Face* f0;
    const Face* f1;
    const Face* f2;
};

typedef map<string, Material*>  MaterialsMap;
typedef map<string, Collision*> CollisionsMap;
typedef map<string, Body*>      BodiesMap;
typedef vector<Triangle>        TriangleVector;
typedef vector<vector<Vector> > FencesVector;
typedef vector<Music*>          MusicVector;

class Level : public NoCopy
{
public:
    Level();
    ~Level();
    void  load(const string& levelFile);
    void  load(const string& levelFile, StringSet& loaded);
    void  render() const;
    void  prepare();
    Body* getBody(const string& id) const;
    Collision* getCollision(const string& id) const;

    Vector          m_gravity;
    BodiesMap       m_bodies;
    CollisionsMap   m_collisions;
    TriangleVector  m_triangles;
    MaterialsMap    m_materials;
    Properties*     m_properties;
    FencesVector    m_fences;
    MusicVector     m_music;
    string          m_skyboxName;
};


#endif
