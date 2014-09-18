// VirtualTrackball.h

#pragma once
#include "FlyingMouse.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

///@brief A rigid transformation with translation, rotation and scale that
/// accumulates successive incremental transformations.
struct Transformation
{
    glm::mat4 m_momentaryHydraOrientation;
    glm::mat4 m_atClickOrientation;
    glm::mat4 m_accumulatedOrientation;
    glm::mat4 m_defaultOrientation;

    glm::mat4 m_momentaryHydraPosition;
    glm::mat4 m_atClickPosition;
    glm::mat4 m_accumulatedPosition;
    glm::mat4 m_defaultPosition;

    float m_momentaryHydraScale;
    float m_atClickScale;
    float m_accumulatedScale;
    float m_secondaryScale;

    bool m_lock;
    bool m_lockedAtClickPos;
    bool m_lockedAtClickOri;

    float m_controllerTParamAtClick;
    glm::vec3 m_controllerRayHitPtAClick;

    Transformation()
        : m_momentaryHydraOrientation(1.0f)
        , m_atClickOrientation(1.0f)
        , m_accumulatedOrientation(1.0f)
        , m_defaultOrientation(1.0f)
        , m_momentaryHydraPosition(1.0f)
        , m_atClickPosition(1.0f)
        , m_accumulatedPosition(1.0f)
        , m_defaultPosition(1.0f)
        , m_momentaryHydraScale(1.0f)
        , m_atClickScale(1.0f)
        , m_accumulatedScale(1.0f)
        , m_secondaryScale(1.0f)
        , m_lock(false)
        , m_lockedAtClickPos(false)
        , m_lockedAtClickOri(false)
        , m_controllerTParamAtClick(0.0f)
        , m_controllerRayHitPtAClick(0.0f)
    {}


    virtual void ResetPosition() { m_accumulatedPosition = m_defaultPosition; }
    virtual void ResetOrientation() { m_accumulatedOrientation = m_defaultOrientation; }
    virtual void ResetScale() { m_accumulatedScale = 1.0f; }

    virtual void TranslatePosition(const glm::vec3& t) { m_accumulatedPosition = glm::translate(m_accumulatedPosition, t); }
    virtual void SetPosition(glm::vec3 pos) { m_accumulatedPosition = glm::translate(glm::mat4(1.0f), pos); }
    virtual void SetDefaultPosition(glm::vec3 pos) { m_defaultPosition = glm::translate(glm::mat4(1.0f), pos); }

    virtual void SetOrientation(glm::mat4 ori) { m_accumulatedOrientation = ori; }
    virtual void SetDefaultOrientation(glm::mat4 ori) { m_defaultOrientation = ori; }

    ///@brief The accumulate functions are effectively a *=, applying the momentary
    /// transforms onto the accumulated ones. The value returned by GetMatrix()
    /// will be identical before and after this function is called.
    virtual void AccumulatePosition()
    {
        if (!m_lockedAtClickPos)
        {
            m_accumulatedPosition = 
                glm::translate(glm::mat4(1.0f), -GetReverseClickTxVec()) *
                m_momentaryHydraPosition *
                m_accumulatedPosition;
        }

        m_atClickPosition = glm::mat4(1.0f);
        m_momentaryHydraPosition = glm::mat4(1.0f);
    }

    ///@brief This is effectively a *= for orientation.
    virtual void AccumulateOrientation()
    {
        if (!m_lockedAtClickOri)
            m_accumulatedOrientation = GetCurrentOrientation();

        m_atClickOrientation = glm::mat4(1.0f);
        m_momentaryHydraOrientation = glm::mat4(1.0f);
    }

    virtual glm::mat4 GetCurrentOrientation() const
    {
        return m_momentaryHydraOrientation
            * glm::transpose(m_atClickOrientation)
            * m_accumulatedOrientation;
    }

    /// Using logarithmic scale
    virtual float GetCurrentScaleFactor() const
    {
        const float delta = m_momentaryHydraScale - m_atClickScale;
        return pow(2.0f, delta);
    }

    virtual float GetCurrentSecondaryScale() const { return m_secondaryScale; }

    virtual glm::vec3 GetReverseClickTxVec() const
    {
        return glm::vec3(
            m_atClickPosition[3][0],
            m_atClickPosition[3][1],
            m_atClickPosition[3][2]);
    }

    /// Reverse transform by the initial rotation to start from identity each gesture
    virtual glm::mat4 GetMatrix() const
    {
        if (m_lockedAtClickPos)
        {
            return m_accumulatedPosition
                * m_accumulatedOrientation
                * glm::scale(glm::mat4(1.0f), glm::vec3(m_accumulatedScale));
        }
        else if (m_lockedAtClickOri)
        {
            return glm::translate(glm::mat4(1.0f), -GetReverseClickTxVec())
                * m_momentaryHydraPosition
                * m_accumulatedPosition
                * m_accumulatedOrientation
                * glm::scale(glm::mat4(1.0f), glm::vec3(m_accumulatedScale));
        }

        return glm::translate(glm::mat4(1.0f), -GetReverseClickTxVec())
            * m_momentaryHydraPosition
            * m_accumulatedPosition
            * GetCurrentOrientation()
            * glm::scale(glm::mat4(1.0f), glm::vec3(GetCurrentScaleFactor()))
            * glm::scale(glm::mat4(1.0f), glm::vec3(m_accumulatedScale));
    }
};


///@brief A control interface using the Sixense Hydra to rotate and move in 3D space.
/// Any number of pointers to Transformations can be added to a list and subsequent
/// input will affect all added Transformations.
class VirtualTrackball
{
public:
    VirtualTrackball();
    virtual ~VirtualTrackball();

    virtual void updateHydraData(const FlyingMouse& fm, float headSize=1.0f);

    void AddTransformation(Transformation* pTx) { m_txs.push_back(pTx); }

protected:
    std::vector<Transformation*> m_txs;
    float m_momentaryRightTriggerState;

private: // Disallow copy ctor and assignment operator
    VirtualTrackball(const VirtualTrackball&);
    VirtualTrackball& operator=(const VirtualTrackball&);
};
