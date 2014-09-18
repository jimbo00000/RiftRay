// FPSTimer.cpp

#include "FPSTimer.h"

FPSTimer::FPSTimer()
: m_timer()
, m_count(10)
, m_frameTimes()
, m_ringPtr(0)
{
}

FPSTimer::~FPSTimer()
{
}

void FPSTimer::OnFrame()
{
    if (m_frameTimes.size() < m_count)
    {
        m_frameTimes.push_back(m_timer.seconds());
    }
    else
    {
        ++m_ringPtr %= m_count;
        m_frameTimes[m_ringPtr] = m_timer.seconds();
    }
}

void FPSTimer::Reset()
{
    m_frameTimes.clear();
    m_ringPtr = 0;
}

float FPSTimer::GetFPS() const
{
    if (m_frameTimes.size() == m_count)
    {
        const int prevIdx = (m_ringPtr+1) % m_count;
        const double totalTime = m_frameTimes[m_ringPtr] - m_frameTimes[prevIdx];
        return static_cast<float>(m_frameTimes.size()) / static_cast<float>(totalTime);
    }
    else if (m_frameTimes.size() < 2)
    {
        return 0.0f;
    }
    else
    {
        const double totalTime = *(m_frameTimes.end()-1) - *m_frameTimes.begin();
        return static_cast<float>(m_frameTimes.size()) / static_cast<float>(totalTime);
    }
}

float FPSTimer::GetInstantaneousFPS() const
{
    if (m_frameTimes.size() < 2)
    {
        return 0.0f;
    }
    else if (m_frameTimes.size() == m_count)
    {
        const int prevIdx = (m_ringPtr-1+m_count) % m_count;
        const double totalTime = m_frameTimes[m_ringPtr] - m_frameTimes[prevIdx];
        return 1.0f / static_cast<float>(totalTime);
    }
    else
    {
        const double timeInterval = *(m_frameTimes.end()-1) - *(m_frameTimes.end()-2);
        return 1.0f / static_cast<float>(timeInterval);
    }
}
