#include "TimerThread.h"

namespace TS_TIMER
{
    //定时器线程
#ifdef WIN32
    DWORD WINAPI thr_fn(void* arg)
#else
    void* thr_fn(void* arg)
#endif
    {
        //得到定时器列表参数
        CTimerInfoList* pTimerInfoList = reinterpret_cast<CTimerInfoList*>(arg);

        //最后运行的TimerID
        int nLastRunTimerID = 0;
        int nTimeCost       = 0;

        CTime_Value obj_Now = TS_TIMER::GetTimeofDay();

        while (pTimerInfoList->Get_Run())
        {
            //得到定时器下一个执行对象的差距时间
            int nInterval = pTimerInfoList->Get_Next_Timer(obj_Now, nTimeCost);

            if (nInterval < 0)
            {
                //计算错误周期
                vector<CTime_Value> vecTimoutList;
                CTime_Value obj_Time_Begin = pTimerInfoList->Get_Curr_Timer()->Get_Next_Time();
                Get_Timout_TimeInfo(obj_Time_Begin,
                                    pTimerInfoList->Get_Curr_Timer()->Get_Timer_Frequency(),
                                    obj_Now,
                                    vecTimoutList);

                pTimerInfoList->Get_Curr_Timer()->Do_Error_Events(nLastRunTimerID, obj_Time_Begin, vecTimoutList);

                //重新计算下一次到期时间
                nInterval = pTimerInfoList->Get_Next_Timer(obj_Now, nTimeCost);
            }

            pTimerInfoList->Lock();

            if (nInterval >= 0)
            {
#ifdef WIN32
                SleepConditionVariableCS(reinterpret_cast<PCONDITION_VARIABLE>(pTimerInfoList->Get_cond()),
                                         reinterpret_cast<PCRITICAL_SECTION>(pTimerInfoList->Get_mutex()),
                                         nInterval);
#else
                struct timespec outtime;

                struct timeval now;
                struct timeval Interval;
                struct timeval abstime;

                Interval.tv_sec = (int)(nInterval / 1000);
                Interval.tv_usec = (nInterval % 1000) * 1000;


                gettimeofday(&now, NULL);
                timeradd(&now, &Interval, &abstime);

                outtime.tv_sec = abstime.tv_sec;
                outtime.tv_nsec = abstime.tv_usec * 1000;  //单位是纳秒

                CTime_Value ttNow = GetTimeofDay();

                pthread_cond_timedwait(pTimerInfoList->Get_cond(),
                                       pTimerInfoList->Get_mutex(),
                                       &outtime);
#endif
            }
            else
            {
                //定时器执行异常，需要调用错误的过程

            }

            if (pTimerInfoList->Get_Event_Type() == TIMER_STOP)
            {
                //关闭当前线程
                printf("[thr_fn]<%d>sig Close.\n", (int)pTimerInfoList->Get_Thread_ID());
            }
            else if (pTimerInfoList->Get_Event_Type() == TIMER_MODIFY)
            {
                //重新计算下一次唤醒时间
                pTimerInfoList->UnLock();
                continue;
            }
            else
            {
                //执行定时器
                CTime_Value obj_Begin = TS_TIMER::GetTimeofDay();

                nLastRunTimerID = pTimerInfoList->Get_Curr_Timer()->Get_Timer_ID();

                if (NULL != pTimerInfoList->Get_Curr_Timer())
                {
                    EM_Timer_State emstate = pTimerInfoList->Get_Curr_Timer()->Do_Timer_Event(obj_Begin);

                    if (TIMER_STATE_DEL == emstate)
                    {
                        //如果执行完需要清除定时器，在这里回收定时器
                        pTimerInfoList->Del_Timer(pTimerInfoList->Get_Curr_Timer()->Get_Timer_ID());
                    }
                }

                obj_Now = TS_TIMER::GetTimeofDay();
                CTime_Value obj_Interval = obj_Now - obj_Begin;

                //计算定时任务执行时间
                nTimeCost = obj_Interval.Get_milliseconds();
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

    void CTimerThread::Init(int nMaxCount)
    {
        Close();

        m_TimerInfoList.Init(nMaxCount);
    }

    void CTimerThread::Close()
    {
        //发起唤醒线程操作
        if (NULL != m_TimerInfoList.Get_mutex() && NULL != m_TimerInfoList.Get_cond())
        {
            Modify(TIMER_STOP);
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
        CTime_Value ttNow = GetTimeofDay();
        printf("[CTimerThread::Run]<%s> is Run.\n", ttNow.Get_string().c_str());
    }

    /*
    * Add_Timer函数，成功返回true
    * nTimerID 定时任务ID
    * nFrequency 时间间隔，单位是毫秒。
    * pttBegin 定时器开始时间（第一次不执行）
    * fn_Timeout_Callback 任务回调事件，不可为空
    * pArgContext 任务到期执行的参数
    * fn_Timeout_Error_Callback 定时器没按时执行的报错事件，可以为空
    */
    bool CTimerThread::Add_Timer(int nTimerID, int nFrequency, CTime_Value* pttBegin, Timeout_Callback fn_Timeout_Callback, void* pArgContext, Timeout_Error_Callback fn_Timeout_Error_Callback)
    {
        ITimerInfo* pTimerInfo = new ITimerInfo();

        if (NULL == fn_Timeout_Callback)
        {
            return false;
        }

        if (NULL != pttBegin)
        {
            //有开始时间
            pTimerInfo->Set_Timer_Param(nTimerID, nFrequency, *pttBegin, fn_Timeout_Callback, pArgContext, fn_Timeout_Error_Callback);
        }
        else
        {
            //从现在开始
            CTime_Value ttBegin = GetTimeofDay();
            pTimerInfo->Set_Timer_Param(nTimerID, nFrequency, ttBegin, fn_Timeout_Callback, pArgContext, fn_Timeout_Error_Callback);
        }

        bool blRet = m_TimerInfoList.Add_Timer(pTimerInfo);

        if (false == blRet)
        {
            delete pTimerInfo;
            pTimerInfo = NULL;
        }
        else
        {
            Modify(TIMER_MODIFY);
        }

        return blRet;
    }

    bool CTimerThread::Del_Timer(int nTimerID)
    {
        bool blRet = m_TimerInfoList.Del_Timer(nTimerID);

        if (true == blRet)
        {
            Modify(TIMER_MODIFY);
        }

        return blRet;
    }

    void CTimerThread::Modify(EM_Event_Type emType)
    {
        m_TimerInfoList.Set_Event_Type(emType);

        m_TimerInfoList.Lock();

        if (NULL != m_TimerInfoList.Get_cond())
        {
#ifdef WIN32
            WakeAllConditionVariable(m_TimerInfoList.Get_cond());
#else
            pthread_cond_signal(m_TimerInfoList.Get_cond());
#endif
        }

        m_TimerInfoList.UnLock();
    }
}
