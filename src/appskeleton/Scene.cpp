// Scene.cpp

#include "Scene.h"

#ifdef __APPLE__
#include "opengl/gl.h"
#endif

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#  include "win/dirent.h"
#else
#  include <dirent.h>
#endif

#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef USE_CUDA
#else
#  include "vector_make_helpers.h"
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "GL/ShaderFunctions.h"
#include "GL/TextureFunctions.h"
#include "Logger.h"


Scene::Scene()
: m_progBasic(0)
, m_progPlane(0)
, m_progRwwtt(0)
, m_globalTime()
, m_phaseVal(0.0f)
, m_cubeScale(1.0f)
, m_amplitude(1.0f)
, m_eyeballCenterTweak(0.1453f) ///< Magic number from OVR SDK docs
, m_triggerVal(1.0f)
, m_shaderNames()
, m_currentShaderIdx(-1)
#ifdef USE_HYDRA
, g_fm()
, m_vtb()
#endif // USE_HYDRA
, m_texChan0(0)
, m_texChan1(0)
, m_texChan2(0)
, m_texChan3(0)
{
#ifdef USE_HYDRA
    float3 hyoff = {0.0f, 0.5f, -2.75f};
    m_hydraOffset = hyoff;
#endif // USE_HYDRA

    //m_texDim0 = {0,0,0};
    //m_texDim1 = {0,0,0};
    //m_texDim2 = {0,0,0};
    //m_texDim3 = {0,0,0};
}

Scene::~Scene()
{
    glDeleteProgram(m_progBasic);
    glDeleteProgram(m_progPlane);
    glDeleteProgram(m_progRwwtt);
    glDeleteTextures(1, &m_texChan0);
    glDeleteTextures(1, &m_texChan1);
    glDeleteTextures(1, &m_texChan2);
    glDeleteTextures(1, &m_texChan3);
#ifdef USE_HYDRA
    g_fm.Destroy();
#endif // USE_HYDRA
}

// http://stackoverflow.com/questions/612097/how-can-i-get-a-list-of-files-in-a-directory-using-c-or-c
std::vector<std::string> GetListOfFilesFromDirectory(const std::string& d)
{
    std::vector<std::string> names;

    // Get a list of files in the directory
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (d.c_str())) != NULL)
    {
        printf("__List of shaders in %s:__\n", d.c_str());

        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL)
        {
#if 0
            // No lstat on Windows...
            struct stat st;
            lstat(ent->d_name, &st);
            if (S_ISDIR(st.st_mode)
                continue;
#endif

            std::string s(ent->d_name);
            if (s.length() < 5)
                continue;

            printf ("  %s\n", s.c_str());
            names.push_back(s);
        }
        closedir (dir);
    }
    else
    {
        /* could not open directory */
        perror ("");
        //return EXIT_FAILURE;
    }

    std::sort(names.begin(), names.end());
    return names;
}

void Scene::initGL()
{
    m_shaderNames = GetListOfFilesFromDirectory("../shaders/shadertoy/");
    NextShaderName();
    RefreshCurrentShader();

#ifdef USE_HYDRA
    g_fm.Init();
#endif // USE_HYDRA
}

void Scene::Timestep(float dt, float headSize)
{
#ifdef USE_HYDRA
    g_fm.updateHydraData();
    m_vtb.updateHydraData(g_fm, headSize);

    // Set opacity to trigger state
    if (g_fm.GetCurrentState().controllers[0].enabled == 1)
    {
        const float x = m_vtb.GetRightTriggerState();
        //m_triggerVal = 1.0f - x;
        m_triggerVal = 1.0f - sqrt( 1 - (1-x)*(1-x) );
        //m_triggerVal = sqrt( 1 - x*x );
    }
#endif // USE_HYDRA

    m_phaseVal += dt;
}

void Scene::ReloadShaders()
{
    glDeleteProgram(m_progBasic), m_progBasic = 0;
    glDeleteProgram(m_progPlane), m_progPlane = 0;
    glDeleteProgram(m_progRwwtt), m_progRwwtt = 0;

    m_progBasic = makeShaderByName("basic");
    m_progPlane = makeShaderByName("basicplane");
    AssembleRwwttShaderByName(NULL);
}

void Scene::DestroyCurrentShader()
{
    glDeleteProgram(m_progRwwtt);
    m_progRwwtt = 0;
}

void Scene::RefreshCurrentShader()
{
    const std::string& current = GetCurrentShaderName();
    AssembleRwwttShaderByName(current.c_str());
    const std::string suffixless = current.substr(0, current.length()-5);
    m_curShaderName = suffixless;
}

void Scene::PrevShaderName()
{
    if (m_shaderNames.empty())
        return;

    --m_currentShaderIdx;
    m_currentShaderIdx += m_shaderNames.size();
    m_currentShaderIdx %= m_shaderNames.size();
}

void Scene::NextShaderName()
{
    if (m_shaderNames.empty())
        return;

    ++m_currentShaderIdx %= m_shaderNames.size();
}

// http://stackoverflow.com/questions/874134/find-if-string-endswith-another-string-in-c
bool hasEnding (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

void Scene::LoadTextures(const std::vector<std::string>& texs)
{
    glDeleteTextures(1, &m_texChan0);
    glDeleteTextures(1, &m_texChan1);
    glDeleteTextures(1, &m_texChan2);
    glDeleteTextures(1, &m_texChan3);

    m_texChan0 = 0;
    m_texChan1 = 0;
    m_texChan2 = 0;
    m_texChan3 = 0;

    /// Texture image files downloaded from ShaderToy.com
    int idx = 0;
    for (std::vector<std::string>::const_iterator it = texs.begin();
        it != texs.end();
        ++it, ++idx)
    {
        const std::string& t = *it;
        const std::string fullName = std::string("../textures/").append(t);

        GLuint texId = 0;
        GLuint width = 0;
        GLuint height = 0;
        if (hasEnding(fullName, ".jpg"))
            texId = LoadTextureFromJpg(fullName.c_str(), &width, &height);
        else if (hasEnding(fullName, ".png"))
            texId = LoadTextureFromPng(fullName.c_str(), &width, &height);

        ///@todo Arrays
        if (idx == 0)
        {
            m_texChan0 = texId;
            m_texDim0.x = width;
            m_texDim0.y = height;
        }
        else if (idx == 1)
        {
            m_texChan1 = texId;
            m_texDim1.x = width;
            m_texDim1.y = height;
        }
        else if (idx == 2)
        {
            m_texChan2 = texId;
            m_texDim2.x = width;
            m_texDim2.y = height;
        }
        else if (idx == 3)
        {
            m_texChan3 = texId;
            m_texDim3.x = width;
            m_texDim3.y = height;
        }
    }
}


GLuint getShaderIdFromSource(const char* pSrc, const unsigned long Type)
{
    const char* shaderSource = pSrc;
    if (shaderSource == NULL)
        return 0;
    GLint length = strlen(shaderSource);

    GLuint shaderId = glCreateShader(Type);
    glShaderSource(shaderId, 1, &shaderSource, &length);
    glCompileShader(shaderId);

    delete [] shaderSource;

    return shaderId;
}


GLuint GetAssembledShader(const char* pFilename)
{
    const char* name = pFilename;
    if (!name)
        return 0;

    std::string vs("rwwtt");
    std::string fs(name);
    vs += ".vert";
    fs += ".frag";
    
    std::cout
        << std::endl
        << "==========================================================================="
        << std::endl
        << "Assembling Shader: "
        << pFilename
        << std::endl;

    GLuint vertSrc = loadShaderFile(vs.c_str(), GL_VERTEX_SHADER);
    //GLuint fragSrc = loadShaderFile(fs.c_str(), GL_FRAGMENT_SHADER);


    std::string shadertoy = "../shaders/shadertoy/" + std::string(pFilename);
    const GLchar* pSrc1 = GetShaderSourceFromFile("rwwtt_header.glsl");
    const GLchar* pSrc2 = GetShaderSourceFromFile(shadertoy.c_str());
    const GLchar* pSrc3 = GetShaderSourceFromFile("rwwtt_footer.glsl");
    


    GLuint fragSrc = 0;

    GLuint shaderId = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* pSrcs[3] = {
        pSrc1,
        pSrc2,
        pSrc3,
    };
    glShaderSource(shaderId, 3, pSrcs, NULL);
    glCompileShader(shaderId);


    fragSrc = shaderId;



    delete [] pSrc1;
    delete [] pSrc2;
    delete [] pSrc3;

    printShaderInfoLog(vertSrc);
    printShaderInfoLog(fragSrc);

    GLuint program = glCreateProgram();

    glCompileShader(vertSrc);
    glCompileShader(fragSrc);

    glAttachShader(program, vertSrc);
    glAttachShader(program, fragSrc);

    // Will be deleted when program is.
    glDeleteShader(vertSrc);
    glDeleteShader(fragSrc);

    glLinkProgram(program);
    printProgramInfoLog(program);

    glUseProgram(0);
    return program;
}

void Scene::AssembleRwwttShaderByName(const char* pFilename)
{
    glDeleteProgram(m_progRwwtt);
    m_progRwwtt = GetAssembledShader(pFilename);
}


/// Draw an RGB color cube
void Scene::DrawColorCube() const
{
    const float3 minPt = {0,0,0};
    const float3 maxPt = {1,1,1};
    const float3 verts[] = {
        minPt,
        {maxPt.x, minPt.y, minPt.z},
        {maxPt.x, maxPt.y, minPt.z},
        {minPt.x, maxPt.y, minPt.z},
        {minPt.x, minPt.y, maxPt.z},
        {maxPt.x, minPt.y, maxPt.z},
        maxPt,
        {minPt.x, maxPt.y, maxPt.z}
    };
    const uint3 quads[] = {
        {0,3,2}, {1,0,2}, // ccw
        {4,5,6}, {7,4,6},
        {1,2,6}, {5,1,6},
        {2,3,7}, {6,2,7},
        {3,0,4}, {7,3,4},
        {0,1,5}, {4,0,5},
    };

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES,
                   6*3*2, // 6 triangle pairs
                   GL_UNSIGNED_INT,
                   &quads[0]);
}


/// Draw a grid in the xz plane at y=0.
void Scene::DrawGrid() const
{
    const int gridSz = 40;
    const float extent = 16.0f;
    const float3 gray = {0.5f, 0.5f, 0.5f};
    const int numVerts = 4 * (gridSz + 1);

    float3 vVertices[numVerts];
    float3 vColors  [numVerts];

    for (int i=0; i<=gridSz; ++i)
    {
        const float lineValue = -extent + (float)(2*i)*extent/(float)gridSz;
        float3 vert = { -extent, 0.0f, lineValue };
        vVertices[4*i  ] = vert;
        vert.x = extent;
        vVertices[4*i+1] = vert;

        float3 vertY = { lineValue, 0.0f, -extent };
        vVertices[4*i+2] = vertY;
        vertY.z = extent;
        vVertices[4*i+3] = vertY;

        vColors[4*i  ] = gray;
        vColors[4*i+1] = gray;
        vColors[4*i+2] = gray;
        vColors[4*i+3] = gray;
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, vColors);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_LINES, 0, numVerts);
}


/// Utility function to draw colored line segments along unit x,y,z axes
void Scene::DrawOrigin() const
{
    const float3 minPt = {0,0,0};
    const float3 maxPt = {1,1,1};
    const float3 verts[] = {
        {0,0,0},
        {1,0,0},
        {0,1,0},
        {0,0,1},
    };
    const unsigned int lines[] = {
        0,1,
        0,2,
        0,3,
    };

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_LINES,
                   3*2,
                   GL_UNSIGNED_INT,
                   &lines[0]);
}

/// Draw a circle of color cubes(why not)
void Scene::_DrawBouncingCubes(const OVR::Matrix4f& mview) const
{
    const int numCubes = 12;
    for (int i=0; i<numCubes; ++i)
    {
        const float radius = 15.0f;
        const float posPhase = 2.0f * (float)M_PI * (float)i / (float)numCubes;
        const float3 cubePosition = {radius * sin(posPhase), 0.0f, radius * cos(posPhase)};

        const float frequency = 3.0f;
        const float amplitude = m_amplitude;
        float oscVal = amplitude * sin(frequency * (m_phaseVal + posPhase));
        
        OVR::Matrix4f tx = mview * OVR::Matrix4f::Translation(cubePosition.x, oscVal, cubePosition.z);
        const float scale = m_cubeScale;
        tx = tx * OVR::Matrix4f::Scaling(scale, scale, scale);
        glUniformMatrix4fv(getUniLoc(m_progBasic, "mvmtx"), 1, false, &tx.Transposed().M[0][0]);

        DrawColorCube();
    }
}

/// Draw a floor and ceiling using GL_LINES.
/// You will find the the LINES and POINTS primitive do not work in VR - they break depth
/// cues with their different rasterization behavior from TRIANGLES.
void Scene::_DrawSceneWireFrame(const OVR::Matrix4f& mview) const
{
    DrawGrid();

    const float ceilHeight = 3.0f;
    OVR::Matrix4f ceiltx = mview * OVR::Matrix4f::Translation(0.0f, ceilHeight, 0.0f);
    glUniformMatrix4fv(getUniLoc(m_progBasic, "mvmtx"), 1, false, &ceiltx.Transposed().M[0][0]);
    DrawGrid();
}

void DrawPlane()
{
    const float3 minPt = {-10.0f, 0.0f, -10.0f};
    const float3 maxPt = {10.0f, 0.0f, 10.0f};
    const float3 verts[] = {
        minPt.x, minPt.y, minPt.z,
        minPt.x, minPt.y, maxPt.z,
        maxPt.x, minPt.y, maxPt.z,
        maxPt.x, minPt.y, minPt.z,
    };
    const float2 texs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };
    const uint3 quads[] = {
        {0,3,2}, {1,0,2}, // ccw
    };

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texs);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES,
                   3*2, // 2 triangle pairs
                   GL_UNSIGNED_INT,
                   &quads[0]);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Scene::_DrawScenePlanes(const OVR::Matrix4f& mview) const
{
    DrawPlane();

    const float ceilHeight = 3.0f;
    OVR::Matrix4f ceiltx = mview * OVR::Matrix4f::Translation(0.0f, ceilHeight, 0.0f);
    glUniformMatrix4fv(getUniLoc(m_progBasic, "mvmtx"), 1, false, &ceiltx.Transposed().M[0][0]);
    DrawPlane();
}


/// Draw the scene(matrices have already been set up).
void Scene::DrawScene(const OVR::Matrix4f& mview, const OVR::Matrix4f& persp) const
{
    glUseProgram(m_progPlane);
    {
        glUniformMatrix4fv(getUniLoc(m_progPlane, "mvmtx"), 1, false, &mview.Transposed().M[0][0]);
        glUniformMatrix4fv(getUniLoc(m_progPlane, "prmtx"), 1, false, &persp.Transposed().M[0][0]);

        _DrawScenePlanes(mview);
    }
    glUseProgram(0);

    glUseProgram(m_progBasic);
    {
        glUniformMatrix4fv(getUniLoc(m_progBasic, "mvmtx"), 1, false, &mview.Transposed().M[0][0]);
        glUniformMatrix4fv(getUniLoc(m_progBasic, "prmtx"), 1, false, &persp.Transposed().M[0][0]);

        //_DrawSceneWireFrame(mview);
        _DrawBouncingCubes(mview);
    }
    glUseProgram(0);
}

/// Rwwtt
void Scene::RenderForOneEye(
    const OVR::Matrix4f& mview,
    const OVR::Matrix4f& persp,
    int pixelWidth,
    int pixelHeight,
    bool isLeft) const
{
    const GLuint prog = m_progRwwtt;
    glUseProgram(prog);
    {
        const float scale = 1.0f / m_cubeScale;
        OVR::Matrix4f scaled = mview * OVR::Matrix4f::Scaling(scale, scale, scale);

        glUniformMatrix4fv(getUniLoc(prog, "mvmtx"), 1, false, &scaled.Transposed().M[0][0]);
        //glUniformMatrix4fv(getUniLoc(prog, "prmtx"), 1, false, &persp.Transposed().M[0][0]);
        glUniform3f(getUniLoc(prog, "iResolution"), (float)pixelWidth, (float)pixelHeight, 0.0f);

        OVR::Matrix4f relmv = //mview
            //* OVR::Matrix4f::Translation(m_hydraOffset.x, m_hydraOffset.y, m_hydraOffset.z)
            //* OVR::Matrix4f::Scaling(m_hydraScale.x, m_hydraScale.y, m_hydraScale.z)
            OVR::Matrix4f::Scaling(-1,1,-1)
#ifdef USE_HYDRA
            * m_vtb.GetMatrix()
#endif // USE_HYDRA
            * OVR::Matrix4f::Translation(-0.5f, -0.5f, -0.5f); //< Center the unit cube about the origin

        // Anything here is better than throwing an assert! Maybe never demo in debug mode...
        if (relmv.Determinant() > 0.5f)
            relmv.Invert();

        const GLint obUniLoc = glGetUniformLocation(prog, "obmtx");
        if (obUniLoc > -1)
            
        {
            glUniformMatrix4fv(obUniLoc, 1, false, &relmv.Transposed().M[0][0]);
        }

        const GLint timeUniLoc = glGetUniformLocation(prog, "iGlobalTime");
        if (timeUniLoc > -1)
        {
            glUniform1f(timeUniLoc, (float)m_globalTime.seconds());
        }

        const GLint samp0UniLoc = glGetUniformLocation(prog, "iChannel0");
        if (samp0UniLoc != -1)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_texChan0);
            glUniform1i(samp0UniLoc, 0);
        }
        const GLint samp1UniLoc = glGetUniformLocation(prog, "iChannel1");
        if (samp1UniLoc != -1)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, m_texChan1);
            glUniform1i(samp1UniLoc, 1);
        }
        const GLint samp2UniLoc = glGetUniformLocation(prog, "iChannel2");
        if (samp2UniLoc != -1)
        {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, m_texChan2);
            glUniform1i(samp2UniLoc, 2);
        }
        const GLint samp3UniLoc = glGetUniformLocation(prog, "iChannel3");
        if (samp3UniLoc != -1)
        {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, m_texChan3);
            glUniform1i(samp3UniLoc, 3);
        }

        // Channel dims
        const GLint chanResUniLoc = glGetUniformLocation(prog, "iChannelResolution");
        if (chanResUniLoc != -1)
        {
            float dimVals[] = {
                (float)m_texDim0.x, (float)m_texDim0.y, (float)m_texDim0.z, 
                (float)m_texDim1.x, (float)m_texDim1.y, (float)m_texDim1.z, 
                (float)m_texDim2.x, (float)m_texDim2.y, (float)m_texDim2.z, 
                (float)m_texDim3.x, (float)m_texDim3.y, (float)m_texDim3.z, 
            };
            glUniform3fv(chanResUniLoc, 12, dimVals);
        }


        glUniform1f(getUniLoc(prog, "u_eyeballCenterTweak"), isLeft ? -m_eyeballCenterTweak : m_eyeballCenterTweak);

        const GLint triggerUniLoc = glGetUniformLocation(prog, "u_trigger");
        if (triggerUniLoc != -1)
        {
            glUniform1f(triggerUniLoc, m_triggerVal);
        }

        const float3 verts[] = {
            -1.0f, -1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
        };
        const float2 texs[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
        };
        const uint3 tris[] = {
            {0,3,2}, {1,0,2}, // ccw
        };

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        int posAttrib =  glGetAttribLocation(m_progRwwtt, "vPosition");
        int texAttrib =  glGetAttribLocation(m_progRwwtt, "vTex");
        
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, verts);
        glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, texs);
        glEnableVertexAttribArray(posAttrib);
        glEnableVertexAttribArray(texAttrib);

        glDrawElements(GL_TRIANGLES,
                       3*2,
                       GL_UNSIGNED_INT,
                       &tris[0]);

        glDisableVertexAttribArray(posAttrib);
        glDisableVertexAttribArray(texAttrib);
    }
    glUseProgram(0);
}
