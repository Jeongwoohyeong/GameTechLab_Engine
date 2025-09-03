#pragma once
#include "windows.h"
class UTime {
private:
    UTime();
    ~UTime() = default;

public:
    static UTime* GetInstance();

    void Init();
    void Update();

    double GetDeltaTime() const { return deltaTime * timeScale; }
    double GetRealDeltaTime() const { return deltaTime; }

    void SetTimeScale(float inTimeScale) { timeScale = inTimeScale; }
    float GetTimeScale() const { return timeScale; }

    void SetTargetFPS(int fps) { targetFPS = fps; }
    int GetTargetFPS() const { return targetFPS; }

private:
    static UTime* instance;

    float timeScale = 1.0f;

    LARGE_INTEGER frequency;
    LARGE_INTEGER prevTime;
    double deltaTime;
    double targetFrameTime;

    int targetFPS = 60;
};