#include "pch.h"
#include "Manager/Camera/Public/PlayerCameraManager.h"
#include "Manager/Camera/Public/CameraModifier.h"
#include "Manager/Camera/Public/CameraModifier_CameraShake.h"
#include "Editor/Public/Camera.h"
#include "Component/Camera/Public/CameraComponent.h"
#include "Pawn/Public/Pawn.h"
#include "Level/Public/World.h"
#include "Level/Public/Level.h"
#include "Component/Collision/Public/ShapeComponent.h"
#include "Component/Collision/Public/BoxComponent.h"
#include "Component/Collision/Public/SphereComponent.h"
#include "Component/Collision/Public/CapsuleComponent.h"
#include "Physics/Public/OBB.h"
#include "Physics/Public/CollisionUtil.h"

IMPLEMENT_CLASS(APlayerCameraManager, AActor)

// Static 변수 정의
float APlayerCameraManager::StaticShakeBezierCP[4] = { 0.250f, 0.460f, 0.450f, 0.940f };
bool APlayerCameraManager::StaticUseBezierDecay = true;
bool APlayerCameraManager::bStaticValuesInitialized = false;

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
	// ✅ Tick 활성화 - 카메라 modifier 업데이트를 위해 필수!
	bCanEverTick = true;
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

void APlayerCameraManager::BeginPlay()
{
	Super::BeginPlay();

	// Static 값이 초기화되어 있으면 그걸 사용 (에디터→PIE 전달)
	if (bStaticValuesInitialized)
	{
		DefaultShakeBezierCP[0] = StaticShakeBezierCP[0];
		DefaultShakeBezierCP[1] = StaticShakeBezierCP[1];
		DefaultShakeBezierCP[2] = StaticShakeBezierCP[2];
		DefaultShakeBezierCP[3] = StaticShakeBezierCP[3];
		bDefaultUseBezierDecay = StaticUseBezierDecay;
	}
	else
	{
		// 처음 실행 시 현재 값을 static에 저장
		StaticShakeBezierCP[0] = DefaultShakeBezierCP[0];
		StaticShakeBezierCP[1] = DefaultShakeBezierCP[1];
		StaticShakeBezierCP[2] = DefaultShakeBezierCP[2];
		StaticShakeBezierCP[3] = DefaultShakeBezierCP[3];
		StaticUseBezierDecay = bDefaultUseBezierDecay;
		bStaticValuesInitialized = true;
	}

	// Add default modifiers if none exist
	if (ModifierList.empty())
	{
		UCameraModifier_CameraShake* ShakeMod = new UCameraModifier_CameraShake();

		// Apply default Bezier settings from this manager
		ShakeMod->BezierCP[0] = DefaultShakeBezierCP[0];
		ShakeMod->BezierCP[1] = DefaultShakeBezierCP[1];
		ShakeMod->BezierCP[2] = DefaultShakeBezierCP[2];
		ShakeMod->BezierCP[3] = DefaultShakeBezierCP[3];
		ShakeMod->bUseBezierDecay = bDefaultUseBezierDecay;

		AddCameraModifier(ShakeMod);
	}
}

void APlayerCameraManager::Initialize(UCamera* InCamera)
{
	Camera = InCamera;
	CameraComponent = nullptr;
}

void APlayerCameraManager::Initialize(UCameraComponent* InCameraComponent)
{
	CameraComponent = InCameraComponent;
	Camera = nullptr;
}

void APlayerCameraManager::Tick(float DeltaTime)
{
	if (!Camera && !CameraComponent)
	{
		return;
	}

	// Update all systems
	UpdateFade(DeltaTime);
	UpdateLetterBox(DeltaTime);
	UpdateViewTransition(DeltaTime);
	UpdateSpringArm(DeltaTime);

	// PIE mode: Apply camera modifiers to CameraComponent
	if (CameraComponent)
	{
		// Get initial camera transform
		FVector CameraLocation = CameraComponent->GetWorldLocation();
		FRotator CameraRotation = FRotator(0, 0, 0);  // TODO: Get actual rotation if needed

		// Apply all camera modifiers (including shake)
		ApplyCameraModifiers(DeltaTime, CameraLocation, CameraRotation);

		// Calculate offset from original location
		FVector ShakeOffset = CameraLocation - CameraComponent->GetWorldLocation();

		// Apply the offset to CameraComponent
		CameraComponent->SetCameraShakeOffset(ShakeOffset);
	}
	// Editor mode: Apply to UCamera
	else if (Camera)
	{
		// Get camera location and rotation
		FVector CameraLocation = Camera->GetLocation();
		FRotator CameraRotation = Camera->GetRotationRotator();

		// Apply camera modifiers (shake, etc.)
		ApplyCameraModifiers(DeltaTime, CameraLocation, CameraRotation);

		// Update camera transform
		Camera->SetLocation(CameraLocation);
		Camera->SetRotation(CameraRotation);
	}
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

	// Spring Arm Collision Test
	if (bEnableCollisionTest)
	{
		// Get World and Level
		UWorld* World = GWorld;
		if (World && World->GetLevel())
		{
			const TArray<UShapeComponent*>& ShapeComponents = World->GetLevel()->GetShapeComponents();

			// LineTrace from Target to Desired Camera Location
			FVector LineStart = TargetLocation;
			FVector LineEnd = DesiredLocation;
			FVector TraceDirection = LineEnd - LineStart;
			float DesiredDistance = TraceDirection.Length();

			if (DesiredDistance < 0.01f)
			{
				// Camera too close to target, skip collision test
				Camera->SetLocation(NewLocation);
				return;
			}

			float MinHitDistance = DesiredDistance;
			bool bHitDetected = false;

			// Test collision with all shape components
			for (const UShapeComponent* ShapeComp : ShapeComponents)
			{
				if (!ShapeComp)
				{
					continue;
				}

				// Skip if this is the target's own collision component
				if (ShapeComp->GetOwner() == ViewTarget.Target)
				{
					continue;
				}

				float Distance = FLT_MAX;

				// Box collision
				if (const UBoxComponent* BoxComp = Cast<UBoxComponent>(const_cast<UShapeComponent*>(ShapeComp)))
				{
					FOBB OBB(BoxComp->GetWorldLocation(), BoxComp->GetBoxExtent(), BoxComp->GetWorldTransformMatrix());
					Distance = CollisionUtil::DistanceSegmentToOBB(LineStart, LineEnd, OBB);
				}
				// Sphere collision
				else if (const USphereComponent* SphereComp = Cast<USphereComponent>(const_cast<UShapeComponent*>(ShapeComp)))
				{
					FVector SphereCenter = SphereComp->GetWorldLocation();
					float SphereRadius = SphereComp->GetSphereRadius();
					FVector Scale = SphereComp->GetWorldScale3D();
					float MaxScale = max(max(Scale.X, Scale.Y), Scale.Z);
					SphereRadius *= MaxScale;

					// Distance from line segment to sphere
					FVector ClosestPointOnLine;
					FVector ToStart = SphereCenter - LineStart;
					float T = Clamp(ToStart.Dot(TraceDirection) / (DesiredDistance * DesiredDistance), 0.0f, 1.0f);
					ClosestPointOnLine = LineStart + T * TraceDirection;
					Distance = FVector::Dist(ClosestPointOnLine, SphereCenter) - SphereRadius;
				}
				// Capsule collision (simplified as two spheres + cylinder)
				else if (const UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(const_cast<UShapeComponent*>(ShapeComp)))
				{
					FVector CapsuleCenter = CapsuleComp->GetWorldLocation();
					float CapsuleRadius = CapsuleComp->GetCapsuleRadius();
					float CapsuleHalfHeight = CapsuleComp->GetCapsuleHalfHeight();
					FVector Scale = CapsuleComp->GetWorldScale3D();
					float RadiusScale = max(Scale.X, Scale.Y);
					CapsuleRadius *= RadiusScale;
					CapsuleHalfHeight *= Scale.Z;

					FVector CapsuleUp = CapsuleComp->GetUpVector();
					FVector CapsuleStart = CapsuleCenter - CapsuleUp * CapsuleHalfHeight;
					FVector CapsuleEnd = CapsuleCenter + CapsuleUp * CapsuleHalfHeight;

					FVector ClosestOnCamera, ClosestOnCapsule;
					CollisionUtil::ClosestPointsBetweenSegments(
						LineStart, LineEnd,
						CapsuleStart, CapsuleEnd,
						ClosestOnCamera, ClosestOnCapsule
					);
					Distance = FVector::Dist(ClosestOnCamera, ClosestOnCapsule) - CapsuleRadius;
				}

				// Check if this is the closest hit
				if (Distance < 1.0f && Distance < MinHitDistance)  // 1.0f threshold for collision
				{
					MinHitDistance = Distance;
					bHitDetected = true;
				}
			}

			// If collision detected, pull camera closer
			if (bHitDetected)
			{
				// Calculate safe camera position
				float SafeDistance = max(MinHitDistance, 10.0f);  // Minimum 10 units from collision
				float T = Clamp(SafeDistance / DesiredDistance, 0.0f, 1.0f);
				FVector SafeLocation = LineStart + TraceDirection * T;

				// Blend to safe location
				NewLocation = Lerp(CurrentLocation, SafeLocation, SpringArmInterpSpeed * DeltaTime * 2.0f);  // Faster pull on collision
			}
		}
	}

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
	// Apply all camera modifiers (shake, etc.)
	for (size_t i = 0; i < ModifierList.size(); i++)
	{
		UCameraModifier* Modifier = ModifierList[i];
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
	// Find the Camera Shake Modifier
	UCameraModifier_CameraShake* ShakeMod = FindModifier<UCameraModifier_CameraShake>();
	if (ShakeMod)
	{
		ShakeMod->StartShake(Intensity, Duration);
	}
}
