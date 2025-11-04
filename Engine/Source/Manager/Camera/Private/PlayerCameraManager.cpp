#include "pch.h"
#include "Manager/Camera/Public/PlayerCameraManager.h"
#include "Manager/Camera/Public/CameraModifier.h"
#include "Editor/Public/Camera.h"
#include "Pawn/Public/Pawn.h"

// Singleton instance
APlayerCameraManager& APlayerCameraManager::GetInstance()
{
	static APlayerCameraManager Instance;
	return Instance;
}

APlayerCameraManager::APlayerCameraManager()
	: Camera(nullptr)
	, FadeColorLinear(0, 0, 0, 1)
	, FadeAmount(0.0f)
	, FadeAlpha(0, 0)
	, FadeTime(0.0f)
	, FadeTimeRemaining(0.0f)
	, bFadeOut(true)
	, bHoldFade(false)
	, bIsLetterBoxActive(false)
	, LetterBoxHeight(0.1f)
	, LetterBoxTargetAlpha(0.0f)
	, LetterBoxCurrentAlpha(0.0f)
	, LetterBoxBlendSpeed(2.0f)
	, bSpringArmEnabled(true)
	, SpringArmOffset(-300.0f, 0.0f, 100.0f)
	, SpringArmLength(300.0f)
	, SpringArmInterpSpeed(10.0f)
	, bEnableCollisionTest(false)
	, CurrentViewType(ECameraViewType::ThirdPerson)
	, PreviousViewType(ECameraViewType::ThirdPerson)
	, ViewTransitionDuration(1.0f)
	, ViewTransitionTimeRemaining(0.0f)
	, bIsTransitioning(false)
{
}

APlayerCameraManager::~APlayerCameraManager()
{
	// Clean up modifiers
	for (UCameraModifier* Modifier : ModifierList)
	{
		if (Modifier)
		{
			delete Modifier;
		}
	}
	ModifierList.clear();
}

void APlayerCameraManager::Initialize(UCamera* InCamera)
{
	Camera = InCamera;
}

void APlayerCameraManager::Tick(float DeltaTime)
{
	if (!Camera)
	{
		return;
	}

	// Update all systems
	UpdateFade(DeltaTime);
	UpdateLetterBox(DeltaTime);
	UpdateViewTransition(DeltaTime);
	UpdateCameraShake(DeltaTime);
	UpdateSpringArm(DeltaTime);

	// Get camera location and rotation
	FVector CameraLocation = Camera->GetLocation();
	FRotator CameraRotation = Camera->GetRotationRotator();

	// Apply camera modifiers (shake, etc.)
	ApplyCameraModifiers(DeltaTime, CameraLocation, CameraRotation);

	// Update camera transform
	Camera->SetLocation(CameraLocation);
	Camera->SetRotation(CameraRotation);
}

void APlayerCameraManager::SetViewTarget(AActor* NewTarget)
{
	ViewTarget.Target = NewTarget;
}

// ========== Fade System ==========

void APlayerCameraManager::StartCameraFade(float Duration, FVector4 ToColor, bool bInFadeOut, bool bHoldWhenFinished)
{
	FadeTime = Duration;
	FadeTimeRemaining = Duration;
	FadeColorLinear = ToColor;
	bFadeOut = bInFadeOut;
	bHoldFade = bHoldWhenFinished;

	if (bInFadeOut)
	{
		// Fade to color: 0 -> 1
		FadeAmount = 0.0f;
	}
	else
	{
		// Fade from color: 1 -> 0
		FadeAmount = 1.0f;
	}
}

void APlayerCameraManager::StopCameraFade()
{
	FadeTimeRemaining = 0.0f;
	FadeAmount = 0.0f;
	bHoldFade = false;
}

void APlayerCameraManager::UpdateFade(float DeltaTime)
{
	if (FadeTimeRemaining > 0.0f)
	{
		FadeTimeRemaining -= DeltaTime;

		if (FadeTimeRemaining <= 0.0f)
		{
			// Fade finished
			FadeTimeRemaining = 0.0f;
			if (bFadeOut)
			{
				FadeAmount = 1.0f;
			}
			else
			{
				FadeAmount = 0.0f;
			}

			if (!bHoldFade)
			{
				FadeAmount = 0.0f;
			}
		}
		else
		{
			// Calculate fade alpha
			float FadeAlpha = 1.0f - (FadeTimeRemaining / FadeTime);

			if (bFadeOut)
			{
				// Fade to color: 0 -> 1
				FadeAmount = FadeAlpha;
			}
			else
			{
				// Fade from color: 1 -> 0
				FadeAmount = 1.0f - FadeAlpha;
			}
		}
	}
}

// ========== Letter Box System ==========

void APlayerCameraManager::StartLetterBox(float Height, float BlendTime)
{
	bIsLetterBoxActive = true;
	LetterBoxHeight = Height;
	LetterBoxTargetAlpha = 1.0f;
	LetterBoxBlendSpeed = (BlendTime > 0.0f) ? (1.0f / BlendTime) : 2.0f;
}

void APlayerCameraManager::StopLetterBox(float BlendTime)
{
	LetterBoxTargetAlpha = 0.0f;
	LetterBoxBlendSpeed = (BlendTime > 0.0f) ? (1.0f / BlendTime) : 2.0f;
}

void APlayerCameraManager::UpdateLetterBox(float DeltaTime)
{
	if (bIsLetterBoxActive || LetterBoxCurrentAlpha > 0.0f)
	{
		// Lerp toward target
		LetterBoxCurrentAlpha = Lerp(LetterBoxCurrentAlpha, LetterBoxTargetAlpha, LetterBoxBlendSpeed * DeltaTime);

		// Snap to target if very close
		if (fabs(LetterBoxCurrentAlpha - LetterBoxTargetAlpha) < 0.01f)
		{
			LetterBoxCurrentAlpha = LetterBoxTargetAlpha;

			// If blended out completely, mark as inactive
			if (LetterBoxCurrentAlpha == 0.0f)
			{
				bIsLetterBoxActive = false;
			}
		}
	}
}

// ========== Spring Arm System ==========

void APlayerCameraManager::SetSpringArmParams(FVector Offset, float ArmLength, float InterpSpeed)
{
	SpringArmOffset = Offset;
	SpringArmLength = ArmLength;
	SpringArmInterpSpeed = InterpSpeed;
}

void APlayerCameraManager::UpdateSpringArm(float DeltaTime)
{
	if (!bSpringArmEnabled || !Camera || !ViewTarget.Target)
	{
		return;
	}

	// Get target location
	FVector TargetLocation = ViewTarget.Target->GetActorLocation();

	// Calculate desired camera location
	FVector DesiredLocation = CalculateCameraLocationForView(CurrentViewType);

	// Lerp camera toward desired location
	FVector CurrentLocation = Camera->GetLocation();
	FVector NewLocation = Lerp(CurrentLocation, DesiredLocation, SpringArmInterpSpeed * DeltaTime);

	// TODO: Add collision test here if bEnableCollisionTest is true

	Camera->SetLocation(NewLocation);

	// Calculate camera rotation (look at target)
	FRotator DesiredRotation = CalculateCameraRotationForView(CurrentViewType);
	FRotator CurrentRotation = Camera->GetRotationRotator();
	FRotator NewRotation = Lerp(CurrentRotation, DesiredRotation, SpringArmInterpSpeed * DeltaTime);

	Camera->SetRotation(NewRotation);
}

FVector APlayerCameraManager::CalculateCameraLocationForView(ECameraViewType ViewType)
{
	if (!ViewTarget.Target)
	{
		return Camera->GetLocation();
	}

	FVector TargetLocation = ViewTarget.Target->GetActorLocation();

	switch (ViewType)
	{
	case ECameraViewType::ThirdPerson:
	{
		// Position camera behind and above the target
		FQuaternion TargetRotation = ViewTarget.Target->GetActorRotation();
		FMatrix RotMatrix = FMatrix::RotationMatrix(TargetRotation);

		// Extract Forward, Right, Up vectors from rotation matrix
		FVector ForwardVector(RotMatrix.Data[0][0], RotMatrix.Data[0][1], RotMatrix.Data[0][2]);
		FVector RightVector(RotMatrix.Data[1][0], RotMatrix.Data[1][1], RotMatrix.Data[1][2]);
		FVector UpVector(RotMatrix.Data[2][0], RotMatrix.Data[2][1], RotMatrix.Data[2][2]);

		return TargetLocation
			+ ForwardVector * SpringArmOffset.X
			+ RightVector * SpringArmOffset.Y
			+ UpVector * SpringArmOffset.Z;
	}

	case ECameraViewType::FirstPerson:
	{
		// Camera at target location (cockpit view)
		return TargetLocation + FVector(0, 0, 50.0f);  // Slightly above
	}

	case ECameraViewType::DeathCam:
	{
		// Camera further back and higher for death view
		FQuaternion TargetRotation = ViewTarget.Target->GetActorRotation();
		FMatrix RotMatrix = FMatrix::RotationMatrix(TargetRotation);
		FVector ForwardVector(RotMatrix.Data[0][0], RotMatrix.Data[0][1], RotMatrix.Data[0][2]);
		return TargetLocation + ForwardVector * -500.0f + FVector(0, 0, 200.0f);
	}

	case ECameraViewType::Cinematic:
	case ECameraViewType::FreeCam:
	default:
		// Keep current camera location
		return Camera->GetLocation();
	}
}

FRotator APlayerCameraManager::CalculateCameraRotationForView(ECameraViewType ViewType)
{
	if (!ViewTarget.Target)
	{
		return Camera->GetRotationRotator();
	}

	FVector CameraLocation = Camera->GetLocation();
	FVector TargetLocation = ViewTarget.Target->GetActorLocation();

	switch (ViewType)
	{
	case ECameraViewType::ThirdPerson:
	{
		// Look at target - convert direction to FRotator
		FVector Direction = (TargetLocation - CameraLocation).GetNormalized();

		// Calculate Yaw and Pitch from direction vector
		float Yaw = atan2f(Direction.Y, Direction.X) * (180.0f / 3.14159265359f);
		float Pitch = asinf(-Direction.Z) * (180.0f / 3.14159265359f);

		return FRotator(Pitch, Yaw, 0.0f);
	}

	case ECameraViewType::FirstPerson:
	{
		// Use target rotation (look where target is looking)
		FQuaternion TargetQuat = ViewTarget.Target->GetActorRotation();
		FVector Euler = TargetQuat.ToEuler();  // Returns (Roll, Pitch, Yaw)
		return FRotator(Euler.Y, Euler.Z, Euler.X);  // FRotator(Pitch, Yaw, Roll)
	}

	case ECameraViewType::DeathCam:
	{
		// Look at target from death cam position
		FVector Direction = (TargetLocation - CameraLocation).GetNormalized();

		float Yaw = atan2f(Direction.Y, Direction.X) * (180.0f / 3.14159265359f);
		float Pitch = asinf(-Direction.Z) * (180.0f / 3.14159265359f);

		return FRotator(Pitch, Yaw, 0.0f);
	}

	case ECameraViewType::Cinematic:
	case ECameraViewType::FreeCam:
	default:
		// Keep current camera rotation
		return Camera->GetRotationRotator();
	}
}

// ========== Camera View Type ==========

void APlayerCameraManager::TransitionToView(ECameraViewType NewView, float Duration)
{
	if (CurrentViewType == NewView)
	{
		return;
	}

	PreviousViewType = CurrentViewType;
	CurrentViewType = NewView;
	ViewTransitionDuration = Duration;
	ViewTransitionTimeRemaining = Duration;
	bIsTransitioning = true;
}

void APlayerCameraManager::UpdateViewTransition(float DeltaTime)
{
	if (!bIsTransitioning)
	{
		return;
	}

	ViewTransitionTimeRemaining -= DeltaTime;

	if (ViewTransitionTimeRemaining <= 0.0f)
	{
		ViewTransitionTimeRemaining = 0.0f;
		bIsTransitioning = false;
	}

	// TODO: Implement smooth transition between view types
	// For now, just snap to new view type
}

// ========== Camera Modifier System ==========

void APlayerCameraManager::AddCameraModifier(UCameraModifier* Modifier)
{
	if (!Modifier)
	{
		return;
	}

	Modifier->Initialize(this);
	ModifierList.push_back(Modifier);

	// Sort by priority (lower priority = applied first)
	std::sort(ModifierList.begin(), ModifierList.end(),
		[](UCameraModifier* A, UCameraModifier* B)
		{
			return A->GetPriority() < B->GetPriority();
		});
}

void APlayerCameraManager::RemoveCameraModifier(UCameraModifier* Modifier)
{
	auto it = std::find(ModifierList.begin(), ModifierList.end(), Modifier);
	if (it != ModifierList.end())
	{
		ModifierList.erase(it);
	}
}

void APlayerCameraManager::ClearAllModifiers()
{
	for (UCameraModifier* Modifier : ModifierList)
	{
		if (Modifier)
		{
			delete Modifier;
		}
	}
	ModifierList.clear();
}

void APlayerCameraManager::ApplyCameraModifiers(float DeltaTime, FVector& CameraLocation, FRotator& CameraRotation)
{
	// Apply camera shake
	if (bIsCameraShaking)
	{
		CameraLocation += ShakeOffset;
	}

	// Apply camera modifiers
	for (UCameraModifier* Modifier : ModifierList)
	{
		if (Modifier && !Modifier->IsDisabled())
		{
			Modifier->UpdateModifier(DeltaTime);
			Modifier->ModifyCamera(DeltaTime, CameraLocation, CameraRotation);
		}
	}
}

// ========== Camera Shake ==========

void APlayerCameraManager::StartCameraShake(float Intensity, float Duration)
{
	bIsCameraShaking = true;
	CameraShakeTimer = 0.0f;
	CameraShakeDuration = Duration;
	CameraShakeIntensity = Intensity;
}

void APlayerCameraManager::UpdateCameraShake(float DeltaTime)
{
	if (!bIsCameraShaking)
	{
		ShakeOffset = FVector::Zero();
		return;
	}

	CameraShakeTimer += DeltaTime;

	if (CameraShakeTimer >= CameraShakeDuration)
	{
		// Shake finished
		bIsCameraShaking = false;
		CameraShakeTimer = 0.0f;
		ShakeOffset = FVector::Zero();
		return;
	}

	// Calculate shake intensity with decay (gets weaker over time)
	float ShakeAlpha = 1.0f - (CameraShakeTimer / CameraShakeDuration);
	float CurrentIntensity = CameraShakeIntensity * ShakeAlpha;

	// Generate random offset
	ShakeOffset.X = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * CurrentIntensity;
	ShakeOffset.Y = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * CurrentIntensity;
	ShakeOffset.Z = ((float)rand() / RAND_MAX - 0.5f) * 2.0f * CurrentIntensity;
}
