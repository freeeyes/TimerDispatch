#ifndef _TIMER_THREAD_H
#define _TIMER_THREAD_H

#include "TimerInfo.h"

class CTimerThread
{
public:
    CTimerThread();
    ~CTimerThread();

    void Init();

    void Close();

    void Run();

    bool Add_Timer(ITimerInfo* pTimerInfo);

    bool Del_Timer(ITimerInfo* pTimerInfo);

private:
    CTimerInfoList   m_TimerInfoList;
};

#endif
