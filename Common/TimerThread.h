#ifndef _TIMER_THREAD_H
#define _TIMER_THREAD_H

#include "TimerInfo.h"

namespace TS_TIMER
{
    class CTimerThread
    {
    public:
        CTimerThread();
        ~CTimerThread();

        void Init(int nMaxCount = MAX_TIMER_LIST_COUNT);

        void Close();

        void Run();

        bool Add_Timer(int nTimerID, int nFrequency, CTime_Value ttBegin, Timeout_Callback fn_Timeout_Callback, void* pArgContext);

        bool Del_Timer(int nTimerID);

    private:
        void Modify(EM_Event_Type emType);

        CTimerInfoList   m_TimerInfoList;
    };
}

#endif
