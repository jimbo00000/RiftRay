// FPSTimer.h

#pragma once

#include "Timer.h"

///@brief Keeps a count of elapsed frames and time for calculating average FPS.
class FPSTimer
{
public:
    FPSTimer();
    virtual ~FPSTimer();

    void OnFrame();
    void Reset();
    float GetFPS() const;

protected:
    Timer        m_timer; ///< Platform-independent timer
    unsigned int m_count; ///< Number of samples

private: // Disallow copy ctor and assignment operator
    FPSTimer(const FPSTimer&);
    FPSTimer& operator=(const FPSTimer&);
};
