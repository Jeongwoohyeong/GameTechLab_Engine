#include "Time.h"
#include <windows.h>

UTime* UTime::instance = nullptr;
UTime::UTime() : deltaTime(0.0), targetFrameTime(0.0) {}

UTime* UTime::GetInstance()
{
    if (instance == nullptr)
    {
        instance = new UTime();
    }
    return instance;
}

void UTime::Init()
{
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&prevTime);
}

void UTime::Update()
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    double actualDeltaTime = static_cast<double>(currentTime.QuadPart - prevTime.QuadPart) / frequency.QuadPart;
    targetFrameTime = 1.0 / targetFPS;

    if (actualDeltaTime < targetFrameTime)
    {
        double remainingTime = targetFrameTime - actualDeltaTime;

        Sleep(static_cast<DWORD>(remainingTime * 1000));

        QueryPerformanceCounter(&currentTime);
        deltaTime = static_cast<double>(currentTime.QuadPart - prevTime.QuadPart) / frequency.QuadPart;
    }
    else
    {
        deltaTime = actualDeltaTime;
    }

    if (deltaTime > 1 / 30.0f)
    {
        deltaTime = 1 / 30.0f;
    }

    prevTime = currentTime;
}
