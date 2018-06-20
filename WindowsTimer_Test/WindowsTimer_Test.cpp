// WindowsTimer_Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "TimerThread.h"

class CTimer_Test : public ITimerInfo
{
public:
    virtual ~CTimer_Test() {};

    virtual int Do_Timer_Event()
    {
        printf("[Do_Timer_Event]TimerID=%d, Frequency=%d.\n", Get_Timer_ID(), Get_Timer_Frequency());

        return 0;
    }
};

int main()
{
    CTimerThread objTimerThread;

    CTimer_Test test1;

    test1.Set_Timer_Param(1, 10000);

    objTimerThread.Init();

    objTimerThread.Add_Timer(reinterpret_cast<ITimerInfo*>(&test1));

    objTimerThread.Run();

    Sleep(1000);
    objTimerThread.Close();

    getchar();
    return 0;
}

