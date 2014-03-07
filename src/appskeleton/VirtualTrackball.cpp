// VirtualTrackball.cpp

#include "VirtualTrackball.h"
#ifdef USE_HYDRA
#ifdef _WIN32
// Workaround for std::min throwing errors
#undef min
#undef max
#include <algorithm>
#endif

VirtualTrackball::VirtualTrackball()
: m_txs(2)
{
}

VirtualTrackball::~VirtualTrackball()
{
}

///
/// Handle Hydra rotation and translation gestures
///
void VirtualTrackball::updateHydraData(const FlyingMouse& g_fm, float headSize)
{
    const float* pH = g_fm.mtxR;
    const OVR::Matrix4f hydraMtx(
        pH[0], pH[4], pH[8],
        pH[1], pH[5], pH[9],
        pH[2], pH[6], pH[10]
    );
    OVR::Vector3f txVec(pH[12], pH[13], pH[14]);
    txVec *= 3.0f; // Attempt to match world space here

    ///@todo Find an appropriate scaling curve to relate movement to headsize
    txVec *= sqrt(headSize);

#ifdef USE_HYDRA
    if (g_fm.IsPressed(FlyingMouse::Right, SIXENSE_BUTTON_3))
        txVec *= 0.1f;
#endif
    const OVR::Matrix4f hydraTxMtx = OVR::Matrix4f::Translation(txVec);

    Transformation& tx = m_txs[0];

#ifdef USE_HYDRA
    /// Use y axis to change scale - it is something of a "preferred" axis.
    const float scaleDelta = pH[13];
    const bool moveButtonJustPressed = 
        g_fm.WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_1)
     || g_fm.WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_2);
    const bool moveButtonIsPressed = 
        g_fm.IsPressed(FlyingMouse::Right, SIXENSE_BUTTON_1)
     || g_fm.IsPressed(FlyingMouse::Right, SIXENSE_BUTTON_2);
    const bool moveButtonJustReleased = 
        g_fm.WasJustReleased(FlyingMouse::Right, SIXENSE_BUTTON_1)
     || g_fm.WasJustReleased(FlyingMouse::Right, SIXENSE_BUTTON_2);


    if (g_fm.WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_BUMPER))
    {
        tx.m_atClickOrientation = hydraMtx;
    }
    else if (moveButtonJustPressed)
    {
        tx.m_atClickPosition = hydraTxMtx;
    }

    if (g_fm.IsPressed(FlyingMouse::Right, SIXENSE_BUTTON_BUMPER))
    {
        tx.m_currentHydraOrientation = hydraMtx;
    }
    if (moveButtonIsPressed)
    {
        tx.m_currentHydraPosition = hydraTxMtx;
    }

    if (g_fm.WasJustReleased(FlyingMouse::Right, SIXENSE_BUTTON_BUMPER))
    {
        // This is effectively a *=
        tx.m_accumulatedOrientation = GetCurrentOrientation();

        tx.m_atClickOrientation.SetIdentity();
        tx.m_currentHydraOrientation.SetIdentity();
    }
    else if (moveButtonJustReleased)
    {
        OVR::Vector3f reverseClickTxVec(
            tx.m_atClickPosition.M[0][3],
            tx.m_atClickPosition.M[1][3],
            tx.m_atClickPosition.M[2][3]
            );
        reverseClickTxVec *= -1;
        const OVR::Matrix4f reverseTxMtx = OVR::Matrix4f::Translation(reverseClickTxVec);

        tx.m_accumulatedPosition =
            reverseTxMtx *
            tx.m_currentHydraPosition *
            tx.m_accumulatedPosition;

        tx.m_atClickPosition.SetIdentity();
        tx.m_currentHydraPosition.SetIdentity();
    }

#if 0
    /// Use stick on Hydra controller to change size as well.
    {
        const int controllerID = FlyingMouse::Right;
        const sixenseAllControllerData& state =  g_fm.GetCurrentState();
        const float coeff = 1.0f + 0.02f * state.controllers[controllerID].joystick_y;

        tx.m_accumulatedScale *= coeff;
    }

    {
        const int controllerID = FlyingMouse::Right;
        const sixenseAllControllerData& state =  g_fm.GetCurrentState();
        const float coeff = 1.0f + 0.02f * state.controllers[controllerID].joystick_x;

        tx.m_secondaryScale *= coeff;
        tx.m_secondaryScale = std::min(std::max(0.001f, tx.m_secondaryScale), 0.5f);
    }
#endif

    /// Auxilliary matrices
    {
        Transformation& tx1 = m_txs[1];
        
        if (g_fm.WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_4))
        {
            tx1.m_atClickPosition = hydraTxMtx;
        }

        else if (g_fm.IsPressed(FlyingMouse::Right, SIXENSE_BUTTON_4))
        {
            tx1.m_currentHydraPosition = hydraTxMtx;
        }
        else if (g_fm.WasJustReleased(FlyingMouse::Right, SIXENSE_BUTTON_4))
        {
            OVR::Vector3f reverseClickTxVec(
                tx1.m_atClickPosition.M[0][3],
                tx1.m_atClickPosition.M[1][3],
                tx1.m_atClickPosition.M[2][3]
                );
            reverseClickTxVec *= -1;
            const OVR::Matrix4f reverseTxMtx = OVR::Matrix4f::Translation(reverseClickTxVec);

            tx1.m_accumulatedPosition =
                reverseTxMtx *
                tx1.m_currentHydraPosition *
                tx1.m_accumulatedPosition;

            tx1.m_atClickPosition.SetIdentity();
            tx1.m_currentHydraPosition.SetIdentity();
        }
    }


    const int controllerID = FlyingMouse::Right;
    const sixenseAllControllerData& state =  g_fm.GetCurrentState();
    m_currentRightTriggerState = state.controllers[controllerID].trigger;

    // Press stick in to reset matrices
    if (g_fm.WasJustPressed(FlyingMouse::Right, SIXENSE_BUTTON_JOYSTICK))
    {
        ResetPosition();
        ResetOrientation();
        ResetScale();
    }
#endif // USE_HYDRA
}

void VirtualTrackball::ResetPosition()
{
    for (std::vector<Transformation>::iterator it = m_txs.begin();
        it != m_txs.end();
        ++it)
    {
        Transformation& tx = *it;
        tx.ResetPosition();
    }
}

void VirtualTrackball::ResetOrientation()
{
    for (std::vector<Transformation>::iterator it = m_txs.begin();
        it != m_txs.end();
        ++it)
    {
        Transformation& tx = *it;
        tx.ResetOrientation();
    }
}

void VirtualTrackball::ResetScale()
{
    for (std::vector<Transformation>::iterator it = m_txs.begin();
        it != m_txs.end();
        ++it)
    {
        Transformation& tx = *it;
        tx.ResetScale();
    }
}

OVR::Matrix4f VirtualTrackball::GetCurrentOrientation(int idx) const
{
    if (idx < 0)
        return OVR::Matrix4f();
    if (idx >= (int)m_txs.size())
        return OVR::Matrix4f();

    const Transformation& tx = m_txs[idx];
    return tx.GetCurrentOrientation();
}

float VirtualTrackball::GetCurrentScaleFactor(int idx) const
{
    if (idx < 0)
        return 1.0f;
    if (idx >= (int)m_txs.size())
        return 1.0f;

    const Transformation& tx = m_txs[idx];
    return tx.GetCurrentScaleFactor();
}

OVR::Matrix4f VirtualTrackball::GetMatrix(int idx) const
{
    if (idx < 0)
        return OVR::Matrix4f();
    if (idx >= (int)m_txs.size())
        return OVR::Matrix4f();

    const Transformation& tx = m_txs[idx];
    return tx.GetMatrix();
}
#endif