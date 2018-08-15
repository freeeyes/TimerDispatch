#include "TimerInfo.h"

namespace TS_TIMER
{

    ITimerInfo::ITimerInfo() : m_nTimerID(-1), m_nFrequency(-1)
    {
        //��ʼ��ʱ��
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

        //����������ǰʱ�䵽��һ��ִ��ʱ��֮��ļ����
        if (m_nFrequency <= 0)
        {
            return -1;
        }
        else
        {
            if (m_ttLastRunTime.IsZero() == true)
            {
                //����ǵ�һ�μ���,������û�г�ʼ��ʱ�����
                if (m_ttBeginTime.IsZero() == true)
                {
                    //��Ҫ����ִ��
                    m_ttLastRunTime = GetTimeofDay();
                    return 0;
                }
                else
                {
                    //�ж��趨�Ŀ�ʼʱ���Ƿ��Ѿ�����
                    CTime_Value ttInterval = ttNow - m_ttBeginTime;

                    int nBeginFrquency = (int)ttInterval.Get_milliseconds() - nFunctionCost;

                    if (nBeginFrquency <= 0)
                    {
                        //��Ҫ����ִ��
                        m_ttLastRunTime = GetTimeofDay();
                        return 0;
                    }
                    else
                    {
                        //��Ҫ�ȴ��ľ���ʱ���
                        nCurrFrequency = nBeginFrquency + m_nFrequency;
                    }
                }
            }
            else
            {
                //�жϺ����ڵ�ʱ����Ƕ���
                int nSeconds = m_nFrequency / 1000;
                int nUseconds = (m_nFrequency % 1000) * 1000;
                CTime_Value ttNextRunTime = m_ttLastRunTime + CTime_Value(nSeconds, nUseconds);
                CTime_Value ttInterval    = ttNextRunTime - ttNow;

                int nIntervalFrquency = ttInterval.Get_milliseconds() - nFunctionCost;

                if (nIntervalFrquency <= 0)
                {
                    //��Ҫ����ִ��
                    m_ttLastRunTime = GetTimeofDay();
                    return 0;
                }
                else
                {
                    //��Ҫ�ȴ��ľ���ʱ���
                    nCurrFrequency = nIntervalFrquency;
                }
            }

            printf("ITimerInfo::Get_Next_Timer]<%s>nCurrFrequency=%d.\n", ttNow.Get_string().c_str(), nCurrFrequency);
            return nCurrFrequency;
        }
    }

    int ITimerInfo::Do_Timer_Event()
    {
        //ִ�лص�����
        m_fn_Timeout_Callback(m_pArgContext);

        //�������ִ��ʱ��
        m_ttLastRunTime = GetTimeofDay();

        return 0;
    }

    //��ʱ���б���
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
        //����Ѱ���Ƿ��Ѿ����ڲ��Ķ�ʱ���ˣ��������ֱ�ӷ���true
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
        //�������һ����Ҫ���еĶ�ʱ����̵���ʱ��
        int nInterval = 0;
        int nCurrCount = (int)m_TimerList.size();
        m_emEventType = TIMER_DO_EVENT;

        //����Ҫ��¼һ�¾���ʱ��
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
