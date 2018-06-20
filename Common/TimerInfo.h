#ifndef _TIMER_INFO_H
#define _TIMER_INFO_H

//定义Timer的执行单元
//add by freeeyes

#ifdef WIN32
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#else
include <stdio.h>
include <sys / time.h>
include <unistd.h>
include <pthread.h>
include <errno.h>
#endif
#include "time.h"
#include <vector>

using namespace std;

#define MAX_TIMER_LIST_COUNT 10

class ITimerInfo
{
public:
    ITimerInfo() : m_nTimerID(-1), m_nFrequency(-1) {};
    virtual ~ITimerInfo() {};

    void Set_Timer_Param(int nTimerID, int nFrequency)
    {
        m_nTimerID   = m_nTimerID;
        m_nFrequency = nFrequency;
    }

    int Get_Timer_ID()
    {
        return m_nTimerID;
    }

    int Get_Timer_Frequency()
    {
        return m_nFrequency;
    }

    int Get_Next_Timer()
    {
        //这里输出下一次运行时间
        if (m_nFrequency <= 0)
        {
            return -1;
        }
        else
        {
            return m_nFrequency;
        }
    }

    //这个需要具体类去实现的方法
    virtual int Do_Timer_Event() = 0;

private:
    int m_nTimerID;        //当前唯一的Timer标识
    int m_nFrequency;      //当前的操作频度(单位是毫秒)

};

//定时事件列表
class CTimerInfoList
{
public:
    CTimerInfoList() : m_nMaxCount(0), m_NextRunTimer(NULL), m_blRun(false) {};
    ~CTimerInfoList() {};

#ifdef WIN32
    CRITICAL_SECTION* Get_mutex()
#else
    pthread_mutex_t* Get_mutex()
#endif
    {
        return m_pMutex;
    }

#ifdef WIN32
    CONDITION_VARIABLE* Get_cond()
#else
    pthread_cond_t* Get_Cond()
#endif
    {
        return m_pCond;
    }

#ifdef WIN32
    void Set_Thread_ID(DWORD nThreadID)
#else
    void Set_Thread_ID(pthread_t nThreadID)
#endif
    {
        m_nThreadID = nThreadID;
    }

#ifdef WIN32
    DWORD Get_Thread_ID()
#else
    pthread_t Get_Thread_ID()
#endif
    {
        return m_nThreadID;
    }

    void Lock()
    {
#ifdef WIN32
        EnterCriticalSection(m_pMutex);
#else
        pthread_mutex_lock(m_pMutex);
#endif
    }

    void UnLock()
    {
#ifdef WIN32
        LeaveCriticalSection(m_pMutex);
#else
        pthread_mutex_unlock(m_pMutex);
#endif
    }

    void Init(int nMaxCount = MAX_TIMER_LIST_COUNT)
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

    void Close()
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

    void Set_Run(bool blRun)
    {
        m_blRun = blRun;
    }

    bool Get_Run()
    {
        return m_blRun;
    }

    bool Add_Timer(ITimerInfo* pTimerInfo)
    {
        if (m_TimerList.size() >= m_nMaxCount)
        {
            return false;
        }

        m_TimerList.push_back(pTimerInfo);
        return true;
    };

    bool Del_Timer(int nTimerID)
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

    int Get_Next_Timer()
    {
        //计算出下一个需要运行的定时器最短到期时间
        int nInterval = 0;
        int nCurrCount = (int)m_TimerList.size();

        for (int i = 0; i < nCurrCount; i++)
        {
            if (i == 0)
            {
                nInterval = m_TimerList[i]->Get_Next_Timer();
                m_NextRunTimer = m_TimerList[i];
            }
            else
            {
                int nCurrInterval = m_TimerList[i]->Get_Next_Timer();

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

    ITimerInfo* Get_Curr_Timer()
    {
        return m_NextRunTimer;
    }

    int Run()
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

private:
    vector<ITimerInfo*> m_TimerList;      //当前定时器对象列表
    int                 m_nMaxCount;      //当前定时器对象最大容量
    ITimerInfo*         m_NextRunTimer;   //下一次要运行的定时器对象
    bool                m_blRun;          //是否运行

#ifdef WIN32
    DWORD               m_nThreadID;
    CRITICAL_SECTION*   m_pMutex;
    CONDITION_VARIABLE* m_pCond;
#else
    pthread_t        m_nThreadID;
    pthread_mutex_t* m_pMutex;
    pthread_cond_t*  m_pCond;
#endif
};

#endif
