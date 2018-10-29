// WindowsTimer_Test.cpp : 定义控制台应用程序的入口点。
//

#include "TimerThread.h"
#include "TimerCommon.h"

void Do_Timer_Event(int nTimerID, ts_timer::CTime_Value& tvNow, void* pArg, ts_timer::EM_Timer_State& emState)
{
    int* pData = (int*)pArg;
    PIRNTF("[Do_Timer_Event](%d)<%s>, Arg=%d.\n", nTimerID, tvNow.Get_string().c_str(), *pData);
    Sleep(500);
}

void Timeout_Error_Callback(int nErrorTimerID, int nTimeout, int nTimerID, std::vector<ts_timer::CTime_Value> vecTimeout, void* pArg)
{
    int* pData = (int*)pArg;
    int nCount = (int)vecTimeout.size();
    PIRNTF("[Timeout_Error_Callback](nErrorTimerID=%d,nTimeout=%d)nTimerID=%d<%d>, Arg=%d.\n", nErrorTimerID, nTimeout, nTimerID, nCount, *pData);
}

int main()
{
    ts_timer::CTimerThread objTimerThread;

    ts_timer::CTime_Value ttbegin = ts_timer::GetTimeofDay() - ts_timer::CTime_Value(5, 0);

    int nID = 1001;

    objTimerThread.Init();

    objTimerThread.Run();

    Sleep(5000);

    //objTimerThread.Pause();
    //objTimerThread.Add_Timer(1, 1000, NULL, Do_Timer_Event, (void*)&nID, Timeout_Error_Callback);

    //Sleep(5000);

    //objTimerThread.Restore();

    //getchar();
    return 0;
}

