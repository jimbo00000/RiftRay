// FlyingMouse.cpp

#include "FlyingMouse.h"

FlyingMouse::FlyingMouse()
 : m_active(false)
{
}

FlyingMouse::~FlyingMouse()
{
}

void FlyingMouse::Init()
{
#ifdef USE_HYDRA
    sixenseInit();

    // Init the controller manager. This makes sure the controllers are present, assigned to left and right hands, and that
    // the hemisphere calibration is complete.
    sixenseUtils::getTheControllerManager()->setGameType(sixenseUtils::ControllerManager::ONE_PLAYER_TWO_CONTROLLER);
    //sixenseUtils::getTheControllerManager()->registerSetupCallback(controller_manager_setup_callback);
    
#endif // USE_HYDRA
}

void FlyingMouse::Destroy()
{
#ifdef USE_HYDRA
    sixenseExit();
#endif // USE_HYDRA
}


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


void FlyingMouse::updateHydraData()
{
#ifdef USE_HYDRA
    sixenseAllControllerData acd;
    int left_index  = sixenseUtils::getTheControllerManager()->getIndex(sixenseUtils::ControllerManager::P1L);
    int right_index = sixenseUtils::getTheControllerManager()->getIndex(sixenseUtils::ControllerManager::P1R);

    m_active = false;
    int base, cont;
    int maxBases = sixenseGetMaxBases();
    for (base=0; base<maxBases; ++base)
    {
        sixenseSetActiveBase(base);
        if (!sixenseIsBaseConnected(base))
            continue;

        sixenseGetAllNewestData(&acd);

        int maxControllers = sixenseGetMaxControllers();
        for (cont=0; cont<maxControllers; cont++)
        {
            if (sixenseIsControllerEnabled(cont))
            {
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
                const float posS = 0.003f;
                mtx[12] = cd.pos[0] * posS;
                mtx[13] = cd.pos[1] * posS;
                mtx[14] = cd.pos[2] * posS;
                mtx[15] = 1.0f;

                /// Check for button presses
                unsigned int buttonlist[] = {
                    SIXENSE_BUTTON_1,
                    SIXENSE_BUTTON_2,
                    SIXENSE_BUTTON_3,
                    SIXENSE_BUTTON_4,
                    SIXENSE_BUTTON_BUMPER,
                    SIXENSE_BUTTON_JOYSTICK,
                    SIXENSE_BUTTON_START,
                };

#if 0
                const int bcount = sizeof(buttonlist)/sizeof(buttonlist[0]);
                for (int i=0; i<bcount; ++i)
                {
                    const unsigned int but = buttonlist[i];
                    if (buttonWasPressedOnController(g_curAcd, acd, cont, but))
                    {
                        printf("button %d pressed on %d   hand %d\n", but, cont, cd.which_hand);
                    }
                }

                if (cd.trigger > 0.0f)
                {
                    printf("Trigger[%d]: %f\n", cont, cd.trigger);
                }

                if (
                    (fabs(cd.joystick_x) > 0.0f) ||
                    (fabs(cd.joystick_y) > 0.0f)
                   )
                {
                    printf("joystick[%d]: %f   %f\n", cont, cd.joystick_x, cd.joystick_y);
                }
#endif

            } //sixenseIsControllerEnabled(cont)
        }


        g_lastAcd = g_curAcd;
        g_curAcd = acd;

    }
    //sixenseUtils::getTheControllerManager()->update(&acd);
#endif // USE_HYDRA
}

bool FlyingMouse::IsPressed(Hand h, int buttonID) const
{
    bool isPressed  = ( g_curAcd.controllers[h].buttons & buttonID) != 0;
    return isPressed;
}

bool FlyingMouse::WasJustPressed(int buttonID) const
{
#ifdef USE_HYDRA
    int maxControllers = sixenseGetMaxControllers();
    for (int cont=0; cont<maxControllers; cont++)
    {
        if (buttonWasPressedOnController(g_lastAcd, g_curAcd, cont, buttonID))
            return true;
    }
#endif // USE_HYDRA
    return false;
}

bool FlyingMouse::WasJustPressed(Hand h, int buttonID) const
{
    return buttonWasPressedOnController(g_lastAcd, g_curAcd, h, buttonID);
}

bool FlyingMouse::WasJustReleased(Hand h, int buttonID) const
{
    return buttonWasReleasedOnController(g_lastAcd, g_curAcd, h, buttonID);
}

bool FlyingMouse::TriggerCrossedThreshold(Hand h, float thresh) const
{
#ifdef USE_HYDRA
    int maxControllers = sixenseGetMaxControllers();
    for (int cont=0; cont<maxControllers; cont++)
    {
        if (triggerPassedThresholdOnController(g_lastAcd, g_curAcd, h, cont, thresh))
            return true;
    }
#endif // USE_HYDRA
    return false;
}

bool FlyingMouse::TriggerIsOverThreshold(Hand h, float thresh) const
{
#ifdef USE_HYDRA
    int maxControllers = sixenseGetMaxControllers();
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

