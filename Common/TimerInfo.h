#ifndef _TIMER_INFO_H
#define _TIMER_INFO_H

//����Timer��ִ�е�Ԫ
//add by freeeyes

#ifdef WIN32
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#else
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#endif
#include "time.h"
#include "TimerCommon.h"
#include "Lcm.h"

//��ʱ����ض�������
//add by freeeyes

namespace ts_timer
{
    enum EM_Timer_State
    {
        TIMER_STATE_OK = 0,    //��ʱ����������
        TIMER_STATE_DEL,       //��ʱ��ɾ��
    };

#define MAX_TIMER_LIST_COUNT 10

    typedef void(*Timeout_Callback)(int, CTime_Value&, void*, EM_Timer_State&);
    typedef void(*Timeout_Error_Callback)(int, int, int, std::vector<CTime_Value>, void*);

    enum EM_Event_Type
    {
        TIMER_STOP = 0,      //ֹͣ�߳�
        TIMER_MODIFY,        //��Timer�仯��
        TIMER_DO_EVENT,      //ִ��Timer
        TIMER_PAUSE,         //��ͣ��ʱ��
        TIMER_RESTORE,       //�ָ���ͣ��ʱ��
    };

    class ITimerInfo
    {
    public:
        ITimerInfo();
        virtual ~ITimerInfo();

        void Set_Timer_Param(int nTimerID, int nFrequency, CTime_Value ttBegin, Timeout_Callback fn_Timeout_Callback, void* pArgContext, Timeout_Error_Callback fn_Timeout_Error_Callback);

        int Get_Timer_ID();

        int Get_Timer_Frequency();

        int Get_Next_Timer(CTime_Value ttNow, bool blState = false);

        CTime_Value Get_Next_Time();

        EM_Timer_State Do_Timer_Event(CTime_Value& obj_Now);

        void Do_Error_Events(int nLastRunTimerID, int nTimeoutTime, std::vector<CTime_Value>& vecTimoutList);

    private:
        int m_nTimerID;                                  //��ǰΨһ��Timer��ʶ
        int m_nFrequency;                                //��ǰ�Ĳ���Ƶ��(��λ�Ǻ���)
        CTime_Value m_ttBeginTime;                       //��ʼ��ʱ����ʱ��
        CTime_Value m_ttLastRunTime;                     //��һ�γɹ����ж�ʱ����ʱ��
        CTime_Value m_ttNextTime;                        //��һ�����е�ʱ��

        Timeout_Callback       m_fn_Timeout_Callback;    //�ص�����
        Timeout_Error_Callback m_fn_Timeout_Error;       //�ص���ʱ��ִ�г�ʱ����
        void*                  m_pArgContext;            //�ص�����������
    };

    //��ʱ�¼��б�
    class CTimerInfoList
    {
    public:
        CTimerInfoList();
        ~CTimerInfoList();

#ifdef WIN32
        CRITICAL_SECTION* Get_mutex();
#else
        pthread_mutex_t* Get_mutex();
#endif

        void Set_Event_Type(EM_Event_Type emEventType);

        EM_Event_Type Get_Event_Type();

#ifdef WIN32
        CONDITION_VARIABLE* Get_cond();
#else
        pthread_cond_t* Get_cond();
#endif

#ifdef WIN32
        void Set_Thread_ID(DWORD nThreadID);
#else
        void Set_Thread_ID(pthread_t nThreadID);
#endif

#ifdef WIN32
        DWORD Get_Thread_ID();
#else
        pthread_t Get_Thread_ID();
#endif

        void Lock();

        void UnLock();

        void Init(int nMaxCount = MAX_TIMER_LIST_COUNT);

        void Close();

        void Set_Run(bool blRun);

        bool Get_Run();

        bool Add_Timer(ITimerInfo* pTimerInfo);

        bool Del_Timer(int nTimerID);

        std::vector<_Lcm_Info>* Get_Curr_Timer();

        void Calculation_Run_Assemble(CTime_Value obj_Now);        //���㶨ʱ��ִ�о��񼯺�

        std::vector<_Lcm_Info>* Get_Curr_Assemble();                    //��õ�ǰҪִ�е��б�

        std::vector<_Lcm_Info>* Get_Next_Assemble();                    //�����һ��Ҫִ�е��б�

        int GetCurrTimerCount();                                   //�õ���ǰ��ʱ����Count

        int GetAssembleCount();                                    //�õ���ǰ�����б������

        ITimerInfo* GetTimerInfo(int nIndex);                      //�õ�ָ����Timerָ��

    private:
        std::vector<ITimerInfo*>             m_TimerList;      //��ǰ��ʱ�������б�
        std::vector<std::vector<_Lcm_Info> > m_TimerAssemble;  //ִ�ж�ʱ���ļƻ�����
        int                                  m_nCurrTimerIndex;//��¼��ǰTimerID
        int                                  m_nMaxCount;      //��ǰ��ʱ�������������
        ITimerInfo*                          m_NextRunTimer;   //��һ��Ҫ���еĶ�ʱ������
        bool                                 m_blRun;          //�Ƿ�����
        EM_Event_Type                        m_emEventType;    //��ǰ�¼�ִ��״̬

#ifdef WIN32
        DWORD                      m_nThreadID;
        CRITICAL_SECTION*          m_pMutex;
        CONDITION_VARIABLE*        m_pCond;
#else
        pthread_t                  m_nThreadID;
        pthread_mutex_t*           m_pMutex;
        pthread_cond_t*            m_pCond;
#endif
    };
}

#endif
