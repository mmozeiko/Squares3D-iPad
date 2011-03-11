#include "intro.h"
#include "input.h"
#include "texture.h"
#include "font.h"
#include "mesh.h"

static const float FADE_OUT_SECS = 10.0f;
static const float MENU_SHOW_SECS = 11.0f;

static const float PIECE_MASS = 0.5f;
static const float PIECE_SIZEX = 0.45f;
static const float PIECE_SIZEY = 0.45f;
static const float PIECE_SIZEZ = 0.08f;

static const int PIECE_XCOUNT = 7;
static const int PIECE_YCOUNT = 4;

static const float BALL_MASS = 5000.0f;
static const float BALL_R = 0.5f;

static void OnForce(const NewtonBody* body, float timestep, int threadIndex)
{
    Vector gravity( 0.0f, -9.81f, 0.0f );
    float m, Ix, Iy, Iz;
    NewtonBodyGetMassMatrix(body, &m, &Ix, &Iy, &Iz);
    gravity.y *= m;
    NewtonBodyAddForce(body, gravity.v);
}

Intro::Intro() : m_timePassed(0), m_nextState(false), m_ballKicked(false), m_mesh(NULL)
{
    m_newtonWorld = NewtonCreate();
	NewtonSetPlatformArchitecture(m_newtonWorld, 0);
	NewtonSetSolverModel(m_newtonWorld, 0);
	NewtonSetThreadsCount(m_newtonWorld, 1);
	NewtonSetFrictionModel(m_newtonWorld, 0);

    vector<UV> piece_uv;

    NewtonCollision* collision = NewtonCreateBox(m_newtonWorld, PIECE_SIZEX, PIECE_SIZEY, PIECE_SIZEZ, 0, NULL);
    for (int y = 0; y < PIECE_YCOUNT; y++)
    {
        float inertia[3];
        float origin[3];
        for (int x = 0; x < PIECE_XCOUNT; x++)
        {
            Matrix matrix = Matrix::translate(
                Vector(-PIECE_XCOUNT*PIECE_SIZEX/2 + x*PIECE_SIZEX + (y%2 ? PIECE_SIZEX/2.0f : 0), 
                       PIECE_SIZEY/2.0f + y * PIECE_SIZEY,
                       0.0f)
            );
            NewtonBody* body = NewtonCreateBody(m_newtonWorld, collision, matrix.m);

            NewtonConvexCollisionCalculateInertialMatrix(collision, inertia, origin);
            inertia[0] *= PIECE_MASS;
            inertia[1] *= PIECE_MASS;
            inertia[2] *= PIECE_MASS;
            NewtonBodySetMassMatrix(body, PIECE_MASS, inertia[0], inertia[1], inertia[2]);
            NewtonBodySetCentreOfMass(body, origin);
            NewtonBodySetForceAndTorqueCallback(body, &OnForce);

            NewtonBodySetAutoSleep(body, 1);
            NewtonBodySetFreezeState(body, 1);

            m_piece_body.push_back(body);
            m_piece_matrix.push_back(matrix);

            static const float maxv = 768.0f/1024.0f; // depends on indago logo texture structure

            static const float uv[][2] = {
                { 0.0f, 1.0f },
                { 1.0f, 1.0f },
                { 1.0f, 0.0f },
                { 0.0f, 0.0f },
            };

            for (int k=0; k<4; k++)
            {
                float tmpX = static_cast<float>(x);
                if ((y&1)==0)
                {
                    tmpX -= 0.5f;
                }
                float u = 1.0f-(tmpX+uv[k][0])/PIECE_XCOUNT;
                float v = (y+uv[k][1])*maxv/PIECE_YCOUNT;
                piece_uv.push_back(UV(u, v));
            }
        }
    }
    NewtonReleaseCollision(m_newtonWorld, collision);

    // ball
    Matrix matrix = Matrix::translate(Vector(0.0f, BALL_R, 1.5f));
    collision = NewtonCreateSphere(m_newtonWorld, BALL_R, BALL_R, BALL_R, 0, NULL);
    m_ball_body = NewtonCreateBody(m_newtonWorld, collision, matrix.m);

    float inertia[3];
    float origin[3];
    NewtonConvexCollisionCalculateInertialMatrix(collision, inertia, origin);
    inertia[0] *= BALL_MASS;
    inertia[1] *= BALL_MASS;
    inertia[2] *= BALL_MASS;
    NewtonBodySetMassMatrix(m_ball_body, BALL_MASS, inertia[0], inertia[1], inertia[2]);
    NewtonBodySetCentreOfMass(m_ball_body, origin);
    NewtonBodySetForceAndTorqueCallback(m_ball_body, &OnForce);

    NewtonBodySetAutoSleep(m_ball_body, 0);
    NewtonReleaseCollision(m_newtonWorld, collision);

    // floor
    NewtonCollision* floor = NewtonCreateBox(m_newtonWorld, 20.0f, 1.0f, 20.0f, 0, NULL);
    matrix = Matrix::translate(Vector(0.0f, -0.5f, 0.0f));
    NewtonCreateBody(m_newtonWorld, floor, matrix.m);
    NewtonReleaseCollision(m_newtonWorld, floor);

    m_ballTex = Video::instance->loadTexture("ball");
    m_logoTex = Video::instance->loadTexture("indago_logo", Texture::ClampToEdge);
    m_bigLogoTex = Video::instance->loadTexture("logo_big", Texture::ClampToEdge);

    // -0.5 .. 0.5
    static const float vertices[][3] = {
        /* 0 */ { -0.5f, -0.5f, -0.5f },
        /* 1 */ {  0.5f, -0.5f, -0.5f },
        /* 2 */ {  0.5f, -0.5f,  0.5f },
        /* 3 */ { -0.5f, -0.5f,  0.5f },

        /* 4 */ { -0.5f, 0.5f, -0.5f },
        /* 5 */ {  0.5f, 0.5f, -0.5f },
        /* 6 */ {  0.5f, 0.5f,  0.5f },
        /* 7 */ { -0.5f, 0.5f,  0.5f },
    };

    static const int faces[][4] = {
        { 0, 1, 2, 3 }, // bottom
        { 4, 7, 6, 5 }, // up
        { 4, 5, 1, 0 }, // front
        { 6, 7, 3, 2 }, // back
        { 7, 4, 0, 3 }, // left
        { 5, 6, 2, 1 }, // right
    };
    
    static const float normals[][3] = {
        {  0.0f, -1.0f,  0.0f }, // bottom
        {  0.0f,  1.0f,  0.0f }, // up
        {  0.0f,  0.0f, -1.0f }, // front
        {  0.0f,  0.0f,  1.0f }, // back
        { -1.0f,  0.0f,  0.0f }, // left
        {  1.0f,  0.0f,  0.0f }, // right
    };

    for (int c = 0; c < PIECE_XCOUNT * PIECE_YCOUNT; c++)
    {
        float* buf = new float [6*6*(3+3+2)];
        m_buffers.push_back(buf);

        for (int i = 0; i < 6; i++)
        {
            const float* normal = normals[i];

            static const int idx[] = { 0, 1, 2, 0, 2, 3 };

            for (int k = 0; k < 6; k++)
            {
                UV uv;
                if (i == 2)
                {
                    uv = piece_uv[4*c+idx[k]];
                }

                *buf++ = vertices[faces[i][idx[k]]][0]*PIECE_SIZEX;
                *buf++ = vertices[faces[i][idx[k]]][1]*PIECE_SIZEY;
                *buf++ = vertices[faces[i][idx[k]]][2]*PIECE_SIZEZ;
                *buf++ = normal[0];
                *buf++ = normal[1];
                *buf++ = normal[2];
                *buf++ = uv.u;
                *buf++ = uv.v;
            }
        }
    }

    m_mesh = new SphereMesh(Vector(BALL_R, BALL_R, BALL_R), 16, 16);
    Input::instance->clearBuffer();
}

Intro::~Intro()
{
    delete m_mesh;

    for each_const(vector<float*>, m_buffers, buf)
    {
        delete [] *buf;
    }

    Video::instance->unloadTextures();
    NewtonDestroyAllBodies(m_newtonWorld);
    NewtonDestroy(m_newtonWorld);
}

void Intro::control()
{
    if (Input::instance->touches().empty() == false && m_timePassed < FADE_OUT_SECS)
    {
        Input::instance->waitForRelease();

        m_timePassed = FADE_OUT_SECS;
        m_ballKicked = true;
    }
    
    if (!m_ballKicked)
    {
        Matrix matrix = Matrix::translate(Vector(0.0f, BALL_R, 2.0f));
        NewtonBodySetMatrix(m_ball_body, matrix.m);
        NewtonBodySetFreezeState(m_ball_body, 0);
        NewtonBodySetVelocity(m_ball_body, Vector(0.0f, 0.0f, -3.0f).v);
        
        m_ballKicked = true;
    }
    else if (m_ballKicked && m_ball_matrix.row(3).z > 0.0f) //BALL_R*2.0f)
    {
        NewtonBodySetVelocity(m_ball_body, Vector(0.0f, 0.0f, -3.0f).v);
    }
}

void Intro::update(float delta)
{
    m_timePassed += delta;

    if (m_timePassed > MENU_SHOW_SECS)
    {
        m_nextState = true;
    }
}

void Intro::updateStep(float delta)
{
    NewtonUpdate(m_newtonWorld, delta);
}

void Intro::prepare()
{
    for (size_t i=0; i<m_piece_body.size(); i++)
    {
        NewtonBodyGetMatrix(m_piece_body[i], m_piece_matrix[i].m);
    }
    NewtonBodyGetMatrix(m_ball_body, m_ball_matrix.m);
}

void Intro::render() const
{
    // clog << "Intro time = " << m_timePassed << endl;
    
    glClear(GL_DEPTH_BUFFER_BIT);

    // camera

    glLoadIdentity();   
    glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, -0.85f, 2.7f);

    glFrontFace(GL_CCW);
        
    glLightfv(GL_LIGHT0, GL_POSITION, Vector(-15.0f, 3.0f, -13.0f).v);
    glLightfv(GL_LIGHT0, GL_AMBIENT, Vector(0,0,0,1).v);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, Vector(1,1,1,1).v);
    glLightfv(GL_LIGHT0, GL_SPECULAR, Vector(1,1,1,1).v);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, Vector::One.v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Vector(0,0,0,0).v);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Vector(0,0,0,1).v);
     
    // background
    {
        const Font* font = Font::get("Arial_32pt_bold");
        const int resX = Video::instance->getResolution().first;
        const int resY = Video::instance->getResolution().second;
        const float aspect = 1.0f-static_cast<float>(resY)/static_cast<float>(resX);

        font->begin();
        glDisable(GL_BLEND);
        Video::instance->setColor(Vector::One);
        m_bigLogoTex->bind();
        glDisableClientState(GL_NORMAL_ARRAY);

        float buf[] = { 0.0f, aspect, 0, 0,
                        0.0f, 1.0f,   0, (float)resY,
                        1.0f, aspect, (float)resX, 0,
                        1.0f, 1.0f,   (float)resX, (float)resY };

        glTexCoordPointer(2, GL_FLOAT, 4*sizeof(float), buf + 0);
        glVertexPointer(2, GL_FLOAT, 4*sizeof(float), buf + 2);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(buf) / sizeof(float) / 4);

        glEnableClientState(GL_NORMAL_ARRAY);

        font->end();
    }

    // cubes

    m_logoTex->bind();
    int i = 0;
    for each_const(vector<Matrix>, m_piece_matrix, iter)
    {
        glPushMatrix();
        glMultMatrixf(iter->m);

        glNormalPointer(GL_FLOAT, (3+3+2)*sizeof(float), m_buffers[i] + 3);
        glTexCoordPointer(2, GL_FLOAT, (3+3+2)*sizeof(float), m_buffers[i] + 6);
        glVertexPointer(3, GL_FLOAT, (3+3+2)*sizeof(float), m_buffers[i]);
        glDrawArrays(GL_TRIANGLES, 0, 6*3*2);

        i++;
        glPopMatrix();
    }

    // ball

    glPushMatrix();
    glMultMatrixf(m_ball_matrix.m);
    m_ballTex->bind();
    m_mesh->render();
    glPopMatrix();

    // fade in
    if (m_timePassed > FADE_OUT_SECS)
    {
        float alpha = (MENU_SHOW_SECS-m_timePassed) / (MENU_SHOW_SECS - FADE_OUT_SECS);
        alpha = alpha*alpha;

        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);

        glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);

        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        float buf[] = {
            0, 0, 0, alpha, 5.0f, -1.0f, -1.8f,
            0, 0, 0, alpha, -5.0f, -1.0f, -1.8f,
            0, 0, 0, alpha, -5.0f, 5.0f, -1.8f,
            0, 0, 0, alpha, 5.0f, 5.0f, -1.8f,
        };

        glColorPointer(4, GL_FLOAT, (4+3)*sizeof(float), buf + 0);
        glVertexPointer(3, GL_FLOAT, (4+3)*sizeof(float), buf + 4);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
    }

    glFrontFace(GL_CW);
}

State::Type Intro::progress()
{
    if (m_nextState)
    {
        return State::Menu;
    }
    return State::Current;
}
