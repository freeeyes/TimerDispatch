[![Build Status](https://www.travis-ci.org/freeeyes/TimerDispatch.svg?branch=master)](https://www.travis-ci.org/freeeyes/TimerDispatch)
[![Quality Gate](https://sonarcloud.io/api/project_badges/measure?project=TimerDispatch&metric=alert_status](https://sonarcloud.io/dashboard?id=TimerDispatch)

# Timer
> This is a timer that supports windows and Linux.  
> Supports absolute time to execute a timed task. If the timed task does not run at the specified time, an error event will pop up.  
# Features  
- Timed tasks supporting the specified time.  
- Support binding timer exception function.  
- Support timer start and pause.  
# How to use
you can make tow youself function sush as:  
```cpp
//timer task function
// nTimerID : you add time task ID
// tvNow    : Timed task execution time
// pArg     : Timed task arg
// emState  : Time Task Does the current timing task need to be cancel? TIMER_STATE_OK is run, TIMER_STATE_DEL is cancel, default TIMER_STATE_OK
void Do_Timer_Event(int nTimerID, TS_TIMER::CTime_Value& tvNow, void* pArg, TS_TIMER::EM_Timer_State& emState)
{
    int* pData = (int*)pArg;
    printf("[Do_Timer_Event](%d)<%s>, Arg=%d.\n", nTimerID, tvNow.Get_string().c_str(), *pData);

	//do your task
}


//timer run error(Timed tasks are not executed at the specified time.)
//nErrorTimerID : Timeout TimerID
//nTimerID      : Currently affected TimerID
//vecTimeout    : Currently affected time list
//pArg          : Currently time task arg
void Timeout_Error_Callback(int nErrorTimerID, int nTimerID, vector<TS_TIMER::CTime_Value> vecTimeout, void* pArg)
{
    int* pData = (int*)pArg;
    int nCount = (int)vecTimeout.size();
    printf("[Timeout_Error_Callback](nErrorTimerID=%d,nTimerID=%d)<%d>, Arg=%d.\n", nErrorTimerID, nTimerID, nCount, *pData);
}
```  
an then you can run timer:  
```cpp  

    TS_TIMER::CTimerThread objTimerThread;

    TS_TIMER::CTime_Value ttbegin = TS_TIMER::GetTimeofDay() - TS_TIMER::CTime_Value(5, 0);

    int nID = 1001;

    objTimerThread.Init();

    objTimerThread.Add_Timer(1, 1000, NULL, Do_Timer_Event, (void*)&nID, Timeout_Error_Callback);

    objTimerThread.Run();

```  


## How to use Add_Timer function
nTimerID   : timing task ID    
nFrequency : interval, in milliseconds.
pttBegin   : start time of pttBegin timer (not executed for the first time, Can be set NULL, NULL is now). 
fn_Timeout_Callback : task callback event, not NULL.  
pArgContext : parameters for task execution at maturity.  
fn_Timeout_Error_Callback : timer does not perform the wrong report on time, it can be NULL.
