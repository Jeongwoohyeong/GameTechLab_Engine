#pragma once
#include "Core/Public/Object.h"

using std::chrono::high_resolution_clock;

UCLASS()
class UTimeManager :
	public UObject
{
	GENERATED_BODY()
	DECLARE_SINGLETON_CLASS(UTimeManager, UObject)

public:
	void Update();

	// Getter & Setter
	float GetFPS() const { return 1 / DeltaTime; }
	float GetDeltaTime() const { return DeltaTime * GlobalTimeDilation; }
	float GetRawDeltaTime() const { return DeltaTime; }
	float GetGameTime() const { return GameTime; }
	bool IsPaused() const { return bIsPaused; }

	void SetDeltaTime(float InDeltaTime) { DeltaTime = InDeltaTime; }

	void PauseGame() { bIsPaused = true; }
	void ResumeGame() { bIsPaused = false; }

	// ========== Time Dilation System ==========

	/**
	 * @brief Set global time dilation (1.0 = normal speed, 0.5 = half speed, 2.0 = double speed)
	 * @param Dilation The time scale factor
	 */
	void SetGlobalTimeDilation(float Dilation) { GlobalTimeDilation = Clamp(Dilation, 0.0f, 10.0f); }

	/**
	 * @brief Get current global time dilation
	 */
	float GetGlobalTimeDilation() const { return GlobalTimeDilation; }

	/**
	 * @brief Reset time dilation to normal speed
	 */
	void ResetTimeDilation() { GlobalTimeDilation = 1.0f; }

	// ========== Hit Stop System ==========

	/**
	 * @brief Start a hit stop (brief time freeze for impact)
	 * @param Duration Duration of hit stop in seconds (real time)
	 */
	void StartHitStop(float Duration = 0.1f);

	/**
	 * @brief Check if hit stop is active
	 */
	bool IsHitStopActive() const { return HitStopTimeRemaining > 0.0f; }

	// ========== Slow Motion System ==========

	/**
	 * @brief Start slow motion effect
	 * @param Speed Slow motion speed (0.0 - 1.0, e.g., 0.3 = 30% speed)
	 * @param Duration Duration in real-time seconds
	 */
	void StartSlowMotion(float Speed, float Duration);

	/**
	 * @brief Stop slow motion and return to normal speed
	 */
	void StopSlowMotion();

	/**
	 * @brief Check if slow motion is active
	 */
	bool IsSlowMotionActive() const { return SlowMotionTimeRemaining > 0.0f; }

private:
	float GameTime;
	float DeltaTime;

	bool bIsPaused;

	// ========== Time Dilation ==========
	float GlobalTimeDilation = 1.0f;

	// ========== Hit Stop ==========
	float HitStopTimeRemaining = 0.0f;

	// ========== Slow Motion ==========
	float SlowMotionSpeed = 1.0f;
	float SlowMotionTimeRemaining = 0.0f;
	float PreviousTimeDilation = 1.0f;  // Store previous dilation before slomo

	void Initialize();
	void CalculateFPS();
	void UpdateTimeDilation();
};
