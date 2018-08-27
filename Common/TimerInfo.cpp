#include "TimerInfo.h"

ts_timer::ITimerInfo::ITimerInfo() : m_nTimerID(-1), m_nFrequency(-1)
{
    //初始化时间
    m_ttBeginTime.Set_time(0, 0);
    m_ttLastRunTime.Set_time(0, 0);
    m_fn_Timeout_Error    = NULL;
    m_fn_Timeout_Callback = NULL;
}

ts_timer::ITimerInfo::~ITimerInfo()
{

}

void ts_timer::ITimerInfo::Set_Timer_Param(int nTimerID, int nFrequency, CTime_Value ttBegin, Timeout_Callback fn_Timeout_Callback, void* pArgContext, Timeout_Error_Callback fn_Timeout_Error_Callback)
{
    m_nTimerID            = nTimerID;
    m_nFrequency          = nFrequency;
    m_ttBeginTime         = ttBegin;
    m_fn_Timeout_Callback = fn_Timeout_Callback;
    m_pArgContext         = pArgContext;
    m_fn_Timeout_Error    = fn_Timeout_Error_Callback;
}

int ts_timer::ITimerInfo::Get_Timer_ID()
{
    return m_nTimerID;
}

int ts_timer::ITimerInfo::Get_Timer_Frequency()
{
    return m_nFrequency;
}

int ts_timer::ITimerInfo::Get_Next_Timer(CTime_Value ttNow)
{
    CTime_Value ttInterval;

    if (m_ttLastRunTime.IsZero() == true && m_ttNextTime.IsZero() == true)
    {
        int nSeconds = m_nFrequency / 1000;
        int nUseconds = (m_nFrequency % 1000) * 1000;
        //如果是第一次计算,看看有没有初始化时间参数
        m_ttNextTime = m_ttBeginTime + CTime_Value(nSeconds, nUseconds);
    }

    //如果下一次运行时间小于当前时间
    if (m_ttNextTime.Get_milliseconds() > ttNow.Get_milliseconds())
    {
        ttInterval = m_ttNextTime - ttNow;
        int nIntervalFrquency = ttInterval.Get_milliseconds();
        return nIntervalFrquency;
    }
    else
    {
        return -1;
    }
}

void ts_timer::ITimerInfo::Set_Next_Timer()
{
    int nSeconds = m_nFrequency / 1000;
    int nUseconds = (m_nFrequency % 1000) * 1000;

    //如有上一次运行时间
    m_ttNextTime = m_ttNextTime + CTime_Value(nSeconds, nUseconds);
}

void ts_timer::ITimerInfo::Set_Next_Time(CTime_Value ttNextTime)
{
    m_ttNextTime = ttNextTime;
}

ts_timer::CTime_Value ts_timer::ITimerInfo::Get_Next_Time()
{
    return m_ttNextTime;
}

ts_timer::EM_Timer_State ts_timer::ITimerInfo::Do_Timer_Event(ts_timer::CTime_Value& obj_Now)
{
    //设置最后执行时间
    m_ttLastRunTime = obj_Now;

    //执行回调函数
    EM_Timer_State emState = TIMER_STATE_OK;
    m_fn_Timeout_Callback(Get_Timer_ID(), obj_Now, m_pArgContext, emState);


    //设置下次运行时间
    Set_Next_Timer();

    return emState;
}

void ts_timer::ITimerInfo::Do_Error_Events(int nLastRunTimerID, int nTimeoutTime, CTime_Value& obj_Next, std::vector<CTime_Value>& vecTimoutList)
{
    if (NULL != m_fn_Timeout_Error)
    {
        m_fn_Timeout_Error(nLastRunTimerID, nTimeoutTime, Get_Timer_ID(), vecTimoutList, m_pArgContext);
    }

    //设置下一次执行时间
    Set_Next_Time(obj_Next);
}

//定时器列表类
#ifdef WIN32
CRITICAL_SECTION* ts_timer::CTimerInfoList::Get_mutex()
#else
pthread_mutex_t* ts_timer::CTimerInfoList::Get_mutex()
#endif
{
    return m_pMutex;
}


ts_timer::CTimerInfoList::CTimerInfoList() : m_nMaxCount(0), m_NextRunTimer(NULL), m_blRun(false), m_emEventType(TIMER_DO_EVENT), m_pCond(NULL), m_pMutex(NULL)
{

}

ts_timer::CTimerInfoList::~CTimerInfoList()
{

}

void ts_timer::CTimerInfoList::Set_Event_Type(EM_Event_Type emEventType)
{
    m_emEventType = emEventType;

    if (emEventType == TIMER_STOP)
    {
        m_blRun = false;
    }
}

ts_timer::EM_Event_Type ts_timer::CTimerInfoList::Get_Event_Type()
{
    return m_emEventType;
}

#ifdef WIN32
CONDITION_VARIABLE* ts_timer::CTimerInfoList::Get_cond()
#else
pthread_cond_t* ts_timer::CTimerInfoList::Get_cond()
#endif
{
    return m_pCond;
}

#ifdef WIN32
void ts_timer::CTimerInfoList::Set_Thread_ID(DWORD nThreadID)
#else
void ts_timer::CTimerInfoList::Set_Thread_ID(pthread_t nThreadID)
#endif
{
    m_nThreadID = nThreadID;
}

#ifdef WIN32
DWORD ts_timer::CTimerInfoList::Get_Thread_ID()
#else
pthread_t ts_timer::CTimerInfoList::Get_Thread_ID()
#endif
{
    return m_nThreadID;
}

void ts_timer::CTimerInfoList::Lock()
{
#ifdef WIN32
    EnterCriticalSection(m_pMutex);
#else
    pthread_mutex_lock(m_pMutex);
#endif
}

void ts_timer::CTimerInfoList::UnLock()
{
#ifdef WIN32
    LeaveCriticalSection(m_pMutex);
#else
    pthread_mutex_unlock(m_pMutex);
#endif
}

void ts_timer::CTimerInfoList::Init(int nMaxCount /*= MAX_TIMER_LIST_COUNT*/)
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

void ts_timer::CTimerInfoList::Close()
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

void ts_timer::CTimerInfoList::Set_Run(bool blRun)
{
    m_blRun = blRun;
}

bool ts_timer::CTimerInfoList::Get_Run()
{
    return m_blRun;
}

bool ts_timer::CTimerInfoList::Add_Timer(ITimerInfo* pTimerInfo)
{
    //首先寻找是否已经是内部的定时器了，如果是则直接返回true
    for (std::vector<ITimerInfo*>::iterator it = m_TimerList.begin(); it != m_TimerList.end(); ++it)
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

bool ts_timer::CTimerInfoList::Del_Timer(int nTimerID)
{
    for (std::vector<ITimerInfo*>::iterator it = m_TimerList.begin(); it != m_TimerList.end(); ++it)
    {
        if ((*it)->Get_Timer_ID() == nTimerID)
        {
            ITimerInfo* pTimerInfo = (ITimerInfo*)(*it);
            it = m_TimerList.erase(it);
            delete pTimerInfo;
            pTimerInfo = NULL;
            return true;
        }
    }

    return false;
}

int ts_timer::CTimerInfoList::Get_Next_Timer(CTime_Value& tvNow)
{
    //计算出下一个需要运行的定时器最短到期时间
    int nInterval = 0;
    int nCurrCount = (int)m_TimerList.size();
    m_emEventType = TIMER_DO_EVENT;

    //这里要记录一下绝对时间
    for (int i = 0; i < nCurrCount; i++)
    {
        if (i == 0)
        {
            nInterval = m_TimerList[i]->Get_Next_Timer(tvNow);
            m_NextRunTimer = m_TimerList[i];
        }
        else
        {
            int nCurrInterval = m_TimerList[i]->Get_Next_Timer(tvNow);

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

ts_timer::ITimerInfo* ts_timer::CTimerInfoList::Get_Curr_Timer()
{
    return m_NextRunTimer;
}

int ts_timer::CTimerInfoList::GetCurrTimerCount()
{
    return (int)m_TimerList.size();
}
