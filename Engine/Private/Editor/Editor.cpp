#include "pch.h"
#include "Editor/Editor.h"

#include "Editor/Camera.h"
#include "Editor/Gizmo.h"
#include "Editor/Grid.h"
#include "Editor/Axis.h"
#include "Editor/ObjectPicker.h"
#include "Render/Renderer/Renderer.h"
#include "Render/Renderer/LineBatchRenderer.h"
#include "Manager/Level/LevelManager.h"
#include "Manager/UI/UIManager.h"
#include "Manager/Input/InputManager.h"
#include "Actor/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Level/Level.h"
#include "Render/UI/Widget/CameraControlWidget.h"
#include "Render/UI/Widget/ViewSettingsWidget.h"

IMPLEMENT_CLASS(UEditor, UObject)

UEditor::UEditor()
{
	ObjectPicker.SetCamera(&Camera);

	// Set Camera to Control Panel
	auto& UIManager = UUIManager::GetInstance();
	UCameraControlWidget* CameraControlWidget =
		Cast<UCameraControlWidget>(UIManager.FindWidget("UCameraControlWidget"));
	CameraControlWidget->SetCamera(&Camera);
	UViewSettingsWidget* ViewSettingsWidget =
		Cast<UViewSettingsWidget>(UIManager.FindWidget("UViewSettingsWidget"));
	ViewSettingsWidget->SetGrid(&Grid);
};

UEditor::~UEditor() = default;

void UEditor::Initialize()
{
	ULevelManager::GetInstance().CreateNewLevel("Default");
}

void UEditor::Shutdown()
{
	// 종료 시 매인 카메라 설정 저장
	Camera.SaveCameraSettings();

	// 렌더러에서 멀티뷰 카메라 설정을 저장할 것이지만,
	// 추가적으로 여기서도 저장하여 안전성 확보
	auto& Renderer = URenderer::GetInstance();
	Renderer.SaveMultiViewCameraSettings();
}
void UEditor::Update()
{
	auto& Renderer = URenderer::GetInstance();
	ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();

	// 현재 레벨에 카메라 주입 보장
	EnsureLevelHasCamera(Level);
	// 입력 루틴이 카메라 파라미터를 직접 바꾸므로
	ProcessMouseInput(Level);
	ProcessKeyboardInput();

	// Gate main camera input: in quad, if RMB and hovering non-main viewport, disable main camera input
	if (Renderer.GetViewportLayout() == URenderer::EViewportLayout::Quad)
	{
		const UInputManager& Input = UInputManager::GetInstance();
		const FVector mp = Input.GetMousePosition();
		FRect rect{};
		int hover = Renderer.GetHoveredViewportIndex(mp.X, mp.Y, rect);
		bool enableMain = !(!Camera.IsDragging() && hover > 0 );
		Camera.SetInputEnabled(enableMain);
	}
	else
	{
		Camera.SetInputEnabled(true);
	}

	// 이전 프레임의 카메라 상태 저장
	static FVector PrevCameraLocation = Camera.GetLocation();
	static FVector PrevCameraRotation = Camera.GetRotation();

	Camera.Update();
	
	// 카메라 상태 변경 감지 및 자동 저장 요청
	FVector CurrentLocation = Camera.GetLocation();
	FVector CurrentRotation = Camera.GetRotation();
	if ((CurrentLocation - PrevCameraLocation).Length() > 0.1f || 
	    (CurrentRotation - PrevCameraRotation).Length() > 0.1f)
	{
		Renderer.RequestCameraSave();
		PrevCameraLocation = CurrentLocation;
		PrevCameraRotation = CurrentRotation;
	}

	for(int i = 0; i < 4; ++i)
	{
		UCamera* Cam = Renderer.GetViewCameraAt(i);
		if (i == 0)
		{
			Camera.GetCameraType() == ECameraType::ECT_Perspective ? Cam->SetCameraType(ECameraType::ECT_Perspective) : Cam->SetCameraType(ECameraType::ECT_Orthographic);
			Cam->SetFovY(Camera.GetFovY());
		}
		Cam->SetNearZ(Camera.GetNearZ());
		Cam->SetFarZ(Camera.GetFarZ());
	}

	Renderer.UpdateConstant(Camera.GetFViewProjConstants());
}

// void UEditor::RenderEditor()
// {
// 	Grid.RenderGrid();
// 	Axis.Render();
// 	Gizmo.RenderGizmo(ULevelManager::GetInstance().GetCurrentLevel()->GetSelectedActor(), Camera.GetLocation());
// }

const FVector& UEditor::GetCameraLocation()
{
	return Camera.GetLocation();
}

void UEditor::RenderEditorBatched(const FVector& CameraLocation)
{
	ULineBatchRenderer& LineBatch = ULineBatchRenderer::GetInstance();

	/** 모든 라인 렌더링을 하나의 배치로 통합 */
	LineBatch.BeginBatch();
	{
		/** Grid 라인들 추가 */
		Grid.AddToLineBatch(LineBatch);

		/** Axis 라인들 추가 */
		Axis.AddToLineBatch(LineBatch);

		/** AABB 라인들 추가 (Min/Max 입력 기반, 인스턴싱) */
		//StaticMesh 구현 후 처리
		ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();
		if (Level)
		{
			const TArray<FAABB>& AABBsToRender = Level->GetAABBsToRender();
			for (auto& Bound : AABBsToRender)
			{
				LineBatch.AddAABB(Bound.Min, Bound.Max, FVector4(0, 1, 0, 1));
			}
			
		}
	}
	
	/** 1회 드로우콜로 모든 라인 렌더링 */
	LineBatch.FlushBatch();

	/** Gizmo는 별도로 렌더링 (기존 방식 유지) */
	Gizmo.RenderGizmo(ULevelManager::GetInstance().GetCurrentLevel()->GetSelectedActor(), CameraLocation);
}


void UEditor::ProcessKeyboardInput()
{
	const UInputManager& InputManager = UInputManager::GetInstance();
	auto& Renderer = URenderer::GetInstance();

	// Alt+C로 바운딩 박스 토글
	if (InputManager.IsKeyDown(EKeyInput::Alt) && InputManager.IsKeyPressed(EKeyInput::C))
	{
		Renderer.ToggleShowFlag(EEngineShowFlags::SF_Bounds);
	}
	if (InputManager.IsKeyPressed(EKeyInput::Space))
	{
		Gizmo.ChangeGizmoMode();
	}

	// F2: Toggle viewport layout (Single <-> Quad)
	if (InputManager.IsKeyPressed(EKeyInput::F2))
	{
		Renderer.ToggleViewportLayout();
	}

	// Gizmo 표시 중 Tab: 월드→로컬 토글 (기본: 토글, 최초 누르면 로컬 보장)
	if (InputManager.IsKeyPressed(EKeyInput::Tab))
	{
		ULevel* Level = ULevelManager::GetInstance().GetCurrentLevel();
		if (Level && Level->GetSelectedActor())
		{
			if (Gizmo.IsWorld())
			{
				Gizmo.SetLocal();
			}
			else
			{
				Gizmo.SetWorld();
			}
		}
	}
}

void UEditor::ProcessMouseInput(ULevel* InLevel)
{
	const UInputManager& InputManager = UInputManager::GetInstance();
	URenderer& Renderer = URenderer::GetInstance();
	// If user is dragging a splitter, suppress picking/gizmo interactions to avoid conflicts
	if (Renderer.IsDraggingSplitter())
	{
		return;
	}

	static EGizmoDirection PreviousGizmoDirection = EGizmoDirection::None;
	AActor* ActorPicked = InLevel->GetSelectedActor();
	float ActorDistance = -1;

	// Single-view must behave exactly like before: use global NDC and main camera
	const bool bIsQuad = (Renderer.GetViewportLayout() == URenderer::EViewportLayout::Quad);
	// Hovered viewport info (valid only in quad)
	FRect HoverRect{0,0,0,0};
	int HoverIndex = -1;
	UCamera* PickCam = nullptr;
	FRay WorldRay = {};
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		if (!bIsQuad)
		{
			FVector MousePositionNdc = InputManager.GetMouseNDCPosition();
			PickCam = &Camera;
			// 싱글뷰는 기존대로 전체 화면 NDC를 사용하지만, 멀티뷰 대비 안전을 위해 종횡비와 행렬 업데이트
			if (URenderer::GetInstance().GetViewportLayout() == URenderer::EViewportLayout::Quad)
			{
				DXGI_SWAP_CHAIN_DESC scd = {};
				URenderer::GetInstance().GetSwapChain()->GetDesc(&scd);
				float vpAspect = (scd.BufferDesc.Height > 0) ? (float)scd.BufferDesc.Width / (float)scd.BufferDesc.Height : 1.0f;
				PickCam->SetAspect(vpAspect);
				if (PickCam->GetCameraType() == ECameraType::ECT_Perspective) PickCam->UpdateMatrixByPers(); else PickCam->UpdateMatrixByOrth();
			}
			WorldRay = Camera.ConvertToWorldRay(MousePositionNdc.X, MousePositionNdc.Y);
		}
		else
		{
			FVector pos;
			// Determine hovered viewport and build ray using its camera
			const FVector MousePosPx = InputManager.GetMousePosition();
			HoverIndex = Renderer.GetHoveredViewportIndex(MousePosPx.X, MousePosPx.Y, HoverRect);
			if (HoverIndex < 0)
			{
				HoverIndex = 0;
				DXGI_SWAP_CHAIN_DESC scd = {};
				Renderer.GetSwapChain()->GetDesc(&scd);
				HoverRect = { 0.0f, 0.0f, (float)scd.BufferDesc.Width, (float)scd.BufferDesc.Height };
			}
			// Convert pixel to local NDC of that viewport rect
			float LocalNdcX = ((MousePosPx.X - HoverRect.X) / HoverRect.W) * 2.0f - 1.0f;
			float LocalNdcY = 1.0f - ((MousePosPx.Y - HoverRect.Y) / HoverRect.H) * 2.0f;

			// 항상 뷰포트 전용 카메라를 사용 (TL도 ViewCameras[0])
			PickCam = Renderer.GetViewCameraAt(HoverIndex);
			if (!PickCam || Camera.IsDragging())
			{
				PickCam = Renderer.GetViewCameraAt(0);
			}
			// 멀티뷰에서는 선택된 뷰포트 타입에 맞춰 카메라 파라미터를 맞춘 뒤 종횡비 반영
			URenderer::EViewportType VType = URenderer::EViewportType::Perspective;
			switch (HoverIndex)
			{
			case 0: VType = URenderer::EViewportType::Perspective; break; // TL
			case 1: VType = URenderer::EViewportType::Right; break;       // BL
			case 2: VType = URenderer::EViewportType::Top; break;         // TR
			case 3: VType = URenderer::EViewportType::Front; break;       // BR
			default: break;
			}
			switch (VType)
			{
			case URenderer::EViewportType::Perspective:
				PickCam->SetCameraType(ECameraType::ECT_Perspective);
				PickCam->SetLocation(Camera.GetLocation());
				PickCam->SetRotation(Camera.GetRotation());
				break;
			default:
				for (int Index = 1; Index < 4; Index++)
				{
					UCamera* PickCam1 = Renderer.GetViewCameraAt(Index);
					if (Index == 2)
					{
						PickCam1->SetCameraType(ECameraType::ECT_Orthographic);
						PickCam1->SetNearZ(0.1f); PickCam1->SetFarZ(1000.f);
						PickCam1->SetLocation(Pos + FVector(0, 0, CameraDistance));
						PickCam1->SetRotation(FVector(90.0f, 0.0f, 0.0f));
					}
					else if (Index == 1)	//Right
					{
						PickCam1->SetCameraType(ECameraType::ECT_Orthographic);
						PickCam1->SetNearZ(0.1f); PickCam1->SetFarZ(1000.f);
						PickCam1->SetLocation(Pos + FVector(0, CameraDistance, 0));
						PickCam1->SetRotation(FVector(0.0f, -90.0f, 0.0f));
					}
					else
					{
						PickCam1->SetCameraType(ECameraType::ECT_Orthographic);
						PickCam1->SetNearZ(0.1f); PickCam1->SetFarZ(1000.f);
						PickCam1->SetLocation(Pos + FVector(-CameraDistance, 0, 0));
						PickCam1->SetRotation(FVector(0.0f, 0.0f, 0.0f));
					}

				}
			}
			// Hovered viewport interactive controls (orthographic): pan with WASD/Arrows, zoom with wheel
			if (PickCam->GetCameraType() == ECameraType::ECT_Orthographic)
			{
				const UInputManager& InputForView = UInputManager::GetInstance();
				bool rmbView = InputForView.IsKeyDown(EKeyInput::MouseRight);
				// Zoom (wheel always active)
				float wheel = InputForView.GetMouseWheelDelta();
				if (wheel != 0.0f)
				{
					float width = Renderer.GetOrthoWorldWidthConst();

					float scale = (wheel > 0.0f) ? 0.9f : 1.1f;
					for (int i = 0; i < (int)std::abs(wheel); ++i) width *= scale;
					width = std::max(1.0f, std::min(5000.0f, width));
					//for (int Index = 0; Index < 4;Index++)
						Renderer.SetOrthoWorldWidthConst(width);
					CameraDistance *= scale;
				}
				// Pan (only when RMB is held over this viewport)
				if (rmbView)
				{
					float PickCameraSpeed = Camera.GetMoveSpeed();
					float base = std::max(0.001f, PickCam->GetOrthoWorldWidth() * 0.06f) * PickCameraSpeed * DT;
					bool keyW = InputForView.IsKeyDown(EKeyInput::W) || InputForView.IsKeyDown(EKeyInput::Up);
					bool keyS = InputForView.IsKeyDown(EKeyInput::S) || InputForView.IsKeyDown(EKeyInput::Down);
					bool keyA = InputForView.IsKeyDown(EKeyInput::A) || InputForView.IsKeyDown(EKeyInput::Left);
					bool keyD = InputForView.IsKeyDown(EKeyInput::D) || InputForView.IsKeyDown(EKeyInput::Right);
					switch (VType)
					{
					case URenderer::EViewportType::Top:
						if (keyW) Pos.X += base; if (keyS) Pos.X -= base; if (keyA) Pos.Y -= base; if (keyD) Pos.Y += base;
						break;
					case URenderer::EViewportType::Right:
						if (keyW) Pos.Z += base; if (keyS) Pos.Z -= base; if (keyA) Pos.X -= base; if (keyD) Pos.X += base;
						break;
					case URenderer::EViewportType::Front:
						if (keyW) Pos.Z += base; if (keyS) Pos.Z -= base; if (keyA) Pos.Y -= base; if (keyD) Pos.Y += base;
						break;
					default: break;
					}
					//PickCam->SetLocation(pos);
				}
			}

			// 선택된 뷰포트 직사각형 기준의 종횡비를 카메라에 적용해 정확한 레이 계산
			float vpAspect = (HoverRect.H > 0.0f) ? (HoverRect.W / HoverRect.H) : 1.0f;
			PickCam->SetAspect(vpAspect);
			if (PickCam->GetCameraType() == ECameraType::ECT_Perspective) PickCam->UpdateMatrixByPers(); else PickCam->UpdateMatrixByOrth();
			// 월드 레이
			WorldRay = PickCam->ConvertToWorldRay(LocalNdcX, LocalNdcY);
			{
				Gizmo.UpdateCollisionScaleForCamera(PickCam->GetLocation());
			}
			// 월드 레이
			WorldRay = PickCam->ConvertToWorldRay(LocalNdcX, LocalNdcY);
		}
	}

	if (InputManager.IsKeyReleased(EKeyInput::MouseLeft))
	{
		// 회전 모드에서 릴리즈 시 마지막 각도 커밋 (로컬/월드 동일)
		if (Gizmo.IsDragging() && Gizmo.GetSelectedActor() && Gizmo.GetGizmoMode() == EGizmoMode::Rotate)
		{
			FQuat FinalQuat = GetGizmoDragRotationQuat(WorldRay, *PickCam);
			Gizmo.SetActorRotation(FinalQuat);
		}
		Gizmo.EndDrag();
	}

	if (Gizmo.IsDragging() && Gizmo.GetSelectedActor())
	{
		switch (Gizmo.GetGizmoMode())
		{
		case EGizmoMode::Translate:
			{
				FVector GizmoDragLocation = GetGizmoDragLocation(WorldRay, *PickCam);
				Gizmo.SetLocation(GizmoDragLocation);
				break;
			}
		case EGizmoMode::Rotate:
			{
				FQuat GizmoDragRotation = GetGizmoDragRotationQuat(WorldRay, *PickCam);
				Gizmo.SetActorRotation(GizmoDragRotation);
				break;
			}
		case EGizmoMode::Scale:
			{
				FVector GizmoDragScale = GetGizmoDragScale(WorldRay, *PickCam);
				Gizmo.SetActorScale(GizmoDragScale);
			}
		}
	}
	else
	{
		FVector CollisionPoint;
		/** 기즈모가 출력되고있음. 레이캐스팅을 계속 해야함. */
		if (InLevel->GetSelectedActor())
		{
			ObjectPicker.PickGizmo(WorldRay, Gizmo, CollisionPoint);
		}
		else
		{
			Gizmo.SetGizmoDirection(EGizmoDirection::None);
		}
		// 뷰포트 위에 마우스가 있을 때는 ImGui 캡처 여부와 관계 없이 피킹 허용
		/*bool bAllowPicking = true;
		if (bIsQuad)
		{
			bAllowPicking = (HoverIndex >= 0);
		}*/
		if (!ImGui::GetIO().WantCaptureMouse && InputManager.IsKeyPressed(EKeyInput::MouseLeft))
		{
			TArray<UPrimitiveComponent*> Candidate = FindCandidatePrimitives(InLevel);

			UPrimitiveComponent* PrimitiveCollided = ObjectPicker.PickPrimitive(WorldRay, Candidate, &ActorDistance);

			if (PrimitiveCollided)
			{
				ActorPicked = PrimitiveCollided->GetOwner();
			}
			else
			{
				ActorPicked = nullptr;
			}
		}

		/** 기즈모에 호버링되거나 클릭되지 않았을 때. Actor 업데이트해줌. */
		if (Gizmo.GetGizmoDirection() == EGizmoDirection::None)
		{
			InLevel->SetSelectedActor(ActorPicked);
			if (PreviousGizmoDirection != EGizmoDirection::None)
			{
				Gizmo.OnMouseRelease(PreviousGizmoDirection);
			}
		}
		/** 기즈모가 선택되었을 때. Actor가 선택되지 않으면 기즈모도 선택되지 않으므로 이미 Actor가 선택된 상황. */
		/** SelectedActor를 update하지 않고 마우스 인풋에 따라 hovering or drag */
		else
		{
			PreviousGizmoDirection = Gizmo.GetGizmoDirection();
			/** 드래그 */
			if (InputManager.IsKeyPressed(EKeyInput::MouseLeft))
			{
				Gizmo.OnMouseDragStart(CollisionPoint);
			}
			else
			{
				Gizmo.OnMouseHovering();
			}
		}
	}
}

TArray<UPrimitiveComponent*> UEditor::FindCandidatePrimitives(ULevel* InLevel)
{
	TArray<UPrimitiveComponent*> Candidate;

	for (AActor* Actor : InLevel->GetLevelActors())
	{
		for (auto& ActorComponent : Actor->GetOwnedComponents())
		{
			UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(ActorComponent);
			if (Primitive)
			{
				Candidate.push_back(Primitive);
			}
		}
	}

	return Candidate;
}

FVector UEditor::GetGizmoDragLocation(const FRay& WorldRay, const UCamera& ViewCam)
{
	FVector MouseWorld;
	FVector PlaneOrigin{Gizmo.GetGizmoLocation()};
	const FVector AxisLocal = Gizmo.GetGizmoAxis(); // (1,0,0) or (0,1,0) or (0,0,1)
	FVector GizmoAxis = AxisLocal;

	// 로컬 모드일 때는 드래그 시작 시점의 로컬 축을 월드로 변환 해줌
	if (!Gizmo.IsWorld())
	{
		const FQuat QuatDragStart = Gizmo.GetDragStartActorRotationQuat();
		GizmoAxis = QuatDragStart.RotateVector(AxisLocal);
	}

	// Compute camera forward from view camera rotation (X-forward convention)
	FVector4 CamForward4 = FVector4(1, 0, 0, 1) * FMatrix::RotationMatrixCamera(FVector::GetDegreeToRadian(ViewCam.GetRotation()));
	FVector CamForward = FVector(CamForward4.X, CamForward4.Y, CamForward4.Z);
	CamForward.Normalize();
	FVector PlaneNormal = (CamForward.Cross(GizmoAxis)).Cross(GizmoAxis);
	if (ObjectPicker.IsRayCollideWithPlane(WorldRay, PlaneOrigin, PlaneNormal, MouseWorld))
	{
		FVector MouseDistance = MouseWorld - Gizmo.GetDragStartMouseLocation();
		return Gizmo.GetDragStartActorLocation() + GizmoAxis * MouseDistance.Dot(GizmoAxis);
	}
	return Gizmo.GetGizmoLocation();
}

FVector UEditor::GetGizmoDragRotation(const FRay& WorldRay, const UCamera& /*ViewCam*/)
{
	FVector MouseWorld;
	FVector PlaneOrigin{Gizmo.GetGizmoLocation()};
	FVector GizmoAxis = Gizmo.GetGizmoAxis();

	if (ObjectPicker.IsRayCollideWithPlane(WorldRay, PlaneOrigin, GizmoAxis, MouseWorld))
	{
		FVector PlaneOriginToMouse = MouseWorld - PlaneOrigin;
		FVector PlaneOriginToMouseStart = Gizmo.GetDragStartMouseLocation() - PlaneOrigin;
		PlaneOriginToMouse.Normalize();
		PlaneOriginToMouseStart.Normalize();
		float Angle = acosf((PlaneOriginToMouseStart).Dot(PlaneOriginToMouse)); //플레인 중심부터 마우스까지 벡터 이용해서 회전각도 구하기
		if ((PlaneOriginToMouse.Cross(PlaneOriginToMouseStart)).Dot(GizmoAxis) < 0) // 회전축 구하기
		{
			Angle = -Angle;
		}
		return Gizmo.GetDragStartActorRotation() + GizmoAxis * FVector::GetRadianToDegree(Angle);
	}
	return Gizmo.GetActorRotation();
}

FQuat UEditor::GetGizmoDragRotationQuat(const FRay& WorldRay, const UCamera& /*ViewCam*/)
{
	FVector MouseWorld;
	FVector PlaneOrigin{Gizmo.GetGizmoLocation()};
	const FVector AxisLocal = Gizmo.GetGizmoAxis(); // (1,0,0) or (0,1,0) or (0,0,1)
	FVector AxisWorld = AxisLocal;


	// Local gizmo: 드래그 시작 시점의 로컬 축을 월드로 변환해 고정
	if (!Gizmo.IsWorld())
	{
		const FQuat QuatDragStart = Gizmo.GetDragStartActorRotationQuat();
		AxisWorld = QuatDragStart.RotateVector(AxisLocal);
	}

	if (ObjectPicker.IsRayCollideWithPlane(WorldRay, PlaneOrigin, AxisWorld, MouseWorld))
	{
		FVector V1 = Gizmo.GetDragStartMouseLocation() - PlaneOrigin;
		FVector V2 = MouseWorld - PlaneOrigin;
		V1.Normalize();
		V2.Normalize();
		float Angle = acosf(clamp(V1.Dot(V2), -1.0f, 1.0f));
		if (V2.Cross(V1).Dot(AxisWorld) < 0)
		{
			Angle = -Angle;
		}

		// 드래그 시작 시점의 오리엔테이션
		const FQuat QuatDragStart = Gizmo.GetDragStartActorRotationQuat();
		// 델타 회전 쿼터니언 구성
		// 월드: 월드축 기준 델타, 좌곱
		// 로컬: 로컬축 기준 델타, 우곱 
		if (Gizmo.IsWorld())
		{
			const FQuat QuatDelta = FQuat::FromAxisAngle(AxisWorld, Angle);
			return QuatDelta * QuatDragStart;
		}
		else
		{
			const FQuat QuatDeltaLocal = FQuat::FromAxisAngle(AxisLocal, Angle);
			return QuatDragStart * QuatDeltaLocal;
		}
	}
	return Gizmo.GetActorRotationQuat();
}

FVector UEditor::GetGizmoDragScale(const FRay& WorldRay, const UCamera& ViewCam)
{
	FVector MouseWorld;
	FVector PlaneOrigin{Gizmo.GetGizmoLocation()};
	FVector GizmoAxis = Gizmo.GetGizmoAxis();

	// 스케일은 항상 로컬 기준으로 작동: 시작 시점 로컬축을 월드로 고정
	GizmoAxis = Gizmo.GetDragStartActorRotationQuat().RotateVector(GizmoAxis);

	// Compute camera forward from view camera rotation (X-forward convention)
	FVector4 CamForward4 = FVector4(1, 0, 0, 1) * FMatrix::RotationMatrixCamera(FVector::GetDegreeToRadian(ViewCam.GetRotation()));
	FVector CamForward = FVector(CamForward4.X, CamForward4.Y, CamForward4.Z);
	CamForward.Normalize();
	FVector PlaneNormal = (CamForward.Cross(GizmoAxis)).Cross(GizmoAxis);
	if (ObjectPicker.IsRayCollideWithPlane(WorldRay, PlaneOrigin, PlaneNormal, MouseWorld))
	{
		FVector PlaneOriginToMouse = MouseWorld - PlaneOrigin;
		FVector PlaneOriginToMouseStart = Gizmo.GetDragStartMouseLocation() - PlaneOrigin;
		float DragStartAxisDistance = PlaneOriginToMouseStart.Dot(GizmoAxis);
		float DragAxisDistance = PlaneOriginToMouse.Dot(GizmoAxis);
		float ScaleFactor = 1.0f;
		if (abs(DragStartAxisDistance) > 0.1f)
		{
			ScaleFactor = DragAxisDistance / DragStartAxisDistance;
		}

		FVector DragStartScale = Gizmo.GetDragStartActorScale();
		if (ScaleFactor > MinScale)
		{
			// Uniform이면 모든 축 동일 비율 스케일
			if (Gizmo.GetSelectedActor()->IsUniformScale())
			{
				return {
					DragStartScale.X * ScaleFactor,
					DragStartScale.Y * ScaleFactor,
					DragStartScale.Z * ScaleFactor
				};
			}

			// 비균일: 선택 축의 성분만 스케일 변경
			FVector NewScale = DragStartScale;
			switch (Gizmo.GetGizmoDirection())
			{
			case EGizmoDirection::Right:
				NewScale.X = max(DragStartScale.X * ScaleFactor, MinScale);
				break;
			case EGizmoDirection::Up:
				NewScale.Y = max(DragStartScale.Y * ScaleFactor, MinScale);
				break;
			case EGizmoDirection::Forward:
				NewScale.Z = max(DragStartScale.Z * ScaleFactor, MinScale);
				break;
			default: break;
			}
			return NewScale;
		}
		return Gizmo.GetActorScale();
	}
	return Gizmo.GetActorScale();
}

// 레벨이 카메라를 가지도록 보장
void UEditor::EnsureLevelHasCamera(ULevel* Level)
{
	if (!Level) return;

	// 포인터 주입
	if (Level->GetCamera() != &Camera)
	{
		Level->SetCamera(&Camera);
	}
	// 매 프레임 '안전 적용' 시도: 더티면 한 번만 적용되고 플래그가 꺼짐
	Level->TryApplySavedCameraSnapshot();
}
