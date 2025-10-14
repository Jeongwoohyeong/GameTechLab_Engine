#include "pch.h"
#include "CameraControlWidget.h"
#include "UI/UIManager.h" 
#include "ImGui/imgui.h"
#include "CameraActor.h"
#include "CameraComponent.h"
#include "Vector.h"
#include "World.h"
#include <algorithm>
#include "SViewportWindow.h"
#include "SMultiViewportWindow.h"

//// UE_LOG 대체 매크로
//#define UE_LOG(fmt, ...)

// Camera Mode
static const char* CameraMode[] = {
	"Perspective",
	"Orthographic"
};

UCameraControlWidget::UCameraControlWidget()
	: UWidget("Camera Control Widget")
	, UIManager(&UUIManager::GetInstance())
{
}

UCameraControlWidget::~UCameraControlWidget() = default;

void UCameraControlWidget::Initialize()
{
	// UIManager 참조 확보
	UIManager = &UUIManager::GetInstance();

	// GizmoActor 참조 획득
	GizmoActor = UIManager->GetGizmoActor();

	// 초기 기즈모 스페이스 모드 설정
	if (GizmoActor)
	{
		CurrentGizmoSpace = GizmoActor->GetSpace();
	}

	// UIManager에 자신 등록(씬 로드시 UI 재동기화 호출 받을 수 있게)
	if (UIManager)
	{
		UIManager->RegisterCameraControlWidget(this);
	}
}

void UCameraControlWidget::Update()
{
	// GizmoActor 참조 업데이트
	if (!GizmoActor && UIManager)
	{
		GizmoActor = UIManager->GetGizmoActor();
	}

	// 월드 정보 업데이트 (옵션)
	if (UIManager && UIManager->GetWorld())
	{
		UWorld* World = UIManager->GetWorld();
		WorldActorCount = static_cast<uint32>(World->GetActors().size());
	}
}

TArray<ACameraActor*> UCameraControlWidget::GetCurrentCamera() const
{
	TArray<ACameraActor*> Cameras;

	SViewportWindow** Viewports = GetEngine()->GetWorld()->GetMultiViewportWindow()->GetViewports();
	if (!Viewports)
		return Cameras;

	for (int64 i = 0; i < 4; i++)
	{
		Cameras.push_back(Viewports[i]->GetViewportClient()->GetCamera());
	}

	return Cameras;
}

void UCameraControlWidget::RenderWidget()
{
	TArray<ACameraActor*> Cameras = GetCurrentCamera();
	
	if (Cameras.empty())
	{
		ImGui::TextUnformatted("Camera not available.");
		ImGui::Separator();
		ImGui::TextUnformatted("No camera is currently set in UIManager.");
		return;
	}

	// 최초 1회 동기화
	if (!bSyncedOnce)
	{
		SyncFromCamera();
		bSyncedOnce = true;
	}

	ImGui::TextUnformatted("Camera Transform");
	ImGui::Spacing();

	// 카메라 이동속도 표시 및 조절 (World와 동기화)
	if (UIManager && UIManager->GetWorld())
	{
		// World에서 현재 카메라 이동 속도 가져오기
		float WorldMoveSpeed = UIManager->GetWorld()->GetCameraActor()->GetCameraSpeed();
		
		ImGui::Text("Move Speed: %.1f", WorldMoveSpeed);
		if (ImGui::SliderFloat("##MoveSpeed", &WorldMoveSpeed, 1.0f, 20.0f, "%.1f"))
		{
			// World에 카메라 이동속도 설정
			UIManager->GetWorld()->GetCameraActor()->SetCameraSpeed(WorldMoveSpeed);
			// 위젯의 로컬 값도 업데이트
			CameraMoveSpeed = WorldMoveSpeed;
		}
	}
	else
	{
		// World가 없는 경우 로컬 값만 표시
		ImGui::Text("Move Speed: %.1f (World not available)", CameraMoveSpeed);
		ImGui::SliderFloat("##MoveSpeed", &CameraMoveSpeed, 1.0f, 20.0f, "%.1f");
	}
	ImGui::Spacing();

	if (ImGui::BeginTabBar("CameraTabBar"))
	{
		for (int i = 0; i < 4; i++)
		{
			FString TabLabel = FString("Camera ") + std::to_string(i);
			if (ImGui::BeginTabItem(TabLabel.c_str()))
			{
				// 카메라 위치 제어
				FVector Location = Cameras[i]->GetActorLocation();
				if (ImGui::DragFloat3("Camera Location", &Location.X, 0.05f))
				{
					Cameras[i]->SetActorLocation(Location);
				}

				// 카메라 회전 제어 (Euler angles)
				FVector Rotation = Cameras[i]->GetActorRotation().ToEuler();
				bool RotationChanged = false;
				RotationChanged |= ImGui::DragFloat3("Camera Rotation", &Rotation.X, 0.1f);
				// Pitch 제한 (-89 ~ 89도)
				Rotation.X = FMath::Clamp(Rotation.X, -89.0f, 89.0f);
				if (RotationChanged)
				{
					FQuat NewRotation = FQuat::MakeFromEuler(Rotation);
					Cameras[i]->SetActorRotation(NewRotation);
				}

				ImGui::TextUnformatted("Camera Optics");
				ImGui::Spacing();
				ImGui::Separator();

				FString CameraUIText = FString("Viewport ") + std::to_string(i) + " Camera";
				ImGui::TextUnformatted(CameraUIText.c_str());

				// FOV 조절
				bool bChanged = false;
				bChanged |= ImGui::SliderFloat("FOV", &UiFovY[i], 1.0f, 170.0f, "%.1f");
				// Near/Far 조절
				bChanged |= ImGui::DragFloat("Z Near", &UiNearZ[i], 0.01f, 0.0001f, 1e6f, "%.4f");
				bChanged |= ImGui::DragFloat("Z Far", &UiFarZ[i], 0.1f, 0.001f, 1e7f, "%.3f");
				if (bChanged)
				{
					PushToCamera(i);
				}

				// 리셋 버튼
				if (ImGui::Button("Reset Optics"))
				{
					UiFovY[i] = 80.0f;
					UiNearZ[i] = 0.1f;
					UiFarZ[i] = 1000.0f;
					PushToCamera(i);
				}

				ImGui::Spacing();
				ImGui::Separator();

				// 카메라 정보 표시
				ImGui::Text("Camera Info:");
				ImGui::Text("  Position: (%.2f, %.2f, %.2f)", Location.X, Location.Y, Location.Z);
				ImGui::Text("  Rotation: (%.1f, %.1f, %.1f)", Rotation.X, Rotation.Y, Rotation.Z);

				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
	}

	// 월드 정보 표시
	ImGui::Text("World Information");
	ImGui::Text("Actor Count: %u", WorldActorCount);
	ImGui::Separator();

	AGridActor* gridActor = UIManager->GetWorld()->GetGridActor();
	if (gridActor)
	{
		float currentLineSize = gridActor->GetLineSize();
		if (ImGui::DragFloat("Grid Spacing", &currentLineSize, 0.1f, 0.1f, 1000.0f))
		{
			gridActor->SetLineSize(currentLineSize);
			EditorINI["GridSpacing"] = std::to_string(currentLineSize);
		}
	}
	else
	{
		ImGui::Text("GridActor not found in the world.");
	}

	ImGui::Text("Transform Editor");

	// 기즈모 스페이스 모드 선택
	if (GizmoActor)
	{
		const char* spaceItems[] = { "World", "Local" };
		int currentSpaceIndex = static_cast<int>(CurrentGizmoSpace);

		if (ImGui::Combo("Gizmo Space", &currentSpaceIndex, spaceItems, IM_ARRAYSIZE(spaceItems)))
		{
			if (UIManager)
			{
				CurrentGizmoSpace = static_cast<EGizmoSpace>(currentSpaceIndex);
				AActor* SelectedActor = UIManager->GetSelectedActor();
				GizmoActor->SetSpaceWorldMatrix(CurrentGizmoSpace, SelectedActor);
			}
		}

		const char* buttonText = CurrentGizmoSpace == EGizmoSpace::World ?
			"Switch to Local" : "Switch to World";

		if (ImGui::Button(buttonText))
		{
			if (UIManager)
			{
				// 스페이스 모드 전환
				CurrentGizmoSpace = (CurrentGizmoSpace == EGizmoSpace::World) ?
				EGizmoSpace::Local : EGizmoSpace::World;
				AActor* SelectedActor = UIManager->GetSelectedActor();
				GizmoActor->SetSpaceWorldMatrix(CurrentGizmoSpace, SelectedActor);
			}
		}

		ImGui::SameLine();
		ImGui::Text("Current: %s",
			CurrentGizmoSpace == EGizmoSpace::World ? "World" : "Local");
	}

}

void UCameraControlWidget::SyncFromCamera()
{
	TArray<ACameraActor*> Cameras = GetCurrentCamera();
	if (Cameras.empty())
		return;

	for (int i = 0; i < 4; i++)
	{
		// 카메라 컴포넌트에서 실제 설정 값 가져오기
		if (UCameraComponent* CameraComp = Cameras[i]->GetCameraComponent())
		{
			// 실제 카메라 컴포넌트에서 값 읽어오기
			UiFovY[i] = CameraComp->GetFOV();
			UiNearZ[i] = CameraComp->GetNearClip();
			UiFarZ[i] = CameraComp->GetFarClip();

			// 프로젝션 모드 설정
			CameraModeIndex[i] = (CameraComp->GetProjectionMode() == ECameraProjectionMode::Perspective) ? 0 : 1;

			UE_LOG("CameraControl: Synced from camera %d - FOV=%.1f, Near=%.4f, Far=%.1f, Mode=%d",
				i, UiFovY[i], UiNearZ[i], UiFarZ[i], CameraModeIndex[i]);
		}
	}
	
	// World에서 카메라 이동 속도 동기화
	if (UIManager && UIManager->GetWorld())
	{
		CameraMoveSpeed = UIManager->GetWorld()->GetCameraActor()->GetCameraSpeed();
		UE_LOG("CameraControl: Synced camera move speed from world - Speed=%.1f", CameraMoveSpeed);
	}
}

void UCameraControlWidget::PushToCamera(int64 CameraIndex)
{
	TArray<ACameraActor*> Cameras = GetCurrentCamera();
	if (Cameras.empty()) 
		return;

	// Near/Far 값 검증
	UiNearZ[CameraIndex] = FMath::Max(0.0001f, UiNearZ[CameraIndex]);
	UiFarZ[CameraIndex] = FMath::Max(UiNearZ[CameraIndex] + 0.0001f, UiFarZ[CameraIndex]);
	UiFovY[CameraIndex] = FMath::Clamp(UiFovY[CameraIndex], 1.0f, 170.0f);

	// 카메라 컴포넌트에 실제 설정 적용
	if (UCameraComponent* CameraComp = Cameras[CameraIndex]->GetCameraComponent())
	{
		// 카메라 설정 업데이트
		CameraComp->SetFOV(UiFovY[CameraIndex]);
		CameraComp->SetNearClipPlane(UiNearZ[CameraIndex]);
		CameraComp->SetFarClipPlane(UiFarZ[CameraIndex]);

		// 프로젝션 모드 설정
		ECameraProjectionMode NewMode = (CameraModeIndex[CameraIndex] == 0) ? ECameraProjectionMode::Perspective : ECameraProjectionMode::Orthographic;
		CameraComp->SetProjectionMode(NewMode);

		UE_LOG("CameraControl: Applied to camera %d - FOV=%.1f, Near=%.4f, Far=%.1f",
			CameraIndex, UiFovY[CameraIndex], UiNearZ[CameraIndex], UiFarZ[CameraIndex]);
	}
}
