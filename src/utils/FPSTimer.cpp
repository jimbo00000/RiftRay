// FPSTimer.cpp

#include "FPSTimer.h"

FPSTimer::FPSTimer()
: m_timer()
, m_count(0)
{
}

FPSTimer::~FPSTimer()
{
}

void FPSTimer::OnFrame()
{
    ++m_count;
    if (m_count > 60)
        Reset();
}

void FPSTimer::Reset()
{
    m_timer.reset();
    m_count = 0;
}

float FPSTimer::GetFPS() const
{
    return (float)m_count / (float)m_timer.seconds();
}
