#include "TimerThread.h"

//定时器线程
#ifdef WIN32
DWORD WINAPI thr_fn(void* arg)
#else
void* thr_fn(void* arg)
#endif
{
    //得到定时器列表参数
    CTimerInfoList* pTimerInfoList = reinterpret_cast<CTimerInfoList*>(arg);

    int nTimeCost = 0;

    while (pTimerInfoList->Get_Run())
    {
        //得到定时器下一个执行对象的差距时间
        int nInterval = pTimerInfoList->Get_Next_Timer();

        if (nInterval - nTimeCost <= 0)
        {
            nInterval = 0;
        }
        else
        {
            nInterval -= nTimeCost;
        }

        if (nInterval > 0)
        {
#ifdef WIN32
            SleepConditionVariableCS(reinterpret_cast<PCONDITION_VARIABLE>(pTimerInfoList->Get_cond()),
                                     reinterpret_cast<PCRITICAL_SECTION>(pTimerInfoList->Get_mutex()),
                                     nInterval);
#else
            struct timespec outtime;
            outtime.tv_sec = 0;
            outtime.tv_nsec = nInterval * 1000 * 1000;  //单位是纳秒
            pthread_cond_timedwait(pTimerInfoList->Get_cond(),
                                   pTimerInfoList->Get_mutex(),
                                   &outtime);
#endif
        }

        pTimerInfoList->Lock();

        if (pTimerInfoList->Get_Run() == false)
        {
            //关闭当前线程
            printf("[thr_fn]<%d>sig Close.\n", pTimerInfoList->Get_Thread_ID());
        }
        else
        {
            //执行定时器
            if (NULL != pTimerInfoList->Get_Curr_Timer())
            {
                pTimerInfoList->Get_Curr_Timer()->Do_Timer_Event();
            }
        }

        pTimerInfoList->UnLock();

    }

    return 0;
}

CTimerThread::CTimerThread()
{

}

CTimerThread::~CTimerThread()
{
    m_TimerInfoList.Close();
}

void CTimerThread::Init()
{
    Close();

    m_TimerInfoList.Init();
}

void CTimerThread::Close()
{
    //发起唤醒线程操作
    m_TimerInfoList.Set_Run(false);

    if (NULL != m_TimerInfoList.Get_cond())
    {
#ifdef WIN32
        WakeAllConditionVariable(m_TimerInfoList.Get_cond());
#else
        pthread_cond_signal(m_TimerInfoList.Get_cond());
#endif
    }
}

void CTimerThread::Run()
{
    m_TimerInfoList.Set_Run(true);

#ifdef WIN32
    DWORD nThreadID = 0;
    CreateThread(NULL, 0, thr_fn, (PVOID)&m_TimerInfoList, 0, &nThreadID);
    m_TimerInfoList.Set_Thread_ID(nThreadID);
#else
    pthread_t nThreadID;
    pthread_create(&nThreadID, NULL, thr_fn, (void*)&m_TimerInfoList);
    m_TimerInfoList.Set_Thread_ID(nThreadID);
#endif

    printf("[CTimerThread::Run]<%d> is Run.\n", m_TimerInfoList.Get_Thread_ID());
}

bool CTimerThread::Add_Timer(ITimerInfo* pTimerInfo)
{
    return m_TimerInfoList.Add_Timer(pTimerInfo);
}

bool CTimerThread::Del_Timer(ITimerInfo* pTimerInfo)
{
    return m_TimerInfoList.Del_Timer(pTimerInfo->Get_Timer_ID());
}
