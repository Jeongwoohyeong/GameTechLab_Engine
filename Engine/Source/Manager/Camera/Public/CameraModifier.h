#pragma once
#include "Core/Public/Object.h"

class APlayerCameraManager;

/**
 * @brief UCameraModifier - Base class for camera modifiers
 * Camera modifiers allow you to stack camera effects (shake, zoom, etc.)
 * Each modifier can be enabled/disabled with priority-based ordering
 */
UCLASS()
class UCameraModifier : public UObject
{
	GENERATED_BODY()
	DECLARE_CLASS(UCameraModifier, UObject)

public:
	UCameraModifier();
	virtual ~UCameraModifier();

	/**
	 * @brief Initialize the modifier with a camera owner
	 * @param InCameraOwner The PlayerCameraManager that owns this modifier
	 */
	virtual void Initialize(APlayerCameraManager* InCameraOwner);

	/**
	 * @brief Called every frame to modify the camera
	 * @param DeltaTime Time since last frame
	 * @param CameraLocation Camera location to modify
	 * @param CameraRotation Camera rotation to modify
	 * @return true if modifier is active and modifying the camera
	 */
	virtual bool ModifyCamera(float DeltaTime, FVector& CameraLocation, FRotator& CameraRotation);

	/**
	 * @brief Update modifier state (alpha blending, timers, etc.)
	 * @param DeltaTime Time since last frame
	 */
	virtual void UpdateModifier(float DeltaTime);

	/**
	 * @brief Enable this modifier (starts alpha blend in)
	 */
	virtual void EnableModifier();

	/**
	 * @brief Disable this modifier (starts alpha blend out)
	 */
	virtual void DisableModifier();

	/**
	 * @brief Instantly enable this modifier
	 */
	virtual void EnableModifierInstant();

	/**
	 * @brief Instantly disable this modifier
	 */
	virtual void DisableModifierInstant();

	// Getters
	bool IsDisabled() const { return bDisabled; }
	float GetAlpha() const { return Alpha; }
	uint8 GetPriority() const { return Priority; }
	APlayerCameraManager* GetCameraOwner() const { return CameraOwner; }

	// Setters
	void SetPriority(uint8 InPriority) { Priority = InPriority; }
	void SetAlphaInTime(float InTime) { AlphaInTime = InTime; }
	void SetAlphaOutTime(float InTime) { AlphaOutTime = InTime; }

protected:
	/** The camera owner that this modifier belongs to */
	APlayerCameraManager* CameraOwner = nullptr;

	/** Time it takes for modifier to blend in */
	float AlphaInTime = 0.3f;

	/** Time it takes for modifier to blend out */
	float AlphaOutTime = 0.3f;

	/** Current alpha value (0 = no effect, 1 = full effect) */
	float Alpha = 0.0f;

	/** Target alpha (for blending) */
	float TargetAlpha = 0.0f;

	/** Whether this modifier is currently disabled */
	bool bDisabled = true;

	/** Priority for modifier ordering (higher = applied later) */
	uint8 Priority = 128;
};
