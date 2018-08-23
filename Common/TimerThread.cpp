#include "TimerThread.h"

namespace TS_TIMER
{
    //��ʱ���߳�
#ifdef WIN32
    DWORD WINAPI thr_fn(void* arg)
#else
    void* thr_fn(void* arg)
#endif
    {
        //�õ���ʱ���б����
        CTimerInfoList* pTimerInfoList = reinterpret_cast<CTimerInfoList*>(arg);

        //������е�TimerID
        int nLastRunTimerID = 0;
        int nTimeCost       = 0;

        CTime_Value obj_Now = TS_TIMER::GetTimeofDay();

        while (pTimerInfoList->Get_Run())
        {
            //�õ���ʱ����һ��ִ�ж���Ĳ��ʱ��
            int nInterval = pTimerInfoList->Get_Next_Timer(obj_Now, nTimeCost);

            if (nInterval < 0)
            {
                //�����������
                vector<CTime_Value> vecTimoutList;
                CTime_Value obj_Time_Begin = pTimerInfoList->Get_Curr_Timer()->Get_Next_Time();
                Get_Timout_TimeInfo(obj_Time_Begin,
                                    pTimerInfoList->Get_Curr_Timer()->Get_Timer_Frequency(),
                                    obj_Now,
                                    vecTimoutList);

                pTimerInfoList->Get_Curr_Timer()->Do_Error_Events(nLastRunTimerID, obj_Time_Begin, vecTimoutList);

                //���¼�����һ�ε���ʱ��
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
                outtime.tv_nsec = abstime.tv_usec * 1000;  //��λ������

                CTime_Value ttNow = GetTimeofDay();

                pthread_cond_timedwait(pTimerInfoList->Get_cond(),
                                       pTimerInfoList->Get_mutex(),
                                       &outtime);
#endif
            }
            else
            {
                //��ʱ��ִ���쳣����Ҫ���ô���Ĺ���

            }

            if (pTimerInfoList->Get_Event_Type() == TIMER_STOP)
            {
                //�رյ�ǰ�߳�
                printf("[thr_fn]<%d>sig Close.\n", (int)pTimerInfoList->Get_Thread_ID());
            }
            else if (pTimerInfoList->Get_Event_Type() == TIMER_MODIFY)
            {
                //���¼�����һ�λ���ʱ��
                pTimerInfoList->UnLock();
                continue;
            }
            else
            {
                //ִ�ж�ʱ��
                CTime_Value obj_Begin = TS_TIMER::GetTimeofDay();

                nLastRunTimerID = pTimerInfoList->Get_Curr_Timer()->Get_Timer_ID();

                if (NULL != pTimerInfoList->Get_Curr_Timer())
                {
                    EM_Timer_State emstate = pTimerInfoList->Get_Curr_Timer()->Do_Timer_Event(obj_Begin);

                    if (TIMER_STATE_DEL == emstate)
                    {
                        //���ִ������Ҫ�����ʱ������������ն�ʱ��
                        pTimerInfoList->Del_Timer(pTimerInfoList->Get_Curr_Timer()->Get_Timer_ID());
                    }
                }

                obj_Now = TS_TIMER::GetTimeofDay();
                CTime_Value obj_Interval = obj_Now - obj_Begin;

                //���㶨ʱ����ִ��ʱ��
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
        //�������̲߳���
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
    * Add_Timer�������ɹ�����true
    * nTimerID ��ʱ����ID
    * nFrequency ʱ��������λ�Ǻ��롣
    * pttBegin ��ʱ����ʼʱ�䣨��һ�β�ִ�У�
    * fn_Timeout_Callback ����ص��¼�������Ϊ��
    * pArgContext ������ִ�еĲ���
    * fn_Timeout_Error_Callback ��ʱ��û��ʱִ�еı����¼�������Ϊ��
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
