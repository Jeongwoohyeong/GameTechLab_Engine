#include "pch.h"
#include "Manager/Camera/Public/CameraModifier.h"
#include "Manager/Camera/Public/PlayerCameraManager.h"

IMPLEMENT_CLASS(UCameraModifier, UObject)

UCameraModifier::UCameraModifier()
	: CameraOwner(nullptr)
	, AlphaInTime(0.3f)
	, AlphaOutTime(0.3f)
	, Alpha(0.0f)
	, TargetAlpha(0.0f)
	, bDisabled(true)
	, Priority(128)
{
}

UCameraModifier::~UCameraModifier() = default;

void UCameraModifier::Initialize(APlayerCameraManager* InCameraOwner)
{
	CameraOwner = InCameraOwner;
}

bool UCameraModifier::ModifyCamera(float DeltaTime, FVector& CameraLocation, FRotator& CameraRotation)
{
	// Base implementation does nothing
	// Derived classes should override this
	return false;
}

void UCameraModifier::UpdateModifier(float DeltaTime)
{
	if (bDisabled)
	{
		return;
	}

	// Blend alpha toward target
	if (Alpha != TargetAlpha)
	{
		float BlendSpeed;
		if (TargetAlpha > Alpha)
		{
			// Blending in
			BlendSpeed = (AlphaInTime > 0.0f) ? (1.0f / AlphaInTime) : 10.0f;
		}
		else
		{
			// Blending out
			BlendSpeed = (AlphaOutTime > 0.0f) ? (1.0f / AlphaOutTime) : 10.0f;
		}

		Alpha = Lerp(Alpha, TargetAlpha, BlendSpeed * DeltaTime);

		// Snap to target if very close
		if (fabs(Alpha - TargetAlpha) < 0.01f)
		{
			Alpha = TargetAlpha;

			// If we finished blending out, mark as disabled
			if (Alpha == 0.0f)
			{
				bDisabled = true;
			}
		}
	}
}

void UCameraModifier::EnableModifier()
{
	bDisabled = false;
	TargetAlpha = 1.0f;
}

void UCameraModifier::DisableModifier()
{
	TargetAlpha = 0.0f;
	// Don't set bDisabled yet - let it blend out first
}

void UCameraModifier::EnableModifierInstant()
{
	bDisabled = false;
	Alpha = 1.0f;
	TargetAlpha = 1.0f;
}

void UCameraModifier::DisableModifierInstant()
{
	bDisabled = true;
	Alpha = 0.0f;
	TargetAlpha = 0.0f;
}
