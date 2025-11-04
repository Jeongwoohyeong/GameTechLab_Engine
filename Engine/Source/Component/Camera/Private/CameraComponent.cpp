#include "pch.h"
#include "Component/Camera/Public/CameraComponent.h"
#include "Utility/Public/JsonSerializer.h"

IMPLEMENT_CLASS(UCameraComponent, USceneComponent)

UCameraComponent::UCameraComponent()
	: FieldOfView(90.0f)
	, AspectRatio(16.0f / 9.0f)
	, NearClipPlane(1.0f)
	, FarClipPlane(10000.0f)
	, ProjectionMode(ECameraProjectionMode::Perspective)
	, OrthoWidth(512.0f)
	, OrthoNearClipPlane(-1000.0f)
	, OrthoFarClipPlane(1000.0f)
	, bViewMatrixDirty(true)
	, bProjectionMatrixDirty(true)
{
	bCanEverTick = true;
}

UCameraComponent::~UCameraComponent() = default;

void UCameraComponent::BeginPlay()
{
	USceneComponent::BeginPlay();
	MarkMatricesDirty();
}

void UCameraComponent::TickComponent(float DeltaTime)
{
	USceneComponent::TickComponent(DeltaTime);

	// Mark view matrix dirty if transform changed
	// (SceneComponent will handle transform updates)
}

void UCameraComponent::Serialize(const bool bInIsLoading, JSON& InOutHandle)
{
	USceneComponent::Serialize(bInIsLoading, InOutHandle);

	// 불러오기
	if (bInIsLoading)
	{
		// Load camera properties
		FJsonSerializer::ReadFloat(InOutHandle, "FieldOfView", FieldOfView, 90.0f);
		FJsonSerializer::ReadFloat(InOutHandle, "AspectRatio", AspectRatio, 16.0f / 9.0f);
		FJsonSerializer::ReadFloat(InOutHandle, "NearClipPlane", NearClipPlane, 1.0f);
		FJsonSerializer::ReadFloat(InOutHandle, "FarClipPlane", FarClipPlane, 10000.0f);

		FString ProjectionModeString;
		FJsonSerializer::ReadString(InOutHandle, "ProjectionMode", ProjectionModeString, "0");
		ProjectionMode = static_cast<ECameraProjectionMode>(std::stoi(ProjectionModeString));

		FJsonSerializer::ReadFloat(InOutHandle, "OrthoWidth", OrthoWidth, 512.0f);
		FJsonSerializer::ReadFloat(InOutHandle, "OrthoNearClipPlane", OrthoNearClipPlane, -1000.0f);
		FJsonSerializer::ReadFloat(InOutHandle, "OrthoFarClipPlane", OrthoFarClipPlane, 1000.0f);

		MarkMatricesDirty();
	}
	// 저장
	else
	{
		// Save camera properties
		InOutHandle["FieldOfView"] = FieldOfView;
		InOutHandle["AspectRatio"] = AspectRatio;
		InOutHandle["NearClipPlane"] = NearClipPlane;
		InOutHandle["FarClipPlane"] = FarClipPlane;
		InOutHandle["ProjectionMode"] = static_cast<int>(ProjectionMode);
		InOutHandle["OrthoWidth"] = OrthoWidth;
		InOutHandle["OrthoNearClipPlane"] = OrthoNearClipPlane;
		InOutHandle["OrthoFarClipPlane"] = OrthoFarClipPlane;
	}
}

// ========== Camera Properties ==========

void UCameraComponent::SetFieldOfView(float InFOV)
{
	FieldOfView = InFOV;
	bProjectionMatrixDirty = true;
}

void UCameraComponent::SetAspectRatio(float InAspect)
{
	AspectRatio = InAspect;
	bProjectionMatrixDirty = true;
}

void UCameraComponent::SetNearClipPlane(float InNearPlane)
{
	NearClipPlane = InNearPlane;
	bProjectionMatrixDirty = true;
}

void UCameraComponent::SetFarClipPlane(float InFarPlane)
{
	FarClipPlane = InFarPlane;
	bProjectionMatrixDirty = true;
}

void UCameraComponent::SetProjectionMode(ECameraProjectionMode InMode)
{
	ProjectionMode = InMode;
	bProjectionMatrixDirty = true;
}

void UCameraComponent::SetOrthoWidth(float InWidth)
{
	OrthoWidth = InWidth;
	bProjectionMatrixDirty = true;
}

void UCameraComponent::SetOrthoNearClipPlane(float InNearPlane)
{
	OrthoNearClipPlane = InNearPlane;
	bProjectionMatrixDirty = true;
}

void UCameraComponent::SetOrthoFarClipPlane(float InFarPlane)
{
	OrthoFarClipPlane = InFarPlane;
	bProjectionMatrixDirty = true;
}

// ========== Matrix Calculations ==========

FMatrix UCameraComponent::GetViewMatrix() const
{
	if (bViewMatrixDirty)
	{
		UpdateViewMatrix();
	}
	return ViewMatrix;
}

FMatrix UCameraComponent::GetProjectionMatrix() const
{
	if (bProjectionMatrixDirty)
	{
		UpdateProjectionMatrix();
	}
	return ProjectionMatrix;
}

FMatrix UCameraComponent::GetViewProjectionMatrix() const
{
	return GetViewMatrix() * GetProjectionMatrix();
}

FCameraConstants UCameraComponent::GetCameraConstants() const
{
	FCameraConstants Constants;

	// Get matrices
	Constants.View = GetViewMatrix();
	Constants.Projection = GetProjectionMatrix();

	// Camera world position
	Constants.ViewWorldLocation = GetWorldLocation();

	// Near/Far clip planes
	Constants.NearClip = NearClipPlane;
	Constants.FarClip = FarClipPlane;

	return Constants;
}

void UCameraComponent::UpdateViewMatrix() const
{
	// Get camera world transform
	FVector CameraPos = GetWorldLocation();
	FVector Forward = GetForwardVector();
	FVector Up = GetUpVector();
	FVector Right = GetRightVector();

	// Build view matrix (world to camera space)
	// This is the inverse of the camera's world transform
	ViewMatrix = FMatrix::Identity();

	// Right vector
	ViewMatrix.Data[0][0] = Right.X;
	ViewMatrix.Data[0][1] = Right.Y;
	ViewMatrix.Data[0][2] = Right.Z;
	ViewMatrix.Data[0][3] = -CameraPos.Dot(Right);

	// Up vector
	ViewMatrix.Data[1][0] = Up.X;
	ViewMatrix.Data[1][1] = Up.Y;
	ViewMatrix.Data[1][2] = Up.Z;
	ViewMatrix.Data[1][3] = -CameraPos.Dot(Up);

	// Forward vector
	ViewMatrix.Data[2][0] = Forward.X;
	ViewMatrix.Data[2][1] = Forward.Y;
	ViewMatrix.Data[2][2] = Forward.Z;
	ViewMatrix.Data[2][3] = -CameraPos.Dot(Forward);

	// Last row
	ViewMatrix.Data[3][0] = 0.0f;
	ViewMatrix.Data[3][1] = 0.0f;
	ViewMatrix.Data[3][2] = 0.0f;
	ViewMatrix.Data[3][3] = 1.0f;

	bViewMatrixDirty = false;
}

void UCameraComponent::UpdateProjectionMatrix() const
{
	if (ProjectionMode == ECameraProjectionMode::Perspective)
	{
		// Use project's CreatePerspectiveFovLH function
		float FovYRadians = FieldOfView * (3.14159265359f / 180.0f);
		ProjectionMatrix = FMatrix::CreatePerspectiveFovLH(FovYRadians, AspectRatio, NearClipPlane, FarClipPlane);
	}
	else
	{
		// Orthographic projection matrix
		float OrthoHeight = OrthoWidth / AspectRatio;
		float Left = -OrthoWidth * 0.5f;
		float Right = OrthoWidth * 0.5f;
		float Bottom = -OrthoHeight * 0.5f;
		float Top = OrthoHeight * 0.5f;

		ProjectionMatrix = FMatrix::CreateOrthoOffCenterLH(Left, Right, Bottom, Top, OrthoNearClipPlane, OrthoFarClipPlane);
	}

	bProjectionMatrixDirty = false;
}

FRay UCameraComponent::ScreenPointToWorldRay(float ScreenX, float ScreenY) const
{
	// Get inverse matrices
	FMatrix InvView = GetViewMatrix().Inverse();
	FMatrix InvProj = GetProjectionMatrix().Inverse();

	// Convert NDC to clip space
	FVector4 ClipSpacePos(ScreenX, ScreenY, 1.0f, 1.0f);

	// Clip space to view space
	FVector4 ViewSpacePos = InvProj.TransformVector4(ClipSpacePos);
	if (ViewSpacePos.W != 0.0f)
	{
		ViewSpacePos.X /= ViewSpacePos.W;
		ViewSpacePos.Y /= ViewSpacePos.W;
		ViewSpacePos.Z /= ViewSpacePos.W;
		ViewSpacePos.W = 1.0f;
	}

	// View space to world space
	FVector4 WorldSpacePos = InvView.TransformVector4(ViewSpacePos);

	// Create ray (FRay uses FVector4 for Origin and Direction)
	FRay Ray;
	FVector CamPos = GetWorldLocation();
	Ray.Origin = FVector4(CamPos.X, CamPos.Y, CamPos.Z, 1.0f);

	FVector Dir = FVector(WorldSpacePos.X, WorldSpacePos.Y, WorldSpacePos.Z).GetNormalized();
	Ray.Direction = FVector4(Dir.X, Dir.Y, Dir.Z, 0.0f);

	return Ray;
}

// ========== Helper Functions ==========

FVector UCameraComponent::GetForwardVector() const
{
	// Get forward vector from world rotation
	FQuaternion WorldRot = GetWorldRotationAsQuaternion();
	FMatrix RotMatrix = FMatrix::RotationMatrix(WorldRot);

	// Forward is the first row
	return FVector(RotMatrix.Data[0][0], RotMatrix.Data[0][1], RotMatrix.Data[0][2]).GetNormalized();
}

FVector UCameraComponent::GetRightVector() const
{
	// Get right vector from world rotation
	FQuaternion WorldRot = GetWorldRotationAsQuaternion();
	FMatrix RotMatrix = FMatrix::RotationMatrix(WorldRot);

	// Right is the second row
	return FVector(RotMatrix.Data[1][0], RotMatrix.Data[1][1], RotMatrix.Data[1][2]).GetNormalized();
}

FVector UCameraComponent::GetUpVector() const
{
	// Get up vector from world rotation
	FQuaternion WorldRot = GetWorldRotationAsQuaternion();
	FMatrix RotMatrix = FMatrix::RotationMatrix(WorldRot);

	// Up is the third row
	return FVector(RotMatrix.Data[2][0], RotMatrix.Data[2][1], RotMatrix.Data[2][2]).GetNormalized();
}

void UCameraComponent::MarkMatricesDirty()
{
	bViewMatrixDirty = true;
	bProjectionMatrixDirty = true;
}
