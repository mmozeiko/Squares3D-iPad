#include "fence.h"
#include "common.h"
#include "level.h"
#include "collision.h"
#include "vmath.h"
#include "body.h"
#include "network.h"

static const float fenceWidth = 0.3f;
static const float fenceHeight = 1.0f;
static const float fenceSpacing = fenceWidth + fenceWidth / 3.0f;

void makeFence(Level* level, const NewtonWorld* newtonWorld)
{
    return;
    CollisionSet fencePartsCollisions;
    fencePartsCollisions.insert(level->getCollision("fence"));
    //fencePartsCollisions.insert(level->getCollision("fenceClip1"));
    //fencePartsCollisions.insert(level->getCollision("fenceClip2"));
    fencePartsCollisions.insert(level->getCollision("fenceTop"));

    Collision* heightMap = level->getCollision("level");

    for (size_t fencesVectorIdx = 0; fencesVectorIdx < level->m_fences.size(); fencesVectorIdx++)
    {
        const vector<Vector>& fence = level->m_fences[fencesVectorIdx];
        for (size_t i = 0; i < fence.size() - 1; i++)
        {    
            const Vector startPoint = fence[i];
            const Vector endPoint = fence[i + 1];
            Vector delta(endPoint - startPoint);
            const Vector rotation(0, - delta.getRotationY(), 0);
            
            float howMany = delta.magnitude() / fenceSpacing;
            delta /= howMany;
            for (int j = 0; j < howMany; j++)
            {
                const string bodyID = "fence" + cast<string>(fencesVectorIdx) + "_" + cast<string>(i) + "_" + cast<string>(j);

                Body* body = new Body(bodyID, level, fencePartsCollisions);
                body->m_soundable = true;

                Vector position = Vector(startPoint + delta * static_cast<float>(j));
                position.y = fenceHeight / 2 + heightMap->getHeight(position.x, position.z) + 0.05f;
                body->setTransform(position, rotation);

                NewtonBodySetAutoSleep(body->m_newtonBody, 1);
                NewtonBodySetFreezeState(body->m_newtonBody, 1);
                //NewtonBodySetMassMatrix(body->m_newtonBody, 0, 0, 0, 0);

                level->m_bodies[bodyID] = body;
            }
        }
    }
}
