// VirtualTrackball.cpp

#include "VirtualTrackball.h"

#ifdef _WIN32
// Workaround for std::min throwing errors
#undef min
#undef max
#include <algorithm>
#endif

VirtualTrackball::VirtualTrackball()
: m_txs()
, m_momentaryRightTriggerState(0.0f)
{
}

VirtualTrackball::~VirtualTrackball()
{
}

///@brief Apply some button logic to take input from Sixense via FlyingMouse
/// and apply it to all accumulating transformations held in the pointer vector.
void VirtualTrackball::updateHydraData(const FlyingMouse& fm, float headSize)
{
    const float* pH = fm.mtxR;
    if (pH == NULL)
        return;

    const glm::mat4 fullmtx = glm::make_mat4(pH);
    const glm::mat3 hydraMtx3 = glm::mat3(fullmtx);

    const glm::mat4 hydraMtx(hydraMtx3);
    glm::vec4 txVec4 = fullmtx[3];
    glm::vec3 txVec = glm::vec3(txVec4);

    ///@todo Find an appropriate scaling curve to relate movement to headsize
    txVec *= sqrt(headSize);

#ifdef USE_SIXENSE
    const int moveButton = SIXENSE_BUTTON_BUMPER;
    const int rotateButton = SIXENSE_BUTTON_1 | SIXENSE_BUTTON_3;
    const int scaleButtonUp = SIXENSE_BUTTON_4;
    const int scaleButtonDown = SIXENSE_BUTTON_2;
    const FlyingMouse::Hand hand = FlyingMouse::Right;

    // Apply operations given by Sixense input to all Transformations in the list
    for (std::vector<Transformation*>::iterator it = m_txs.begin();
        it != m_txs.end();
        ++it)
    {
        Transformation* ptx = *it;
        if (ptx == NULL)
            return;
        Transformation& tx = *ptx;

        if (fm.WasJustPressed(hand, rotateButton))
        {
            tx.m_atClickOrientation = hydraMtx;
            tx.m_lockedAtClickOri = tx.m_lock;
        }
        
        if (fm.WasJustPressed(hand, moveButton))
        {
            tx.m_atClickPosition = glm::mat4(1.0f);
            tx.m_lockedAtClickPos = tx.m_lock;

            // Find initial position of hit against controller ray with t param stored by Scene.
            if (tx.m_controllerTParamAtClick > 0.0f)
            {
                glm::vec3 origin3;
                glm::vec3 dir3;
                fm.GetControllerOriginAndDirection(FlyingMouse::Right, origin3, dir3);
                const glm::vec3 newPt = origin3 + tx.m_controllerTParamAtClick * dir3;
                tx.m_controllerRayHitPtAClick = newPt;
            }
        }

        if (fm.IsPressed(hand, rotateButton))
        {
            tx.m_momentaryHydraOrientation = hydraMtx;
        }
        if (fm.IsPressed(hand, moveButton))
        {
            // Take t param along controller ray and calculate new positional delta for tx.
            if (tx.m_controllerTParamAtClick > 0.0f)
            {
                glm::vec3 origin3;
                glm::vec3 dir3;
                fm.GetControllerOriginAndDirection(FlyingMouse::Right, origin3, dir3);
                const glm::vec3 newPt = origin3 + tx.m_controllerTParamAtClick * dir3;
                tx.m_momentaryHydraPosition = glm::translate(glm::mat4(1.0f), newPt - tx.m_controllerRayHitPtAClick);
            }
        }

        if (fm.WasJustReleased(hand, rotateButton))
        {
            tx.AccumulateOrientation();
            tx.m_lockedAtClickOri = false;
        }

        if (fm.WasJustReleased(hand, moveButton))
        {
            tx.AccumulatePosition();
            tx.m_lockedAtClickPos = false;
            tx.m_controllerTParamAtClick = 0.0f;
            tx.m_controllerRayHitPtAClick = glm::vec3(0.0f);
        }

        ///@note The action here is dependent on frame rate
        if (!tx.m_lock)
        {
            const float scaleCoeff = 1.0015f;
            if (fm.IsPressed(hand, scaleButtonUp))
            {
                tx.m_accumulatedScale *= scaleCoeff;
            }
            else if (fm.IsPressed(hand, scaleButtonDown))
            {
                tx.m_accumulatedScale /= scaleCoeff;
            }
        }

        // Press stick in to reset matrices
        if (fm.IsPressed(hand, moveButton) && fm.WasJustPressed(hand, SIXENSE_BUTTON_JOYSTICK))
        {
            tx.ResetPosition();
            tx.ResetOrientation();
            tx.ResetScale();
        }
    }

#if 0
    // Recenter the base's location by holding one start button, then moving the
    // other controller to the desired location and pressing its start button.
    const int recenterButton = SIXENSE_BUTTON_START;
    if (fm.IsPressed(FlyingMouse::Left, recenterButton) &&
        fm.WasJustPressed(FlyingMouse::Right, recenterButton))
    {
        const float* pMtx = fm.mtxR;
        m_baseOffset += glm::vec3(pMtx[12], pMtx[13], pMtx[14]);
    }
    else if (fm.IsPressed(FlyingMouse::Right, recenterButton) &&
        fm.WasJustPressed(FlyingMouse::Left, recenterButton))
    {
        const float* pMtx = fm.mtxL;
        m_baseOffset += glm::vec3(pMtx[12], pMtx[13], pMtx[14]);
    }
#endif

#endif // USE_SIXENSE
}
