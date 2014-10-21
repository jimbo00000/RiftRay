// main_include.cpp
// This cpp file is #included to glfw_main.cpp and sdl_main.cpp
// to avoid duplication of code.
// It depends on global variables defined in each respective main.
// Yes, this is an ugly technique - but not as bad as repeating the code.


#ifdef USE_ANTTWEAKBAR
static void TW_CALL EnableVSyncCB(void*) { SetVsync(1); }
static void TW_CALL DisableVSyncCB(void*) { SetVsync(0); }
static void TW_CALL AdaptiveVSyncCB(void*) { SetVsync(-1); }
static void TW_CALL RecenterPoseCB(void*) { g_app.RecenterPose(); }
static void TW_CALL StandingCB(void*) { ovrVector3f p = {0,1.78f,2}; g_app.SetChassisPosition(p); }
static void TW_CALL SittingCB(void*) { ovrVector3f p = {0,1.27f,2}; g_app.SetChassisPosition(p); }

static void TW_CALL GetDisplayFPS(void* value, void*)
{
    *(unsigned int *)value = static_cast<unsigned int>(g_fps.GetFPS());
}

static void TW_CALL ResetTimerCB(void *clientData)
{
    static_cast<ShaderToyScene *>(clientData)->ResetTimer();
}

void InitializeBar()
{
    ///@note Bad size errors will be thrown if this is not called before bar creation.
    TwWindowSize(g_auxWindow_w, g_auxWindow_h);

    // Create a tweak bar
    g_pTweakbar = TwNewBar("TweakBar");
    g_app.m_pTweakbar = g_pTweakbar;

    TwDefine(" GLOBAL fontsize=3 ");
    TwDefine(" TweakBar size='300 700' ");

    TwAddButton(g_pTweakbar, "Disable VSync", DisableVSyncCB, NULL, " group='VSync' ");
    TwAddButton(g_pTweakbar, "Enable VSync", EnableVSyncCB, NULL, " group='VSync' ");
    TwAddButton(g_pTweakbar, "Adaptive VSync", AdaptiveVSyncCB, NULL, " group='VSync' ");

    TwAddVarCB(g_pTweakbar, "Display FPS", TW_TYPE_UINT32, NULL, GetDisplayFPS, NULL, " group='Performance' ");

    TwAddVarRW(g_pTweakbar, "Target FPS", TW_TYPE_INT32, &g_targetFPS,
               " min=45 max=200 group='Performance' ");

    TwAddVarRW(g_pTweakbar, "FBO Scale", TW_TYPE_FLOAT, g_app.GetFBOScalePointer(),
               " min=0.05 max=1.0 step=0.005 group='Performance' ");
    TwAddVarRW(g_pTweakbar, "Dynamic FBO Scale", TW_TYPE_BOOLCPP, &g_dynamicallyScaleFBO,
               "  group='Performance' ");
    TwAddVarRW(g_pTweakbar, "DynFBO Smooth", TW_TYPE_FLOAT, &g_fpsSmoothingFactor,
               " min=0.001 max=1.0 step=0.001 group='Performance' ");
    TwAddVarRW(g_pTweakbar, "FPS Delta Threshold", TW_TYPE_FLOAT, &g_fpsDeltaThreshold,
               " min=0.0 max=100.0 step=1.0 group='Performance' ");
    TwAddVarRW(g_pTweakbar, "CinemaScope", TW_TYPE_FLOAT, &g_app.m_cinemaScopeFactor,
               " min=0.0 max=0.95 step=0.005 group='Performance' ");


    TwAddButton(g_pTweakbar, "Recenter Pose", RecenterPoseCB, NULL, " group='Position' ");
    TwAddButton(g_pTweakbar, "Standing", StandingCB, NULL, " group='Position' ");
    TwAddButton(g_pTweakbar, "Sitting", SittingCB, NULL, " group='Position' ");



    TwAddButton(g_pTweakbar, "Reset Timer", ResetTimerCB, &g_app.m_shaderToyScene,
        " label='Reset Timer' group='Shader' ");
    TwAddVarRW(g_pTweakbar, "headSize", TW_TYPE_FLOAT, &g_app.m_headSize,
               " label='headSize' precision=4 min=0.0001 step=0.001 group='Shader' ");
    TwAddVarRW(g_pTweakbar, "eyePos.x", TW_TYPE_FLOAT, &g_app.m_chassisPos.x,
               " label='eyePos.x' step=0.001 group='Shader' ");
    TwAddVarRW(g_pTweakbar, "eyePos.y", TW_TYPE_FLOAT, &g_app.m_chassisPos.y,
               " label='eyePos.y' step=0.001 group='Shader' ");
    TwAddVarRW(g_pTweakbar, "eyePos.z", TW_TYPE_FLOAT, &g_app.m_chassisPos.z,
               " label='eyePos.z' step=0.001 group='Shader' ");



    TwAddVarRW(g_pTweakbar, "Draw Scene", TW_TYPE_BOOLCPP, &g_app.m_scene.m_bDraw,
               "  group='Scene' ");
#ifdef USE_SIXENSE
    TwAddVarRW(g_pTweakbar, "Draw HydraScene", TW_TYPE_BOOLCPP, &g_app.m_hydraScene.m_bDraw,
               "  group='HydraScene' ");
    TwAddVarRW(g_pTweakbar, "Hydra Location x", TW_TYPE_FLOAT, &g_app.m_fm.m_baseOffset.x,
               " min=-10 max=10 step=0.05 group='HydraScene' ");
    TwAddVarRW(g_pTweakbar, "Hydra Location y", TW_TYPE_FLOAT, &g_app.m_fm.m_baseOffset.y,
               " min=-10 max=10 step=0.05 group='HydraScene' ");
    TwAddVarRW(g_pTweakbar, "Hydra Location z", TW_TYPE_FLOAT, &g_app.m_fm.m_baseOffset.z,
               " min=-10 max=10 step=0.05 group='HydraScene' ");
#endif
    TwAddVarRW(g_pTweakbar, "Draw RaymarchScene", TW_TYPE_BOOLCPP, &g_app.m_raymarchScene.m_bDraw,
               "  group='RaymarchScene' ");

    int opened = 0;
    TwSetParam(g_pTweakbar, "Scene", "opened", TW_PARAM_INT32, 1, &opened);
    TwSetParam(g_pTweakbar, "HydraScene", "opened", TW_PARAM_INT32, 1, &opened);
    TwSetParam(g_pTweakbar, "RaymarchScene", "opened", TW_PARAM_INT32, 1, &opened);
}
#endif


// Try to adjust the FBO scale on the fly to match rendering performance.
// Depends on globals g_fps, g_app, g_fpsSmoothingFactor, g_fpsDeltaThreshold, and g_targetFPS.
void DynamicallyScaleFBO()
{
#ifndef APPLE
    // Emergency condition: if we drop below a hard lower limit in any two successive frames,
    // immediately drop the FBO resolution to minimum.
    if (g_fps.GetInstantaneousFPS() < 45.0f)
    {
        g_app.SetFBOScale(0.0f); // bounds checks will choose minimum resolution
        return;
    }
#endif
    const float targetFPS = static_cast<float>(g_targetFPS);
    const float fps = g_fps.GetFPS();
    if (fabs(fps-targetFPS) < g_fpsDeltaThreshold)
        return;

    const float oldScale = g_app.GetFBOScale();
    // scale*scale*fps = targetscale*targetscale*targetfps
    const float scale2Fps = oldScale * oldScale * fps;
    const float targetScale = sqrt(scale2Fps / targetFPS);

    // Use smoothing here to avoid oscillation - blend old and new
    const float t = g_fpsSmoothingFactor;
    const float scale = (1.0f-t)*oldScale + t*targetScale;
    g_app.SetFBOScale(scale);
}
