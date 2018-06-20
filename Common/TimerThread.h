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
    void Modify(EM_Event_Type emType);

    CTimerInfoList   m_TimerInfoList;
};

#endif
