#include "TimerThread.h"


//��ʱ���߳�
#ifdef WIN32
DWORD WINAPI thr_fn(void* arg)
#else
void* thr_fn(void* arg)
#endif
{
    //�õ���ʱ���б����
    ts_timer::CTimerInfoList* pTimerInfoList = reinterpret_cast<ts_timer::CTimerInfoList*>(arg);

    //������е�TimerID
    int nLastRunTimerID = 0;
    int nTimeCost       = 0;
    int nInterval       = 0;

    ts_timer::CTime_Value obj_Now = ts_timer::GetTimeofDay();

    while (pTimerInfoList->Get_Run())
    {
        if (pTimerInfoList->Get_Event_Type() != ts_timer::TIMER_PAUSE)
        {
            //�õ���ʱ����һ��ִ�ж���Ĳ��ʱ��
            nInterval = pTimerInfoList->Get_Next_Timer(obj_Now);

            if (nInterval < 0)
            {
                //�����������
                std::vector<ts_timer::CTime_Value> vecTimoutList;
                ts_timer::CTime_Value obj_Time_Begin = pTimerInfoList->Get_Curr_Timer()->Get_Next_Time();
                Get_Timout_TimeInfo(obj_Time_Begin,
                                    pTimerInfoList->Get_Curr_Timer()->Get_Timer_Frequency(),
                                    obj_Now,
                                    vecTimoutList);

                pTimerInfoList->Get_Curr_Timer()->Do_Error_Events(nLastRunTimerID, nTimeCost, obj_Time_Begin, vecTimoutList);

                //���¼�����һ�ε���ʱ��
                nInterval = pTimerInfoList->Get_Next_Timer(obj_Now);
            }
        }

        pTimerInfoList->Lock();

        if (nInterval >= 0 && pTimerInfoList->Get_Event_Type() != ts_timer::TIMER_PAUSE && pTimerInfoList->GetCurrTimerCount() > 0)
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
            outtime.tv_nsec = abstime.tv_usec * 1000;  //��λ������

            pthread_cond_timedwait(pTimerInfoList->Get_cond(),
                                   pTimerInfoList->Get_mutex(),
                                   &outtime);
#endif
        }
        else
        {
            //��ʱ����ͣ�����б�������
            if (pTimerInfoList->Get_Event_Type() == ts_timer::TIMER_PAUSE || pTimerInfoList->GetCurrTimerCount() == 0)
            {
#ifdef WIN32
                SleepConditionVariableCS(reinterpret_cast<PCONDITION_VARIABLE>(pTimerInfoList->Get_cond()),
                                         reinterpret_cast<PCRITICAL_SECTION>(pTimerInfoList->Get_mutex()),
                                         INFINITE);
#else
                pthread_cond_wait(pTimerInfoList->Get_cond(),
                                  pTimerInfoList->Get_mutex());
#endif
            }

        }

        if (pTimerInfoList->Get_Event_Type() == ts_timer::TIMER_STOP)
        {
            //�رյ�ǰ�߳�
            PIRNTF("[thr_fn]sig Close.\n");
            pTimerInfoList->Set_Run(false);
        }
        else if (pTimerInfoList->Get_Event_Type() == ts_timer::TIMER_MODIFY
                 || pTimerInfoList->Get_Event_Type() == ts_timer::TIMER_PAUSE
                 || pTimerInfoList->Get_Event_Type() == ts_timer::TIMER_RESTORE)
        {
            //���¼�����һ�λ���ʱ��
            obj_Now = ts_timer::GetTimeofDay();
            pTimerInfoList->UnLock();
            continue;
        }
        else
        {
            pTimerInfoList->Get_Event_Type();

            //ִ�ж�ʱ��
            ts_timer::CTime_Value obj_Begin = ts_timer::GetTimeofDay();

            nLastRunTimerID = pTimerInfoList->Get_Curr_Timer()->Get_Timer_ID();

            if (NULL != pTimerInfoList->Get_Curr_Timer())
            {
                ts_timer::EM_Timer_State emstate = pTimerInfoList->Get_Curr_Timer()->Do_Timer_Event(obj_Begin);

                if (ts_timer::TIMER_STATE_DEL == emstate)
                {
                    //���ִ������Ҫ�����ʱ������������ն�ʱ��
                    pTimerInfoList->Del_Timer(pTimerInfoList->Get_Curr_Timer()->Get_Timer_ID());
                }
            }

            obj_Now = ts_timer::GetTimeofDay();
            ts_timer::CTime_Value obj_Interval = obj_Now - obj_Begin;

            //���㶨ʱ����ִ��ʱ��
            nTimeCost = obj_Interval.Get_milliseconds();
        }

        pTimerInfoList->UnLock();

    }

    return 0;
}

ts_timer::CTimerThread::CTimerThread()
{

}

ts_timer::CTimerThread::~CTimerThread()
{
    m_TimerInfoList.Close();
}

void ts_timer::CTimerThread::Init(int nMaxCount)
{
    Close();

    m_TimerInfoList.Init(nMaxCount);
}

void ts_timer::CTimerThread::Close()
{
    //�������̲߳���
    if (NULL != m_TimerInfoList.Get_mutex() && NULL != m_TimerInfoList.Get_cond())
    {
        Modify(TIMER_STOP);
    }
}

void ts_timer::CTimerThread::Run()
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
* Add_Timer�������ɹ�����true
* nTimerID ��ʱ����ID
* nFrequency ʱ��������λ�Ǻ��롣
* pttBegin ��ʱ����ʼʱ�䣨��һ�β�ִ�У�
* fn_Timeout_Callback ����ص��¼�������Ϊ��
* pArgContext ������ִ�еĲ���
* fn_Timeout_Error_Callback ��ʱ��û��ʱִ�еı����¼�������Ϊ��
*/
bool ts_timer::CTimerThread::Add_Timer(int nTimerID, int nFrequency, CTime_Value* pttBegin, Timeout_Callback fn_Timeout_Callback, void* pArgContext, Timeout_Error_Callback fn_Timeout_Error_Callback)
{
    if (NULL == fn_Timeout_Callback)
    {
        return false;
    }

    ITimerInfo* pTimerInfo = new ITimerInfo();

    if (NULL != pttBegin)
    {
        //�п�ʼʱ��
        pTimerInfo->Set_Timer_Param(nTimerID, nFrequency, *pttBegin, fn_Timeout_Callback, pArgContext, fn_Timeout_Error_Callback);
    }
    else
    {
        //�����ڿ�ʼ
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

bool ts_timer::CTimerThread::Del_Timer(int nTimerID)
{
    bool blRet = m_TimerInfoList.Del_Timer(nTimerID);

    if (true == blRet)
    {
        Modify(TIMER_MODIFY);
    }

    return blRet;
}

bool ts_timer::CTimerThread::Pause()
{
    Modify(TIMER_PAUSE);
    return true;
}

bool ts_timer::CTimerThread::Restore()
{
    Modify(TIMER_RESTORE);
    return true;
}

void ts_timer::CTimerThread::Modify(EM_Event_Type emType)
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
