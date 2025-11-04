#pragma once
#include "Component/Public/SceneComponent.h"

/**
 * @brief Camera projection mode
 */
enum class ECameraProjectionMode : uint8
{
	Perspective,
	Orthographic
};

/**
 * @brief UCameraComponent - Camera component that can be attached to actors
 * Similar to Unreal Engine's UCameraComponent
 * Provides camera view and projection matrices for rendering
 */
UCLASS()
class UCameraComponent : public USceneComponent
{
	GENERATED_BODY()
	DECLARE_CLASS(UCameraComponent, USceneComponent)

public:
	UCameraComponent();
	virtual ~UCameraComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime) override;
	virtual void Serialize(const bool bInIsLoading, JSON& InOutHandle) override;

	// ========== Camera Properties ==========

	/**
	 * @brief Set field of view (in degrees) for perspective mode
	 * @param InFOV Field of view angle in degrees (typically 60-90)
	 */
	void SetFieldOfView(float InFOV);

	/**
	 * @brief Get field of view in degrees
	 */
	float GetFieldOfView() const { return FieldOfView; }

	/**
	 * @brief Set aspect ratio (width / height)
	 * @param InAspect Aspect ratio (e.g., 16/9 = 1.777)
	 */
	void SetAspectRatio(float InAspect);

	/**
	 * @brief Get aspect ratio
	 */
	float GetAspectRatio() const { return AspectRatio; }

	/**
	 * @brief Set near clipping plane distance
	 * @param InNearPlane Distance to near plane
	 */
	void SetNearClipPlane(float InNearPlane);

	/**
	 * @brief Get near clipping plane distance
	 */
	float GetNearClipPlane() const { return NearClipPlane; }

	/**
	 * @brief Set far clipping plane distance
	 * @param InFarPlane Distance to far plane
	 */
	void SetFarClipPlane(float InFarPlane);

	/**
	 * @brief Get far clipping plane distance
	 */
	float GetFarClipPlane() const { return FarClipPlane; }

	/**
	 * @brief Set camera projection mode
	 * @param InMode Projection mode (Perspective or Orthographic)
	 */
	void SetProjectionMode(ECameraProjectionMode InMode);

	/**
	 * @brief Get camera projection mode
	 */
	ECameraProjectionMode GetProjectionMode() const { return ProjectionMode; }

	/**
	 * @brief Set orthographic width (for orthographic mode)
	 * @param InWidth Width of orthographic view
	 */
	void SetOrthoWidth(float InWidth);

	/**
	 * @brief Get orthographic width
	 */
	float GetOrthoWidth() const { return OrthoWidth; }

	/**
	 * @brief Set orthographic near clip plane (for orthographic mode)
	 * @param InNearPlane Near plane distance for ortho
	 */
	void SetOrthoNearClipPlane(float InNearPlane);

	/**
	 * @brief Get orthographic near clip plane
	 */
	float GetOrthoNearClipPlane() const { return OrthoNearClipPlane; }

	/**
	 * @brief Set orthographic far clip plane (for orthographic mode)
	 * @param InFarPlane Far plane distance for ortho
	 */
	void SetOrthoFarClipPlane(float InFarPlane);

	/**
	 * @brief Get orthographic far clip plane
	 */
	float GetOrthoFarClipPlane() const { return OrthoFarClipPlane; }

	// ========== Matrix Calculations ==========

	/**
	 * @brief Get the view matrix (world to camera space)
	 * Calculated from the component's world transform
	 */
	FMatrix GetViewMatrix() const;

	/**
	 * @brief Get the projection matrix (camera to clip space)
	 * Based on projection mode and camera parameters
	 */
	FMatrix GetProjectionMatrix() const;

	/**
	 * @brief Get view-projection matrix (combined)
	 */
	FMatrix GetViewProjectionMatrix() const;

	/**
	 * @brief Get camera constants for shaders
	 */
	struct FCameraConstants GetCameraConstants() const;

	/**
	 * @brief Convert screen point to world ray
	 * @param ScreenX Screen X coordinate (-1 to 1 in NDC)
	 * @param ScreenY Screen Y coordinate (-1 to 1 in NDC)
	 * @return Ray in world space
	 */
	FRay ScreenPointToWorldRay(float ScreenX, float ScreenY) const;

	// ========== Helper Functions ==========

	/**
	 * @brief Get camera forward vector (where camera is looking)
	 */
	FVector GetForwardVector() const;

	/**
	 * @brief Get camera right vector
	 */
	FVector GetRightVector() const;

	/**
	 * @brief Get camera up vector
	 */
	FVector GetUpVector() const;

	/**
	 * @brief Mark matrices as dirty (needs recalculation)
	 */
	void MarkMatricesDirty();

protected:
	/**
	 * @brief Update projection matrix
	 */
	void UpdateProjectionMatrix() const;

	/**
	 * @brief Update view matrix
	 */
	void UpdateViewMatrix() const;

private:
	// ========== Camera Parameters ==========

	/** Field of view in degrees (for perspective) */
	float FieldOfView = 90.0f;

	/** Aspect ratio (width / height) */
	float AspectRatio = 16.0f / 9.0f;

	/** Near clipping plane distance (perspective) */
	float NearClipPlane = 1.0f;

	/** Far clipping plane distance (perspective) */
	float FarClipPlane = 10000.0f;

	/** Projection mode (perspective or orthographic) */
	ECameraProjectionMode ProjectionMode = ECameraProjectionMode::Perspective;

	/** Orthographic width */
	float OrthoWidth = 512.0f;

	/** Orthographic near clip plane */
	float OrthoNearClipPlane = -1000.0f;

	/** Orthographic far clip plane */
	float OrthoFarClipPlane = 1000.0f;

	// ========== Cached Matrices ==========

	/** Cached view matrix */
	mutable FMatrix ViewMatrix;

	/** Cached projection matrix */
	mutable FMatrix ProjectionMatrix;

	/** Cached view-projection matrix */
	mutable FMatrix ViewProjectionMatrix;

	/** Whether view matrix needs update */
	mutable bool bViewMatrixDirty = true;

	/** Whether projection matrix needs update */
	mutable bool bProjectionMatrixDirty = true;
};