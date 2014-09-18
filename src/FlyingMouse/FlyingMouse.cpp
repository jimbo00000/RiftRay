// FlyingMouse.cpp

#include "FlyingMouse.h"
#include <glm/gtc/type_ptr.hpp>


///@brief Stateless global helper function
bool buttonWasPressedOnController(
    const sixenseAllControllerData& before,
    const sixenseAllControllerData& after,
    unsigned int controllerID,
    unsigned int button)
{
    bool wasPressed = (before.controllers[controllerID].buttons & button) != 0;
    bool isPressed  = ( after.controllers[controllerID].buttons & button) != 0;
    return isPressed && !wasPressed;
}

///@brief Stateless global helper function
bool buttonWasReleasedOnController(
    const sixenseAllControllerData& before,
    const sixenseAllControllerData& after,
    unsigned int controllerID,
    unsigned int button)
{
    bool wasPressed = (before.controllers[controllerID].buttons & button) != 0;
    bool isPressed  = ( after.controllers[controllerID].buttons & button) != 0;
    return !isPressed && wasPressed;
}

///@brief Stateless global helper function
///@note This one takes a hand param
bool triggerPassedThresholdOnController(
    const sixenseAllControllerData& before,
    const sixenseAllControllerData& after,
    unsigned char which_hand,
    unsigned int controllerID,
    float threshold)
{
    const sixenseControllerData& db = before.controllers[controllerID];
    const sixenseControllerData& da =  after.controllers[controllerID];
    if (which_hand != da.which_hand)
        return false;
    bool wasOver = (db.trigger > threshold);
    bool isOver  = (da.trigger > threshold);
    return isOver && !wasOver;
}

///@brief Stateless global helper function
/// Either controller can reset on either side of the base, but their indices in the
/// controllers[] array are hard-coded as they are tethered.
///@return The index into the controller array of the controller on the given side.
int getHandArrayIndex(
    const sixenseAllControllerData& acd,
    FlyingMouse::Hand hand
    )
{
    const int wh0 = acd.controllers[0].which_hand;
    const int wh1 = acd.controllers[1].which_hand;
    int arrayIdx = 0;
    if (hand == FlyingMouse::Right)
    {
        if (wh0 == 1)
            arrayIdx = 1;
        else
            arrayIdx = 0;
    }
    else
    {
        if (wh1 == 2)
            arrayIdx = 0;
        else
            arrayIdx = 1;
    }

    return arrayIdx;
}


FlyingMouse::FlyingMouse()
: m_active(false)
, m_baseOffset(0.0f, -0.4f, -0.1f)
, m_pChassisPos(NULL)
, m_pChassisYaw(NULL)
{
}

FlyingMouse::~FlyingMouse()
{
}

void FlyingMouse::Init()
{
#ifdef USE_SIXENSE
    sixenseInit();
#endif
}

void FlyingMouse::Destroy()
{
#ifdef USE_SIXENSE
    sixenseExit();
#endif
}

void FlyingMouse::updateHydraData()
{
#ifdef USE_SIXENSE
    //int left_index  = sixenseUtils::getTheControllerManager()->getIndex(sixenseUtils::ControllerManager::P1L);
    //int right_index = sixenseUtils::getTheControllerManager()->getIndex(sixenseUtils::ControllerManager::P1R);

    m_active = false;
    const int maxBases = sixenseGetMaxBases();
    for (int base = 0; base < maxBases; ++base)
    {
        sixenseSetActiveBase(base);
        if (!sixenseIsBaseConnected(base))
            continue;

        sixenseAllControllerData acd;
        sixenseGetAllNewestData(&acd);

        const int maxControllers = sixenseGetMaxControllers();
        for (int cont = 0; cont < maxControllers; cont++)
        {
            if (!sixenseIsControllerEnabled(cont))
               continue;

            m_active = true;
            const sixenseControllerData& cd = acd.controllers[cont];
            float* mtx = mtxL;
            if (cd.which_hand == 2)
                mtx = mtxR;

            mtx[0] = cd.rot_mat[0][0];
            mtx[1] = cd.rot_mat[0][1];
            mtx[2] = cd.rot_mat[0][2];
            mtx[3] = 0.0f;
            mtx[4] = cd.rot_mat[1][0];
            mtx[5] = cd.rot_mat[1][1];
            mtx[6] = cd.rot_mat[1][2];
            mtx[7] = 0.0f;
            mtx[ 8] = cd.rot_mat[2][0];
            mtx[ 9] = cd.rot_mat[2][1];
            mtx[10] = cd.rot_mat[2][2];
            mtx[11] = 0.0f;
            const float posS = 0.001f; // Try to match world space
            mtx[12] = cd.pos[0] * posS;
            mtx[13] = cd.pos[1] * posS;
            mtx[14] = cd.pos[2] * posS;
            mtx[15] = 1.0f;
        }

        g_lastAcd = g_curAcd;
        g_curAcd = acd;
    }
    //sixenseUtils::getTheControllerManager()->update(&acd);
#endif // USE_SIXENSE
}

bool FlyingMouse::IsPressed(Hand h, int buttonID) const
{
    const int idx = getHandArrayIndex(g_curAcd, h);
    const bool isPressed  = (g_curAcd.controllers[idx].buttons & buttonID) != 0;
    return isPressed;
}

bool FlyingMouse::WasJustPressed(int buttonID) const
{
#ifdef USE_SIXENSE
    const int maxControllers = sixenseGetMaxControllers();
    for (int cont=0; cont<maxControllers; cont++)
    {
        if (buttonWasPressedOnController(g_lastAcd, g_curAcd, cont, buttonID))
            return true;
    }
#endif // USE_SIXENSE
    return false;
}

bool FlyingMouse::WasJustPressed(Hand h, int buttonID) const
{
    const int idx = getHandArrayIndex(g_curAcd, h);
    return buttonWasPressedOnController(g_lastAcd, g_curAcd, idx, buttonID);
}

bool FlyingMouse::WasJustReleased(Hand h, int buttonID) const
{
    const int idx = getHandArrayIndex(g_curAcd, h);
    return buttonWasReleasedOnController(g_lastAcd, g_curAcd, idx, buttonID);
}

bool FlyingMouse::TriggerCrossedThreshold(Hand h, float thresh) const
{
#ifdef USE_SIXENSE
    const int maxControllers = sixenseGetMaxControllers();
    for (int cont=0; cont<maxControllers; cont++)
    {
        if (triggerPassedThresholdOnController(g_lastAcd, g_curAcd, static_cast<char>(h), cont, thresh))
            return true;
    }
#endif // USE_SIXENSE
    return false;
}

bool FlyingMouse::TriggerIsOverThreshold(Hand h, float thresh) const
{
#ifdef USE_SIXENSE
    const int maxControllers = sixenseGetMaxControllers();
    for (int cont=0; cont<maxControllers; cont++)
    {
        const sixenseControllerData& da =  g_curAcd.controllers[cont];
        if ((h == da.which_hand) &&
            (da.trigger > thresh))
            return true;
    }
#endif
    return false;
}

float FlyingMouse::GetTriggerValue(Hand h) const
{
#ifdef USE_SIXENSE
    const int maxControllers = sixenseGetMaxControllers();
    for (int cont=0; cont<maxControllers; cont++)
    {
        const sixenseControllerData& da =  g_curAcd.controllers[cont];
        if (h == da.which_hand)
            return da.trigger;
    }
#endif
    return 0.0f;
}

///@brief Get the current location and direction of the FlyingMouse's manipulator.
///@param [in] h Right or Left hand controller
///@param [out] origin
///@param [out] direction
void FlyingMouse::GetControllerOriginAndDirection(Hand h, glm::vec3& origin, glm::vec3& direction) const
{
    const glm::mat4 mR = glm::make_mat4(h == Right ? mtxR : mtxL);
    const glm::vec4 ori4 = mR * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    const glm::vec4 dir4 = mR * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    origin = glm::vec3(ori4) + m_baseOffset + GetChassisPos();
    direction = glm::normalize(glm::vec3(dir4));
}

bool FlyingMouse::ControllerIsOnBase(Hand h) const
{
    const glm::mat4 mR = glm::make_mat4(h == Right ? mtxR : mtxL);
    const glm::vec4 ori4 = mR * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    ///@todo Check along the x axis from the orb center.
    const float dist = glm::length(glm::vec3(ori4));
    return dist < 0.10f; // Distance determined experimentally
}
