// ShaderGalleryScene.cpp

#include "ShaderGalleryScene.h"
#include "ShaderToyPane.h"
#include "DirectoryFunctions.h"
#include "Logger.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <sstream>

ShaderGalleryScene::ShaderGalleryScene()
: PaneScene()
, m_pActiveShaderToy(NULL)
, m_pActiveShaderToyPane(NULL)
, m_texLibrary()
, m_transitionTimer()
, m_transitionState(0)
, m_pMainTweakbar(NULL)
, m_pShaderTweakbar(NULL)
, m_paneDimensionPixels(400)
, m_globalShadertoyState()
, m_useFulldome(false)
, m_pChassisPos(NULL)
{
}

ShaderGalleryScene::~ShaderGalleryScene()
{
}

void ShaderGalleryScene::LoadTextureLibrary()
{
    Timer t;
    std::map<std::string, textureChannel>& texLib = m_texLibrary;
    const std::string texdir("../textures/");
    LoadShaderToyTexturesFromDirectory(texLib, texdir);
    std::cout << "Textures loaded in " << t.seconds() << " seconds." << std::endl;
}

void ShaderGalleryScene::DiscoverShaders(bool recurse)
{
    const std::vector<std::string> shadernames = recurse ?
        GetListOfFilesFromDirectoryAndSubdirs(ShaderToy::s_shaderDir) :
        GetListOfFilesFromDirectory(ShaderToy::s_shaderDir);
    for (std::vector<std::string>::const_iterator it = shadernames.begin();
        it != shadernames.end();
        ++it)
    {
        const std::string& s = *it;

        ShaderToy* pSt = new ShaderToy(s);
        if (pSt == NULL)
            continue;

        Pane* pP = AddShaderToyPane(pSt);
        pP->initGL();
    }
}

Pane* ShaderGalleryScene::AddShaderToyPane(ShaderToy* pSt)
{
    ShaderToyPane* pP = new ShaderToyPane(m_paneDimensionPixels);
    if (pP == NULL)
        return NULL;

    const int idx = static_cast<int>(m_panes.size());
    ///@todo Scalable positioning
    const glm::vec3 pos(-8.0f + static_cast<float>(idx)*1.1f, 1.5f, -2.0f);
    pP->m_tx.SetPosition(pos);
    pP->m_tx.SetDefaultPosition(pos);

    const glm::mat4 ori = glm::mat4(1.0f);
    pP->m_tx.SetDefaultOrientation(ori);
    pP->m_tx.SetOrientation(ori);

    pP->m_pShadertoy = pSt;
    pP->SetTextureLibraryPointer(&m_texLibrary);
    // It feels ugly to do all this pointer setting, but is it worse than a singleton?
    pP->SetFontShaderPointer(&m_fontShader);
    pP->SetFontPointer(&m_font);
    pP->SetGlobalStatePointer(&m_globalShadertoyState);

    m_panes.push_back(pP);

    return pP;
}

void ShaderGalleryScene::RearrangePanes()
{
    int idx = 0;
    for (std::vector<Pane*>::iterator it = m_panes.begin();
        it != m_panes.end();
        ++it, ++idx)
    {
        Pane* pP = *it;

        const int numrows = 3;
        const int rowsz = 1 + static_cast<int>(m_panes.size()) / numrows;
        const int rownum = idx / rowsz;
        const int rowpos = idx % rowsz;
        const float colstep = 1.1f;
#if 1
        // Lay the panes out in cylindrical rows in front of the viewer.
        const float radstep = static_cast<float>(M_PI) / 16.0f;
        const float rads = static_cast<float>(rowpos-rowsz/2) * radstep;
        const float radius = 6.0f;

        const glm::vec3 pos(
            radius*sin(rads),
            0.8f + colstep * static_cast<float>(rownum),
            2.0f - radius*cos(rads));
        const glm::mat4 ori = glm::rotate(glm::mat4(1.0f), -rads, glm::vec3(0,1,0));
#else
        // Lay out panes in a flat grid
        const float xspacing = 1.1f;
        const glm::vec3 pos(
            static_cast<float>(rowpos-rowsz/2) * xspacing,
            0.8f + colstep * static_cast<float>(rownum),
            -6.0f);
        const glm::mat4 ori = glm::mat4(1.0f);
#endif

        pP->m_tx.SetPosition(pos);
        pP->m_tx.SetDefaultPosition(pos);

        pP->m_tx.SetDefaultOrientation(ori);
        pP->m_tx.SetOrientation(ori);
    }
}

void ShaderGalleryScene::CompileShaders()
{
    std::vector<Pane*>& panes = m_panes;
    for (std::vector<Pane*>::iterator it = panes.begin();
        it != panes.end();
        ++it)
    {
        ShaderToyPane* pP = reinterpret_cast<ShaderToyPane*>(*it);
        if (pP == NULL)
            continue;
        ShaderToy* pSt = pP->m_pShadertoy;
        if (pSt == NULL)
            continue;

        Timer t;
        pSt->CompileShader();

        std::cout
            << "\t\t "
            << t.seconds()
            << "s"
            ;
    }
}

const ShaderToyPane* ShaderGalleryScene::GetFocusedPane() const
{
    int idx = 0;
    for (std::vector<Pane*>::const_iterator it = m_panes.begin();
        it != m_panes.end();
        ++it, ++idx)
    {
        const ShaderToyPane* pP = reinterpret_cast<ShaderToyPane*>(*it);
        if (pP == NULL)
            continue;
        if (pP->m_cursorInPane)
            return pP;
    }

    return NULL;
}

void ShaderGalleryScene::RenderForOneEye(const float* pMview, const float* pPersp) const
{
    if (m_bDraw == false)
        return;

    if (m_pActiveShaderToy == NULL)
    {
        // Draw the gallery of panes
        PaneScene::RenderForOneEye(pMview, pPersp);
        return;
    }

    const ShaderToyPane* pP = GetActiveShaderToyPane();
    if (pP == NULL)
        return;

    // Draw only the current ShaderToy encompassing the world
    const glm::mat4 modelview = glm::make_mat4(pMview);
    const glm::mat4 projection = glm::make_mat4(pPersp);

    pP->DrawPaneAsPortal(
        modelview,
        projection,
        glm::mat4(1.0f),
        glm::mat4(1.f),
        1.f,
        m_useFulldome);
}

void ShaderGalleryScene::RenderThumbnails() const
{
    // Render a view of the shader to the FBO
    // We must keep the previously bound FBO and restore
    GLint bound_fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &bound_fbo);

    const ShaderWithVariables& fsh = GetFontShader();
    const BMFont& fnt = GetFont();
    const std::vector<Pane*>& panes = m_panes;
    for (std::vector<Pane*>::const_iterator it = panes.begin();
        it != panes.end();
        ++it)
    {
        const ShaderToyPane* pP = reinterpret_cast<ShaderToyPane*>(*it);
        if (pP == NULL)
            continue;

        pP->RenderThumbnail();
        pP->DrawShaderInfoText(fsh, fnt);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, bound_fbo);
}

///@brief Initiate the change, timestep will call _ToggleShaderWorld after a small delay.
void ShaderGalleryScene::ToggleShaderWorld()
{
    m_transitionState = 1;
    m_transitionTimer.reset();
}

#ifdef USE_ANTTWEAKBAR
static void TW_CALL GoToURLCB(void *clientData)
{
    const ShaderGalleryScene* pThis = reinterpret_cast<ShaderGalleryScene *>(clientData);
    if (!pThis)
        return;

    const ShaderToy* pST = pThis->GetActiveShaderToy();
    if (pST == NULL)
        return;

    const std::string url = pST->GetStringByName("url");
    if (url.empty())
        return;

#ifdef _WIN32
    ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#elif _LINUX
    std::string command = "x-www-browser ";
    command += url;
    system(command.c_str());
#elif __APPLE__
    std::string command = "open ";
    command += url;
    system(command.c_str());
#endif
}

static void TW_CALL ResetShaderVariablesCB(void *clientData)
{
    ShaderToy* pST = (ShaderToy*)clientData;
    if (pST == NULL)
        return;
    pST->ResetVariables();
}
#endif

void ShaderGalleryScene::_ToggleShaderWorld()
{
    if (GetActiveShaderToy() != NULL)
    {
        // Back into gallery
        LOG_INFO("Back to gallery");
        *m_pChassisPos = glm::vec3(0.f);
        //m_chassisYaw = m_chassisYawCached;
        //m_headSize = 1.0f;
        SetActiveShaderToy(NULL);
        SetActiveShaderToyPane(NULL);

#ifdef USE_ANTTWEAKBAR
        //m_dashScene.SendMouseClick(0); // Leaving a drag in progress can cause a crash!
        TwRemoveVar(m_pMainTweakbar, "title");
        TwRemoveVar(m_pMainTweakbar, "author");
        TwRemoveVar(m_pMainTweakbar, "gotourl");
        TwRemoveAllVars(m_pShaderTweakbar);
#endif
        return;
    }

    ShaderToyPane* pP = const_cast<ShaderToyPane*>(GetFocusedPane());
    if (pP == NULL)
        return;

    ShaderToy* pST = pP->m_pShadertoy;
    if (pST == NULL)
        return;

    // Transitioning into shader world
    ///@todo Will we drop frames here? Clear to black if so.
    LOG_INFO("Transition to shadertoy: %s", pST->GetSourceFile().c_str());
    SetActiveShaderToy(pST);
    SetActiveShaderToyPane(pP);

    // Return to the gallery in the same place we left it
    *m_pChassisPos = pST->GetHeadPos();
    //m_chassisYawCached = m_chassisYaw;
    //m_headSize = pST->GetHeadSize();
    //m_chassisYaw = static_cast<float>(M_PI);

#ifdef USE_ANTTWEAKBAR
    const std::string titleStr = "title: " + pST->GetSourceFile();
    const std::string authorStr = "author: " + pST->GetStringByName("author");
    std::stringstream ss;
    // Assemble a string to pass into help here
    ss << " label='"
        << titleStr
        << "' group=Shader ";
    TwAddButton(m_pMainTweakbar, "title", NULL, NULL, ss.str().c_str());
    ss.str("");
    ss << " label='"
        << authorStr
        << "' group=Shader ";
    TwAddButton(m_pMainTweakbar, "author", NULL, NULL, ss.str().c_str());
    TwAddButton(m_pMainTweakbar, "gotourl", GoToURLCB, this, " label='Go to URL'  group='Shader' ");
    TwAddButton(m_pShaderTweakbar, "Reset Variables", ResetShaderVariablesCB, pST, " label='Reset Variables' ");

    // for each var type, add vec3 direction control
    ///@todo Different type widths
    std::map<std::string, shaderVariable>& tweakVars = pST->m_tweakVars;
    for (std::map<std::string, shaderVariable>::iterator it = tweakVars.begin();
        it != tweakVars.end();
        ++it)
    {
        const std::string& name = it->first;
        const shaderVariable& var = it->second;

        std::ostringstream oss;
        oss << " group='Shader' ";
        ETwType t = TW_TYPE_FLOAT;
        if (var.width == 1)
        {
            // Assemble min/max/incr param string for ant
            oss << "min="
                << var.minVal.x
                << " max="
                << var.maxVal.x
                << " step="
                << var.incr
                << " ";
        }
        else if (var.width == 3)
        {
            t = TW_TYPE_DIR3F;
            if (var.varType == shaderVariable::Direction)
            {
                t = TW_TYPE_DIR3F;
            }
            else if (var.varType == shaderVariable::Color)
            {
                t = TW_TYPE_COLOR3F;
            }
            ///@todo handle free, non-normalized values
            else
            {
            }
        }
        const glm::vec4& tv = var.value;
        const std::string vn = name;
        TwAddVarRW(m_pShaderTweakbar, vn.c_str(), t, (void*)glm::value_ptr(tv), oss.str().c_str());
    }
#endif
}

void ShaderGalleryScene::timestep(double absTime, double dt)
{
    if (m_bDraw == false)
        return;
    PaneScene::timestep(absTime, dt);

    // Manage transition animations
    {
        const float duration = 0.15f;
        const float t = static_cast<float>(m_transitionTimer.seconds()) / duration;
        if (t >= 1.0f)
        {
            if (m_transitionState == 1)
            {
                _ToggleShaderWorld();
                m_transitionState = 2;
            }
        }
        if (t < 2.0f)
        {
            // eye blink transition
            //const float fac = std::max(1.0f - t, t - 1.0f);
            //m_cinemaScopeFactor = 1.0f - fac;
        }
        else
        {
            m_transitionState = 0;
        }
    }
}
