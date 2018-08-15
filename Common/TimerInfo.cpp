#include "TimerInfo.h"

namespace TS_TIMER
{

    ITimerInfo::ITimerInfo() : m_nTimerID(-1), m_nFrequency(-1)
    {
        //初始化时间
        m_ttBeginTime.Set_time(0, 0);
        m_ttLastRunTime.Set_time(0, 0);
    }

    ITimerInfo::~ITimerInfo()
    {

    }

    void ITimerInfo::Set_Timer_Param(int nTimerID, int nFrequency, CTime_Value ttBegin, Timeout_Callback fn_Timeout_Callback, void* pArgContext)
    {
        m_nTimerID            = nTimerID;
        m_nFrequency          = nFrequency;
        m_ttBeginTime         = ttBegin;
        m_fn_Timeout_Callback = fn_Timeout_Callback;
        m_pArgContext         = pArgContext;
    }

    int ITimerInfo::Get_Timer_ID()
    {
        return m_nTimerID;
    }

    int ITimerInfo::Get_Timer_Frequency()
    {
        return m_nFrequency;
    }

    int ITimerInfo::Get_Next_Timer(int nFunctionCost)
    {
        int nCurrFrequency = 0;
        CTime_Value ttNow  = GetTimeofDay();

        //这里计算出当前时间到下一次执行时间之间的间隔。
        if (m_nFrequency <= 0)
        {
            return -1;
        }
        else
        {
            if (m_ttLastRunTime.IsZero() == true)
            {
                //如果是第一次计算,看看有没有初始化时间参数
                if (m_ttBeginTime.IsZero() == true)
                {
                    //需要立即执行
                    m_ttLastRunTime = GetTimeofDay();
                    return 0;
                }
                else
                {
                    //判断设定的开始时间是否已经到了
                    CTime_Value ttInterval = ttNow - m_ttBeginTime;

                    int nBeginFrquency = (int)ttInterval.Get_milliseconds() - nFunctionCost;

                    if (nBeginFrquency <= 0)
                    {
                        //需要立即执行
                        m_ttLastRunTime = GetTimeofDay();
                        return 0;
                    }
                    else
                    {
                        //需要等待的绝对时间差
                        nCurrFrequency = nBeginFrquency + m_nFrequency;
                    }
                }
            }
            else
            {
                //判断和现在的时间差是多少
                int nSeconds = m_nFrequency / 1000;
                int nUseconds = (m_nFrequency % 1000) * 1000;
                CTime_Value ttNextRunTime = m_ttLastRunTime + CTime_Value(nSeconds, nUseconds);
                CTime_Value ttInterval    = ttNextRunTime - ttNow;

                int nIntervalFrquency = ttInterval.Get_milliseconds() - nFunctionCost;

                if (nIntervalFrquency <= 0)
                {
                    //需要立即执行
                    m_ttLastRunTime = GetTimeofDay();
                    return 0;
                }
                else
                {
                    //需要等待的绝对时间差
                    nCurrFrequency = nIntervalFrquency;
                }
            }

            printf("ITimerInfo::Get_Next_Timer]<%s>nCurrFrequency=%d.\n", ttNow.Get_string().c_str(), nCurrFrequency);
            return nCurrFrequency;
        }
    }

    int ITimerInfo::Do_Timer_Event()
    {
        //执行回调函数
        m_fn_Timeout_Callback(m_pArgContext);

        //设置最后执行时间
        m_ttLastRunTime = GetTimeofDay();

        return 0;
    }

    //定时器列表类
#ifdef WIN32
    CRITICAL_SECTION* CTimerInfoList::Get_mutex()
#else
    pthread_mutex_t* CTimerInfoList::Get_mutex()
#endif
    {
        return m_pMutex;
    }

    void CTimerInfoList::Set_Event_Type(EM_Event_Type emEventType)
    {
        m_emEventType = emEventType;

        if (emEventType == TIMER_STOP)
        {
            m_blRun = false;
        }
    }

    EM_Event_Type CTimerInfoList::Get_Event_Type()
    {
        return m_emEventType;
    }

#ifdef WIN32
    CONDITION_VARIABLE* CTimerInfoList::Get_cond()
#else
    pthread_cond_t* CTimerInfoList::Get_cond()
#endif
    {
        return m_pCond;
    }

#ifdef WIN32
    void CTimerInfoList::Set_Thread_ID(DWORD nThreadID)
#else
    void CTimerInfoList::Set_Thread_ID(pthread_t nThreadID)
#endif
    {
        m_nThreadID = nThreadID;
    }

#ifdef WIN32
    DWORD CTimerInfoList::Get_Thread_ID()
#else
    pthread_t CTimerInfoList::Get_Thread_ID()
#endif
    {
        return m_nThreadID;
    }

    void CTimerInfoList::Lock()
    {
#ifdef WIN32
        EnterCriticalSection(m_pMutex);
#else
        pthread_mutex_lock(m_pMutex);
#endif
    }

    void CTimerInfoList::UnLock()
    {
#ifdef WIN32
        LeaveCriticalSection(m_pMutex);
#else
        pthread_mutex_unlock(m_pMutex);
#endif
    }

    void CTimerInfoList::Init(int nMaxCount /*= MAX_TIMER_LIST_COUNT*/)
    {
#ifdef WIN32
        m_pMutex = new CRITICAL_SECTION();
        m_pCond = new CONDITION_VARIABLE();
#else
        m_pMutex = new pthread_mutex_t();
        m_pCond = new pthread_cond_t();
#endif

#ifdef WIN32
        InitializeConditionVariable(m_pCond);
        InitializeCriticalSection(m_pMutex);
#else
        pthread_cond_init(m_pCond, NULL);
        pthread_mutex_init(m_pMutex, NULL);
#endif

        m_nMaxCount = nMaxCount;
    }

    void CTimerInfoList::Close()
    {
        if (NULL != m_pMutex && NULL != m_pCond)
        {
#ifdef WIN32
            DeleteCriticalSection(m_pMutex);
            delete m_pMutex;
            delete m_pCond;
            m_pMutex = NULL;
            m_pCond = NULL;
#else
            pthread_mutex_destroy(m_pMutex);
            pthread_cond_destroy(m_pCond);

            delete m_pMutex;
            delete m_pCond;
            m_pMutex = NULL;
            m_pCond = NULL;
#endif
        }
    }

    void CTimerInfoList::Set_Run(bool blRun)
    {
        m_blRun = blRun;
    }

    bool CTimerInfoList::Get_Run()
    {
        return m_blRun;
    }

    bool CTimerInfoList::Add_Timer(ITimerInfo* pTimerInfo)
    {
        //首先寻找是否已经是内部的定时器了，如果是则直接返回true
        for (vector<ITimerInfo*>::iterator it = m_TimerList.begin(); it != m_TimerList.end(); ++it)
        {
            if ((*it)->Get_Timer_ID() == pTimerInfo->Get_Timer_ID())
            {
                return true;
            }
        }

        if (m_TimerList.size() >= m_nMaxCount)
        {
            return false;
        }

        m_TimerList.push_back(pTimerInfo);
        return true;
    }

    bool CTimerInfoList::Del_Timer(int nTimerID)
    {
        for (vector<ITimerInfo*>::iterator it = m_TimerList.begin(); it != m_TimerList.end(); ++it)
        {
            if ((*it)->Get_Timer_ID() == nTimerID)
            {
                it = m_TimerList.erase(it);
                return true;
            }
        }

        return false;
    }

    int CTimerInfoList::Get_Next_Timer(int nFunctionCost)
    {
        //计算出下一个需要运行的定时器最短到期时间
        int nInterval = 0;
        int nCurrCount = (int)m_TimerList.size();
        m_emEventType = TIMER_DO_EVENT;

        //这里要记录一下绝对时间
        CTime_Value tvNow = GetTimeofDay();

        for (int i = 0; i < nCurrCount; i++)
        {
            if (i == 0)
            {
                nInterval = m_TimerList[i]->Get_Next_Timer(nFunctionCost);
                m_NextRunTimer = m_TimerList[i];
            }
            else
            {
                int nCurrInterval = m_TimerList[i]->Get_Next_Timer(nFunctionCost);

                if (nCurrInterval < nInterval)
                {
                    nInterval = nCurrInterval;
                    m_NextRunTimer = m_TimerList[i];
                }
            }

            if (nInterval <= 0)
            {
                return nInterval;
            }
        }

        return nInterval;
    }

    ITimerInfo* CTimerInfoList::Get_Curr_Timer()
    {
        return m_NextRunTimer;
    }

    int CTimerInfoList::Run()
    {
        if (NULL != m_NextRunTimer)
        {
            return m_NextRunTimer->Do_Timer_Event();
        }
        else
        {
            return -1;
        }
    }
}
