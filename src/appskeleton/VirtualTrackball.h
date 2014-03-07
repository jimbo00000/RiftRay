// VirtualTrackball.h

#pragma once
#ifdef USE_HYDRA
#include "OVRkill/OVRkill.h"
#include "FlyingMouse.h"
#include <vector>

///@brief A Hydra-modifiable rigid transformation with translation, rotation and scale.
struct Transformation {
    Transformation()
        : m_currentHydraOrientation()
        , m_atClickOrientation()
        , m_accumulatedOrientation()
        , m_currentHydraPosition()
        , m_atClickPosition()
        , m_accumulatedPosition()
        , m_currentHydraScale(1.0f)
        , m_atClickScale(1.0f)
        , m_accumulatedScale(1.0f)
        , m_secondaryScale(1.0f)
    {}

    OVR::Matrix4f  m_currentHydraOrientation;
    OVR::Matrix4f  m_atClickOrientation;
    OVR::Matrix4f  m_accumulatedOrientation;

    OVR::Matrix4f  m_currentHydraPosition;
    OVR::Matrix4f  m_atClickPosition;
    OVR::Matrix4f  m_accumulatedPosition;

    float          m_currentHydraScale;
    float          m_atClickScale;
    float          m_accumulatedScale;

    float          m_secondaryScale;

    virtual void ResetPosition() { m_accumulatedPosition.SetIdentity(); }
    virtual void ResetOrientation() { m_accumulatedOrientation.SetIdentity(); }
    virtual void ResetScale() { m_accumulatedScale = 1.0f; }

    OVR::Matrix4f GetCurrentOrientation() const
    {
        return m_currentHydraOrientation
             * m_atClickOrientation.Transposed()
             * m_accumulatedOrientation;
    }

    /// Using logarithmic scale
    float GetCurrentScaleFactor() const
    {
        const float delta = m_currentHydraScale - m_atClickScale;
        return pow(2.0f, delta);
    }

    float GetCurrentSecondaryScale() const { return m_secondaryScale; }

    /// Reverse transform by the initial rotation to start from identity each gesture
    OVR::Matrix4f GetMatrix() const
    {
        OVR::Vector3f reverseClickTxVec(
            m_atClickPosition.M[0][3],
            m_atClickPosition.M[1][3],
            m_atClickPosition.M[2][3]
            );
        reverseClickTxVec *= -1;
        const OVR::Matrix4f reverseTxMtx = OVR::Matrix4f::Translation(reverseClickTxVec);

        const float scaleFac = GetCurrentScaleFactor();
        OVR::Matrix4f totalMtx = reverseTxMtx
                * m_currentHydraPosition
                * m_accumulatedPosition
                * GetCurrentOrientation()
                * OVR::Matrix4f::Scaling(scaleFac, scaleFac, scaleFac)
                * OVR::Matrix4f::Scaling(m_accumulatedScale, m_accumulatedScale, m_accumulatedScale);
        return totalMtx;
    }
};


///@brief A control interface using the Sixense Hydra to rotate and move an object in 3D space.
/// Basically just encapsulates a matrix.
class VirtualTrackball
{
public:
    VirtualTrackball();
    virtual ~VirtualTrackball();

    virtual void updateHydraData(const FlyingMouse& fm, float headSize);
    virtual void ResetPosition();
    virtual void ResetOrientation();
    virtual void ResetScale();

    OVR::Matrix4f GetCurrentOrientation(int idx=0) const;
    float GetCurrentScaleFactor(int idx=0) const;
    OVR::Matrix4f GetMatrix(int idx=0) const;

    float GetRightTriggerState() const { return m_currentRightTriggerState; }
    float GetCurrentSecondaryScale() const { return m_txs[0].m_secondaryScale; }

protected:
    std::vector<Transformation>  m_txs;
    float           m_currentRightTriggerState;

private: // Disallow copy ctor and assignment operator
    VirtualTrackball(const VirtualTrackball&);
    VirtualTrackball& operator=(const VirtualTrackball&);
};
#endif