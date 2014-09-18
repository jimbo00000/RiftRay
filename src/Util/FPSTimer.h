// FPSTimer.h

#pragma once

#include "Timer.h"
#include <vector>

///@brief Keeps a history of elapsed frame times for calculating average FPS.
class FPSTimer
{
public:
    FPSTimer();
    virtual ~FPSTimer();

    void OnFrame();
    void Reset();
    float GetFPS() const;
    float GetInstantaneousFPS() const;

protected:
    Timer m_timer;
    unsigned int m_count; ///< Number of samples in history
    std::vector<double> m_frameTimes;
    unsigned int m_ringPtr;

private: // Disallow copy ctor and assignment operator
    FPSTimer(const FPSTimer&);
    FPSTimer& operator=(const FPSTimer&);
};
