#include <Newton.h>
#include "openal_includes.h"

#include "properties.h"
#include "property.h"
#include "world.h"
#include "level.h"
#include "body.h"
#include "collision.h"
#include "xml.h"
#include "sound.h"
#include "sound_buffer.h"
#include "random.h"
#include "audio.h"
#include "network.h"

struct MaterialContact : NoCopy
{
    static int onBegin(const NewtonMaterial* material, const NewtonBody* body0, const NewtonBody* body1, int threadIndex);
    static void onProcess(const NewtonJoint* contact, float timestep, int threadIndex);
    
    void onEnd(Body* body0, Body* body1);

    Properties* properties;

    int m0;
    int m1;
    
    Vector position;
    float  maxSpeed;
};

Properties::Properties() : m_uniqueID(2), m_soundBufID(0)
{
    NewtonWorld* world = World::instance->m_newtonWorld;
    int defaultID = NewtonMaterialGetDefaultGroupID(world);
    NewtonMaterialSetDefaultCollidable(world, defaultID, defaultID, 1);
  
    m_materialContact = new MaterialContact();
    m_materialContact->properties = this;
    
    NewtonMaterialSetCollisionCallback(
        world, defaultID, defaultID,
        static_cast<void*>(m_materialContact), 
        MaterialContact::onBegin, 
        MaterialContact::onProcess);

    for (int i=0; i<8; i++)
    {
        m_idle.push_back(Soundable(new Sound(false), i<4+1)); // 5 important sources!
    }
}

Properties::~Properties()
{
    delete m_materialContact;

    for each_const(SoundableList, m_idle, iter)
    {
        delete iter->sound;
    }
    for each_const(SoundableList, m_active, iter)
    {
        delete iter->sound;
    }
    for each_(SoundBufMap, m_soundBufs, iter)
    {
        for each_(SoundBufferVector, iter->second, iter2)
        {
            Audio::instance->unloadSound(iter2->second);
        }
    }
}

void Properties::update()
{
    SoundableList::iterator iter = m_active.begin();
    while (iter != m_active.end())
    {
        if (iter->sound->is_playing() == false)
        {
            m_idle.push_back(*iter);
            iter = m_active.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

void Properties::play(Body* body, const pair<byte, SoundBuffer*>* buffer, bool important, const Vector& position)
{
    if (buffer == NULL)
    {
        return;
    }

    // check if not already playing
    SoundableList::iterator iterA = m_active.begin();
    while (iterA != m_active.end())
    {
        if (iterA->body == body)
        {
            return;
        }
        iterA++;
    }

    // check for free space
    SoundableList::iterator iterI = m_idle.begin();
    while (iterI != m_idle.end() && iterI->important != important)
    {
        iterI++;
    }
    if (iterI == m_idle.end())
    {
        return;
    }

    iterI->sound->play(buffer->second);
    iterI->sound->update(position, body->getVelocity());

    m_active.push_back(*iterI);
    m_idle.erase(iterI);
}

void Properties::play(byte id, const Vector& position)
{
    SoundBuffer* buffer = NULL;
    for each_const(SoundBufMap, m_soundBufs, iter)
    {
        for each_const(SoundBufferVector, iter->second, iter2)
        {
            if (iter2->first == id)
            {
                buffer = iter2->second;
                break;
            }
        }
        if (buffer != NULL)
        {
            break;
        }
    }

    if (buffer == NULL)
    {
        return;
    }

    // check for free space
    if (m_idle.size() == 0)
    {
        return;
    }
    SoundableList::iterator iterI = m_idle.begin();

    iterI->sound->play(buffer);
    iterI->sound->update(position, Vector::Zero);

    m_active.push_back(*iterI);
    m_idle.erase(iterI);
}

bool Properties::isPlaying(Body* body) const
{
    for each_const(SoundableList, m_active, iter)
    {
        if (iter->body == body)
        {
            return true;
        }
    }
    return false;
}

int  Properties::getUndefined() const
{
    return 0;
}

int  Properties::getInvisible() const
{
    return 1;
}

int  Properties::getDefault() const
{
    return 2;
}
    
int Properties::getPropertyID(const string& name)
{
    if (name.empty())
    {
        return getDefault();
    }

    IntMap::const_iterator iter = m_propertiesID.find(name);
    if (iter == m_propertiesID.end())
    {
        int result = ++m_uniqueID;
        //clog << "Property " << name << " id = " << result << endl;
        m_propertiesID.insert(make_pair(name, result));

        return result;
    }
    else
    {
        return iter->second;
    }
}

int  Properties::getPropertyID(const string& name) const
{
    if (name.empty())
    {
        return getDefault();
    }

    IntMap::const_iterator iter = m_propertiesID.find(name);
    if (iter == m_propertiesID.end())
    {
        assert(false);
    }
    return iter->second;
}

bool Properties::hasPropertyID(int id) const
{
    return id >= 2;
}

void Properties::load(const XMLnode& node)
{
    string prop0 = node.getAttribute("property0");
    string prop1 = node.getAttribute("property1");
    
    const int id0 = getPropertyID(prop0);
    const int id1 = getPropertyID(prop1);
    
    if (foundIn(m_properties, makepID(id0, id1)))
    {
        Exception("Properties for '" + prop0 + "' and '"
                                           + prop1 + "' already loaded");
    }

    const Property& def = m_properties.find(makepID(getDefault(), getDefault()))->second;

    float sF = node.getAttribute("staticFriction", def.staticFriction);
    float kF = node.getAttribute("kineticFriction", def.kineticFriction);
    float eC = node.getAttribute("elasticityCoefficient", def.elasticityCoefficient);
    float sC = node.getAttribute("softnessCoefficient", def.softnessCoefficient);

    m_properties.insert(make_pair(makepID(id0, id1), Property(sF, kF, eC, sC)));

    SoundBufferVector& vec = m_soundBufs.insert(make_pair(makepID(id0, id1), SoundBufferVector())).first->second;
    for each_const(XMLnodes, node.childs, n)
    {
        if (n->name == "sound")
        {
            vec.push_back(make_pair(m_soundBufID++, Audio::instance->loadSound(n->getAttribute("name"))));
        }
        else
        {
            Exception("Invalid property node, expected sound child, but got - " + node.name);
        }       
    }
}

void Properties::loadDefault(const XMLnode& node)
{
    NewtonWorld* world = World::instance->m_newtonWorld;

    int defaultID = NewtonMaterialGetDefaultGroupID(world);

    float sF = node.getAttribute<float>("staticFriction");
    float kF = node.getAttribute<float>("kineticFriction");
    float eC = node.getAttribute<float>("elasticityCoefficient");
    float sC = node.getAttribute<float>("softnessCoefficient");

    m_properties.insert(make_pair(makepID(getDefault(), getDefault()), Property(sF, kF, eC, sC)));

    NewtonMaterialSetDefaultElasticity(world, defaultID, defaultID, eC);
    NewtonMaterialSetDefaultFriction(world, defaultID, defaultID, sF, kF);
    NewtonMaterialSetDefaultSoftness(world, defaultID, defaultID, sC);
}

pID Properties::makepID(int id0, int id1) const
{
    if (id0 > id1)
    {
        swap(id0, id1);
    }
    // now id0 always <= id1
    return (static_cast<pID>(id0) << 32) | (id1);
}

const Property* Properties::get(int id0, int id1) const
{
    PropertiesMap::const_iterator iter = m_properties.find(makepID(id0, id1));
    
    if (iter != m_properties.end())
    {
        return &iter->second;
    }
    else
    {
        return NULL;
    }
}
    
const pair<byte, SoundBuffer*>* Properties::getSB(int id0, int id1) const
{
    SoundBufMap::const_iterator iter = m_soundBufs.find(makepID(id0, id1));
    
    if (iter != m_soundBufs.end())
    {
        const SoundBufferVector& vec = iter->second;
        if (vec.size() > 0)
        {
            int r = Randoms::getIntN(static_cast<int>(vec.size()));
            return &vec[r];
        }
    }

    return NULL;
}

int MaterialContact::onBegin(const NewtonMaterial* material, const NewtonBody* body0, const NewtonBody* body1, int threadIndex)
{
    MaterialContact* self = static_cast<MaterialContact*>(NewtonMaterialGetMaterialPairUserData(material));
   
    if (body0 == body1)
    {
        assert(false);
        return 0;
    }

    self->maxSpeed = 0.0f;

    return 1;
}

void MaterialContact::onProcess(const NewtonJoint* contactJoint, dFloat timestep, int threadIndex)
{
    Contacts contactsToRemove;

    Body* body0 = static_cast<Body*>(NewtonBodyGetUserData(NewtonJointGetBody0(contactJoint)));
    Body* body1 = static_cast<Body*>(NewtonBodyGetUserData(NewtonJointGetBody1(contactJoint)));

    bool hasContact = false;

    MaterialContact* selff = NULL;

    for (void* contact = NewtonContactJointGetFirstContact(contactJoint);
         contact != NULL;
         contact = NewtonContactJointGetNextContact(contactJoint, contact))
    {
        NewtonMaterial* material = NewtonContactGetMaterial(contact);
        MaterialContact* self = static_cast<MaterialContact*>(NewtonMaterialGetMaterialPairUserData(material));
        
        int colID0 = NewtonMaterialGetBodyCollisionID(material, body0->m_newtonBody);
        int colID1 = NewtonMaterialGetBodyCollisionID(material, body1->m_newtonBody);
        int faceAttr = NewtonMaterialGetContactFaceAttribute(material);

        if (colID0 == self->properties->getInvisible())
        {
            body0->onCollideHull(body1);

            const Property * prop = self->properties->get((faceAttr ? faceAttr : colID1), self->properties->getPropertyID("football"));
            if (prop != NULL)
            {
                prop->apply(material);
            }
            contactsToRemove.push_back(contact);
            continue;
        }
        else if (colID1 == self->properties->getInvisible())
        {
            body1->onCollideHull(body0);
            const Property * prop = self->properties->get((faceAttr ? faceAttr : colID0), self->properties->getPropertyID("football"));
            if (prop != NULL)
            {
                prop->apply(material);
            }
            contactsToRemove.push_back(contact);
            continue;
        }
         
        bool isConvex0 = self->properties->hasPropertyID(colID0);
        bool isConvex1 = self->properties->hasPropertyID(colID1);

        int m0, m1;
        
        if (isConvex0 && isConvex1)
        {
            // both covex
            m0 = colID0;
            m1 = colID1;
        }
        else if (isConvex0)
        {
            m0 = colID0;
            m1 = faceAttr;
        }
        else if (isConvex1)
        {
            m0 = faceAttr;
            m1 = colID1;
        }
        else
        {
            // both not convex
            assert(false);
            // TODO: what to do?
            //return 1;
            m0 = self->properties->getDefault();
            m1 = self->properties->getDefault();
        }

        Vector normal;

        float sp = NewtonMaterialGetContactNormalSpeed(material);
        if (sp > self->maxSpeed)
        {
            self->maxSpeed = sp;
            NewtonMaterialGetContactPositionAndNormal(material, self->position.v, normal.v);
            self->m0 = m0;
            self->m1 = m1;
        }

        for (int i=0; i<2; i++)
        {
            float speed = NewtonMaterialGetContactTangentSpeed(material, i);
            if (speed > self->maxSpeed)
            {
                self->maxSpeed = speed;
                NewtonMaterialGetContactPositionAndNormal(material, self->position.v, normal.v);
                self->m0 = m0;
                self->m1 = m1;
            }
        }

        const Property* prop = self->properties->get(m0, m1);
        
        if (prop == NULL)
        {
            prop = self->properties->get(self->properties->getDefault(), 
                                         self->properties->getDefault());
        }
        int playerID = self->properties->getPropertyID("player");
        if (m0==playerID && m1==playerID)
        {
            // TODO: this is hack, to bounce players off each other
            NewtonMaterialSetContactNormalAcceleration(material, 50.0f);
        }

        prop->apply(material);
        
        hasContact = true;
        selff = self;
    }
    
    if (hasContact)
    {
        selff->onEnd(body0, body1);
    }

    for each_const(Contacts, contactsToRemove, c)
    {
        NewtonContactJointRemoveContact(contactJoint, *c);
    }
}

void MaterialContact::onEnd(Body* body0, Body* body1)
{
    body0->onCollide(body1, position, maxSpeed);
    body1->onCollide(body0, position, maxSpeed);
    
    if (maxSpeed > 0.5f && !properties->isPlaying(body0) && !properties->isPlaying(body1) )
    {
        if (body0->m_soundable)
        {
            properties->play(body0, properties->getSB(m0, m1), body0->m_important, position);
        }
        else if (body1->m_soundable)
        {
            properties->play(body1, properties->getSB(m0, m1), body1->m_important, position);
        }
    }
}
