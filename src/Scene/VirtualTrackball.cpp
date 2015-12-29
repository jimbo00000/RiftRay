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
