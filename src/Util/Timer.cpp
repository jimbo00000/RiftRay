// Timer.cpp

#ifdef _LINUX

#include "Timer.h"

// Linux's g++ 4.6.3 has trouble linking this when it is just inlined in Timer.h.
///@note This file is #defined to nothing on Windows.
///@todo There is probably a better way to do this.

// http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
timespec diff(const timespec& start, const timespec& end)
{
    timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0)
    {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    }
    else
    {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

#endif
