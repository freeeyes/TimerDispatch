#include <unistd.h>
#include "TimerThread.h"

void Do_Timer_Event(int nTimerID, ts_timer::CTime_Value& tvNow, void* pArg, ts_timer::EM_Timer_State& emState)
{
    int* pData = (int*)pArg;
    printf("[Do_Timer_Event](%d)<%s>, Arg=%d, emState=%d.\n", nTimerID, tvNow.Get_string().c_str(), *pData, emState);
	usleep(500*1000);
}

void Timeout_Error_Callback(int nErrorTimerID, int nTimerID, std::vector<ts_timer::CTime_Value> vecTimeout, void* pArg)
{
    int* pData = (int*)pArg;
    int nCount = (int)vecTimeout.size();
    printf("[Timeout_Error_Callback](nErrorTimerID=%d,nTimerID=%d)<%d>, Arg=%d.\n", nErrorTimerID, nTimerID, nCount, *pData);
}

int main()
{
    ts_timer::CTimerThread objTimerThread;

    ts_timer::CTime_Value ttbegin = ts_timer::GetTimeofDay() - ts_timer::CTime_Value(5, 0);

    int nID = 1001;

    objTimerThread.Init();

    objTimerThread.Add_Timer(1, 1000, &ttbegin, Do_Timer_Event, (void*)&nID, Timeout_Error_Callback);

    objTimerThread.Run();
	
    sleep(10);
	//getchar();
    return 0;
}

