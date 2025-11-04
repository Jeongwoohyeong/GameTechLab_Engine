#include "pch.h"
#include "Manager/Time/Public/TimeManager.h"
#include "Utility/Public/ScopeCycleCounter.h"

IMPLEMENT_SINGLETON_CLASS(UTimeManager, UObject)

UTimeManager::UTimeManager()
{
	Initialize();
}

UTimeManager::~UTimeManager() = default;

void UTimeManager::Initialize()
{
	GameTime = 0.0f;
	DeltaTime = 0.0f;
	bIsPaused = false;
	GlobalTimeDilation = 1.0f;
	HitStopTimeRemaining = 0.0f;
	SlowMotionSpeed = 1.0f;
	SlowMotionTimeRemaining = 0.0f;
	PreviousTimeDilation = 1.0f;
}

void UTimeManager::Update()
{
	// Update time dilation effects (hit stop, slomo)
	UpdateTimeDilation();

	if (!bIsPaused)
	{
		// Apply time dilation to game time
		GameTime += DeltaTime * GlobalTimeDilation;
	}
}

void UTimeManager::UpdateTimeDilation()
{
	// Use raw delta time (unaffected by time dilation) for timers
	float RawDelta = DeltaTime;

	// Update hit stop (has priority over slomo)
	if (HitStopTimeRemaining > 0.0f)
	{
		HitStopTimeRemaining -= RawDelta;
		if (HitStopTimeRemaining <= 0.0f)
		{
			HitStopTimeRemaining = 0.0f;
			// Restore time dilation (could be slomo or normal)
			if (SlowMotionTimeRemaining > 0.0f)
			{
				GlobalTimeDilation = SlowMotionSpeed;
			}
			else
			{
				GlobalTimeDilation = 1.0f;
			}
		}
		else
		{
			// Hit stop active - freeze time
			GlobalTimeDilation = 0.0f;
		}
		return;
	}

	// Update slow motion
	if (SlowMotionTimeRemaining > 0.0f)
	{
		SlowMotionTimeRemaining -= RawDelta;
		if (SlowMotionTimeRemaining <= 0.0f)
		{
			SlowMotionTimeRemaining = 0.0f;
			// Restore normal time dilation
			GlobalTimeDilation = PreviousTimeDilation;
		}
		else
		{
			// Slow motion active
			GlobalTimeDilation = SlowMotionSpeed;
		}
	}
}

// ========== Hit Stop System ==========

void UTimeManager::StartHitStop(float Duration)
{
	HitStopTimeRemaining = Duration;
	// Time dilation will be set to 0 in UpdateTimeDilation
}

// ========== Slow Motion System ==========

void UTimeManager::StartSlowMotion(float Speed, float Duration)
{
	// Store current dilation (in case we're not at 1.0)
	if (SlowMotionTimeRemaining <= 0.0f)
	{
		PreviousTimeDilation = GlobalTimeDilation;
	}

	SlowMotionSpeed = Clamp(Speed, 0.0f, 1.0f);
	SlowMotionTimeRemaining = Duration;
	GlobalTimeDilation = SlowMotionSpeed;
}

void UTimeManager::StopSlowMotion()
{
	SlowMotionTimeRemaining = 0.0f;
	GlobalTimeDilation = PreviousTimeDilation;
}
