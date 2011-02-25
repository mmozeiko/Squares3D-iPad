#include "collision.h"
#include "xml.h"
#include "material.h"
#include "level.h"
#include "property.h"
#include "properties.h"
#include "world.h"
#include "texture.h"
#include "config.h"
#include "geometry.h"
#include "input.h"
#include "mesh.h"
#include "video.h"

class CollisionConvex : public Collision
{
protected:
    CollisionConvex(const XMLnode& node, const Level* level);
    ~CollisionConvex();
    void render() const;

    Material* m_material;
    Mesh* m_mesh;

    bool      m_hasOffset; // false
    Matrix    m_matrix;
    int       m_propertyID;
};

class CollisionBox : public CollisionConvex
{
public:
    CollisionBox(const XMLnode& node, const Level* level);

    Vector m_size;      // (1.0f, 1.0f, 1.0f)
};

class CollisionSphere : public CollisionConvex
{
public:
    CollisionSphere(const XMLnode& node, const Level* level);

    float getRadius() const { return m_radius.x; }

    Vector m_radius;      // (1.0f, 1.0f, 1.0f)
};

class CollisionCylinder : public CollisionConvex
{
public:
    CollisionCylinder(const XMLnode& node, const Level* level);

    float m_radius;      // 1.0f
    float m_height;      // 1.0f
};

class CollisionCone : public CollisionConvex
{
public:
    CollisionCone(const XMLnode& node, const Level* level);

    float m_radius;      // 1.0f
    float m_height;      // 1.0f
};

class CollisionTree : public Collision
{
public:
    CollisionTree(const XMLnode& node, Level* level);
    ~CollisionTree();

    void render() const;

    vector<Material*> m_materials;
    FaceVector        m_faces;
};

class CollisionHMap : public Collision
{
public:
    CollisionHMap(const XMLnode& node, Level* level);
    ~CollisionHMap();
    
    void render() const;
    float getHeight(float x, float y) const;

private:
    int                    m_width;
    int                    m_height;
    float                  m_size;

    FaceVector             m_faces;
    vector<unsigned short> m_indices;
    
    int                    m_realCount;

    Material*    m_material;
    unsigned int m_buffers[2];
};

Collision::Collision(const XMLnode& node) :
    m_newtonCollision(NULL),
    m_origin(),
    m_inertia(),
    m_mass(0.0f)
{
}

Collision::~Collision()
{
    //NewtonReleaseCollision(World::instance->m_newtonWorld, m_newtonCollision);
}

float Collision::getHeight(float x, float z) const
{
    return 0;
}

void Collision::create(NewtonCollision* collision)
{
    m_newtonCollision = collision;
}

void Collision::create(NewtonCollision* collision, int propertyID, float mass)
{
    m_newtonCollision = collision;
    m_mass = mass;

    NewtonConvexCollisionCalculateInertialMatrix(m_newtonCollision, m_inertia.v, m_origin.v);
    NewtonCollisionSetUserID(m_newtonCollision, propertyID);

    m_inertia *= mass;
    m_origin *= mass;
}

Collision* Collision::create(const XMLnode& node, Level* level)
{
    string type = node.getAttribute("type");
    
    if (type == "box")
    {
        return new CollisionBox(node, level);
    }    
    else if (type == "sphere")
    {
        return new CollisionSphere(node, level);
    }
    else if (type == "cylinder")
    {
        return new CollisionCylinder(node, level);
    }
    else if (type == "cone")
    {
        return new CollisionCone(node, level);
    }
    else if (type == "tree")
    {
        return new CollisionTree(node, level);
    }
    else if (type == "heightmap")
    {
        return new CollisionHMap(node, level);
    }
    else
    {
        Exception("Unknown collision type - " + type);
    }
}

CollisionConvex::CollisionConvex(const XMLnode& node, const Level* level) :
    Collision(node),
    m_material(NULL),
    m_hasOffset(false),
    m_matrix(),
    m_propertyID(0),
    m_mesh(NULL)
{
    Vector offset(0.0f, 0.0f, 0.0f);
    Vector rotation(0.0f, 0.0f, 0.0f);

    if (node.hasAttribute("property"))
    {
        m_propertyID = level->m_properties->getPropertyID(node.getAttribute("property"));
    }
    else
    {
        m_propertyID = level->m_properties->getDefault();
    }

    if (node.hasAttribute("material"))
    {
        string material = node.getAttribute("material");
        MaterialsMap::const_iterator iter = level->m_materials.find(material);
        if (iter == level->m_materials.end())
        {
            Exception("Couldn't find material '" + material + "'");
        }
        m_material = iter->second;;
    }
    
    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "offset")
        {
            offset = node.getAttributesInVector("xyz");
            m_hasOffset = true;
        }
        else if (node.name == "rotation")
        {
            rotation = node.getAttributesInVector("xyz") * DEG_IN_RAD;
            m_hasOffset = true;
        }
    }

    if (m_hasOffset)
    {
        NewtonSetEulerAngle(rotation.v, m_matrix.m);
        m_matrix = Matrix::translate(offset) * m_matrix;
    }
}

CollisionConvex::~CollisionConvex()
{
    delete m_mesh;
}


void CollisionConvex::render() const
{
    glPushMatrix();

    Video::instance->bind(m_material);

    if (m_hasOffset)
    {
        glMultMatrixf(m_matrix.m);
    }
    m_mesh->render();

    glPopMatrix();
}

CollisionBox::CollisionBox(const XMLnode& node, const Level* level) :
    CollisionConvex(node, level),
    m_size(1.0f, 1.0f, 1.0f)
{
    float mass = node.getAttribute<float>("mass");

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "size")
        {
            m_size = node.getAttributesInVector("xyz");
        }
        else if (node.name != "offset" && node.name != "rotation")
        {
            Exception("Invalid collision, unknown node - " + node.name);
        }
    }

    create(
        NewtonCreateBox(
            World::instance->m_newtonWorld, 
            m_size.x, m_size.y, m_size.z,
            0,
            (m_hasOffset ? m_matrix.m : NULL)
        ),
        m_propertyID,
        mass);

    m_mesh = new CubeMesh(m_size);
}

CollisionSphere::CollisionSphere(const XMLnode& node, const Level* level) :
    CollisionConvex(node, level),
    m_radius(1.0f, 1.0f, 1.0f)
{
    float mass = node.getAttribute<float>("mass");

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "radius")
        {
            m_radius = node.getAttributesInVector("xyz");
        }
        else if (node.name != "offset" && node.name != "rotation")
        {
            Exception("Invalid collision, unknown node - " + node.name);
        }
    }

    create(
        NewtonCreateSphere(
            World::instance->m_newtonWorld, 
            m_radius.x, m_radius.y, m_radius.z,
            0,
            (m_hasOffset ? m_matrix.m : NULL)
        ),
        m_propertyID,
        mass);

    m_mesh = new SphereMesh(m_radius, 12, 12);
}

CollisionCylinder::CollisionCylinder(const XMLnode& node, const Level* level) :
    CollisionConvex(node, level),
    m_radius(1.0f),
    m_height(1.0f)
{
    float mass = node.getAttribute<float>("mass");

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "radius")
        {
            m_radius = cast<float>(node.value);
        }
        else if (node.name == "height")
        {
            m_height = cast<float>(node.value);
        }
        else if (node.name != "offset" && node.name != "rotation")
        {
            Exception("Invalid collision, unknown node - " + node.name);
        }
    }

    create(
        NewtonCreateCylinder(
            World::instance->m_newtonWorld, 
            m_radius,
            m_height,
            0,
            (m_hasOffset ? m_matrix.m : NULL)
        ),
        m_propertyID,
        mass);

    m_hasOffset = true;
    m_matrix *= Matrix::translate(Vector(-m_height/2.0f, 0.0f, 0.0f));

    m_mesh = new CylinderMesh(m_radius, m_height, 2, 12);
}

CollisionCone::CollisionCone(const XMLnode& node, const Level* level) :
    CollisionConvex(node, level),
    m_radius(1.0f),
    m_height(1.0f)
{
    float mass = node.getAttribute<float>("mass");

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "radius")
        {
            m_radius = cast<float>(node.value);
        }
        else if (node.name == "height")
        {
            m_height = cast<float>(node.value);
        }
        else if (node.name != "offset" && node.name != "rotation")
        {
            Exception("Invalid collision, unknown node - " + node.name);
        }
    }

    create(
        NewtonCreateCone(
            World::instance->m_newtonWorld, 
            m_radius,
            m_height,
            0,
            (m_hasOffset ? m_matrix.m : NULL)
        ),
        m_propertyID,
        mass);

    m_hasOffset = true;
    m_matrix *= Matrix::translate(Vector(-m_height/2.0f, 0.0f, 0.0f));

    m_mesh = new ConeMesh(m_radius, m_height, 2, 12);
}

CollisionTree::CollisionTree(const XMLnode& node, Level* level) : Collision(node)
{
    vector<int> props;

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "face")
        {
            string material = node.getAttribute("material");
            if (material.empty())
            {
                m_materials.push_back(NULL);
            }
            else
            {
                MaterialsMap::const_iterator iter = level->m_materials.find(material);
                if (iter == level->m_materials.end())
                {
                    Exception("Couldn't find material '" + material + "'");
                }
                m_materials.push_back(iter->second);
            }
            
            string p = node.getAttribute("property");
            props.push_back(level->m_properties->getPropertyID(p));

            size_t back = m_faces.size();
            // needed in grass calculations
            for each_const(XMLnodes, node.childs, iter)
            {
                const XMLnode& node = *iter;
                if (node.name == "vertex")
                {
                    m_faces.push_back(Face(UV(node.getAttribute<float>("u"), node.getAttribute<float>("v")),
                                           Vector(),
                                           node.getAttributesInVector("xyz"))
                                     );
                }
                else
                { 
                    Exception("Invalid face, unknown node - " + node.name);
                }
            }
            if (m_faces.size() - back != 4)
            {
                Exception("Face must have 4 vertexes");
            }

            const Vector& v0 = Vector(m_faces[back].v);
            const Vector& v1 = Vector(m_faces[back+1].v);
            const Vector& v2 = Vector(m_faces[back+2].v);
            
            Vector normal = (v1-v0) ^ (v2-v0);
            normal.norm();

            m_faces[back].n[0] = m_faces[back+1].n[0] = m_faces[back+2].n[0] = m_faces[back+3].n[0] = normal.x;
            m_faces[back].n[1] = m_faces[back+1].n[1] = m_faces[back+2].n[1] = m_faces[back+3].n[1] = normal.y;
            m_faces[back].n[2] = m_faces[back+1].n[2] = m_faces[back+2].n[2] = m_faces[back+3].n[2] = normal.z;

        }
        else
        {
            Exception("Invalid collision, unknown node - " + node.name);
        }
    }
    
    NewtonCollision* collision = NewtonCreateTreeCollision(World::instance->m_newtonWorld, NULL);
    NewtonTreeCollisionBeginBuild(collision);
    for (size_t i=0; i<m_faces.size(); i+=4)
    {
        int prop = props[i/4];
        Face& face = m_faces[i];
        NewtonTreeCollisionAddFace(collision, 4, face.v, sizeof(Face), prop);
        std::swap(m_faces[i+2], m_faces[i+3]);
    }
    NewtonTreeCollisionEndBuild(collision, 0);
    
    create(collision);
}

void CollisionTree::render() const
{
    if (m_faces.size() == 0)
    {
        return;
    }

    Material* lastMaterial = NULL;
    for (size_t i = 0; i < m_faces.size(); i += 4)
    {
        Material* material = m_materials[i/4];
        if (material != lastMaterial)
        {
            Video::instance->bind(material);
            lastMaterial = material;
        }

        Video::instance->renderFace(m_faces, static_cast<int>(i), 4);
    }
}

CollisionTree::~CollisionTree()
{
}

CollisionHMap::CollisionHMap(const XMLnode& node, Level* level) : Collision(node), m_material(NULL)
{
    string hmap;
    float size = 0.0f;
    string material;
    float repeat = 0.0f;

    for each_const(XMLnodes, node.childs, iter)
    {
        const XMLnode& node = *iter;
        if (node.name == "heightmap")
        {
            hmap = node.getAttribute("name");
            size = node.getAttribute<float>("size");
            material = node.getAttribute("material");
            repeat = node.getAttribute<float>("repeat");
        }
        else
        {
            Exception("Invalid collision, unknown node - " + node.name);
        }
    }
    
    if (hmap.empty())
    {
        Exception("Invalid heightmap collision, heightmap name not specified");
    }
    if (material.empty())
    {
        Exception("Invalid heightmap collision, material name not specified");
    }
    if (repeat == 0.0f)
    {
        Exception("Invalid heightmap collision, repeat not specified");
    }

    if (!foundIn(level->m_materials, material))
    {
        Exception("Material '" + material + "' not found!");
    }
    m_material = level->m_materials.find(material)->second;

    const float c = repeat/size; //1.5f; //m_texture->m_size;//1.5f;

    int id = level->m_properties->getPropertyID("grass");

    NewtonCollision* collision = NewtonCreateTreeCollision(World::instance->m_newtonWorld, NULL);
    NewtonTreeCollisionBeginBuild(collision);
     
#if 0    
    if (File::exists("/data/heightmaps/" + hmap + ".hmap"))
    {
        string filename = "/data/heightmaps/" + hmap + ".hmap";
        File::Reader file(filename);
        if (!file.is_open())
        {
            Exception("Heightmap '" + filename + "' not found");
        }
        char head[4];
        file.read(head, 4);
        if (string(head, head+4) != "HMAP")
        {
            Exception("Invalid heightmap '" + filename + "' format");
        }
        int vlen, ilen;
        file.read(&vlen, sizeof(vlen));
        file.read(&ilen, sizeof(ilen));

        m_vertices.resize(vlen);
        m_normals.resize(vlen);
        m_uv.resize(vlen);

        m_indices.resize(3*ilen);

        file.read(&m_vertices[0], sizeof(Vector)*vlen);
        file.read(&m_normals[0], sizeof(Vector)*vlen);
        file.read(&m_uv[0], sizeof(UV)*vlen);
        file.read(&m_indices[0], sizeof(unsigned short)*3*ilen);
        
        file.close();

        m_tmpFaces.resize(3*ilen);
        for (int i=0; i<ilen*3; i+=3)
        {
            int i0 = m_indices[i+0];
            int i1 = m_indices[i+1];
            int i2 = m_indices[i+2];

            const Vector arr[] = { m_vertices[i0], m_vertices[i1], m_vertices[i2] };
            NewtonTreeCollisionAddFace(collision, 3, arr[0].v, sizeof(Vector), id);

            Face* face = & m_tmpFaces[i];
            face->vertexes.resize(3);
            face->vertexes[0] = m_vertices[i0];
            face->vertexes[1] = m_vertices[i1];
            face->vertexes[2] = m_vertices[i2];
            level->m_faces.insert(make_pair(face, level->m_materials["grass"]));
        }
    }
    else
#endif
    {
        int width;
        int height;
        int comp;
        unsigned char* image = loadImg("/data/heightmaps/" + hmap + ".img", width, height, comp);

        if (comp != 1)
        {
            Exception("Invalid heightmap '" + hmap + "', image must be grayscale");
        }

        m_size = size;
        m_width = width;
        m_height = height;
        
        float size2 = size/2.0f;

        // 0.5f, 1.0f, 1.5f
        const float STEP = 0.25f;

        int maxIdx = 0;
        bool maxIdxB = false;

        float z = -size2;
        while (true)
        {
            bool badMargin = false; // for normal

            float z2 = z + STEP;
            int iz = static_cast<int>(std::floor((z + size2) * height / size));
            int iz2 = static_cast<int>(std::floor((z2 + size2) * height / size));
            if (iz >= height)
            {
                z = size2;
                iz = height-1;
            }
            if (iz2 >= height)
            {
                z2 = size2;
                iz2 = height-1;
                badMargin = true;
            }

            float x = -size2;
            while (true)
            {
                float x2 = x + STEP;
                int ix = static_cast<int>(std::floor((x + size2) * width / size));
                int ix2 = static_cast<int>(std::floor((x2 + size2) * width / size));
                if (ix >= width)
                {
                    x = size2;
                    ix = width-1;
                }
                if (ix2 >= width)
                {
                    x2 = size2;
                    ix2 = width-1;
                    badMargin = true;
                }

                float y1 = (image[width * (height-1-iz) + ix] - 128) / 20.0f;
                float y2 = (image[width * (height-1-iz2) + ix] - 128) / 20.0f;
                float y4 = (image[width * (height-1-iz) + ix2] - 128) / 20.0f;

                const Vector v0 = Vector(x, y1, z);
                const Vector v1 = Vector(x, y2, z2);
                const Vector v3 = Vector(x2, y4, z);
                
                Vector normal;
                if (badMargin)
                {
                    normal = Vector(m_faces.back().n);
                }
                else
                {
                    normal = (v1 - v0)  ^ (v3 - v0);
                    normal.norm();
                }

                m_faces.push_back(Face(UV((x+size2)*c, (z+size2)*c), normal, v0));

                if (x == size2)
                {
                    break;
                }
                x += STEP;
            }
            if (!maxIdxB)
            {
                maxIdx = static_cast<int>(m_faces.size());
                maxIdxB = true;
            }

            if (z == size2)
            {
                break;
            }
            z += STEP;
        }
      
        delete [] image;

        m_realCount = maxIdx;

        for (int z=0; z<maxIdx-1; z++)
        {
            for (int x=0; x<maxIdx-1; x++)
            {
                int i1 = z*maxIdx+x;
                int i2 = (z+1)*maxIdx+x;
                int i3 = (z+1)*maxIdx+x+1;
                int i4 = z*maxIdx+x+1;

                const Vector v1 = Vector(m_faces[i1].v);
                const Vector v2 = Vector(m_faces[i2].v);
                const Vector v3 = Vector(m_faces[i3].v);
                const Vector v4 = Vector(m_faces[i4].v);
                
                if (i3 > 65535)
                {
                    Exception("Too many vertices in heightmap!!");
                }

                m_indices.push_back(i1);
                m_indices.push_back(i2);
                m_indices.push_back(i4);

                m_indices.push_back(i2);
                m_indices.push_back(i3);
                m_indices.push_back(i4);

                {
                    const Vector arr[] = { v1, v2, v4 };
                    NewtonTreeCollisionAddFace(collision, 3, arr[0].v, sizeof(Vector), id);
                }
                {
                    const Vector arr[] = { v2, v3, v4 };
                    NewtonTreeCollisionAddFace(collision, 3, arr[0].v, sizeof(Vector), id);
                }

                if (m_material->m_id == "grass")
                {
                    Triangle tri;
                    tri.f0 = &m_faces[i1];
                    tri.f1 = &m_faces[i2];
                    tri.f2 = &m_faces[i3];
                    level->m_triangles.push_back(tri);

                    tri.f0 = &m_faces[i2];
                    tri.f1 = &m_faces[i3];
                    tri.f2 = &m_faces[i4];
                    level->m_triangles.push_back(tri);
                }
            }
        }
    }
    
    NewtonTreeCollisionEndBuild(collision, 0);
    
    create(collision);  
}

void CollisionHMap::render() const
{
    if (m_indices.size() == 0)
    {
        return;
    }

    m_material->bind();

    glTexCoordPointer(2, GL_FLOAT, sizeof(Face), &m_faces[0].tc[0]);
    glNormalPointer(GL_FLOAT,      sizeof(Face), &m_faces[0].n[0]);
    glVertexPointer(3, GL_FLOAT,   sizeof(Face), &m_faces[0].v[0]);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_SHORT, &m_indices[0]);
}

CollisionHMap::~CollisionHMap()
{
}

float CollisionHMap::getHeight(float x, float z) const
{
    float x0 = (x + m_size/2.0f) * (m_realCount-1) / m_size;
    float z0 = (z + m_size/2.0f) * (m_realCount-1) / m_size;
    
    x0 = std::min(std::max(x0, 0.0f), static_cast<float>(m_realCount-2));
    z0 = std::min(std::max(z0, 0.0f), static_cast<float>(m_realCount-2));
    
    int ix = static_cast<int>(std::floor(x0));
    int iz = static_cast<int>(std::floor(z0));

    x0 -= static_cast<float>(ix);
    z0 -= static_cast<float>(iz);
  
    Vector v1;
    Vector v2;
    Vector v3;

    if (x0+z0 <= 1.0f)
    {
        // lower triangle
        v1 = Vector(m_faces[m_realCount * iz + ix].v);
        v2 = Vector(m_faces[m_realCount * (iz+1) + ix].v);
        v3 = Vector(m_faces[m_realCount * iz + ix+1].v);
    }
    else
    {
        // upper triangle
        v1 = Vector(m_faces[m_realCount * (iz+1) + (ix+1)].v);
        v2 = Vector(m_faces[m_realCount * (iz+1) + ix].v);
        v3 = Vector(m_faces[m_realCount * iz + ix+1].v);
    }
   
    Vector s1 = v2 - v1;
    Vector s2 = v3 - v1;
    s1.norm();
    s2.norm();
    Vector n = s1 ^ s2;
    n.norm();
    const float D = - (n % v1);

    return -(n.x * x + n.z * z + D)/n.y;
}
