#include "FTransform.h"

FTransform::FTransform()
{
	Transform = FMatrix::Identity();
	Scale = FVector(1.0f, 1.0f, 1.0f);
	// Rotation = FVector(0.0f, 0.0f, 0.0f);
	Location = FVector(0.0f, 0.0f, 0.0f);
	Inverse = {};
	Quaternion = FQuaternion();
	// PrevRotation = Rotation;
	bIsTransformDirty = true;
	bIsInverseDirty = true;
}

FTransform::~FTransform()
{
}

void FTransform::SetScale(float x, float y, float z)
{
	Scale = FVector(x, y, z);

	bIsTransformDirty = true;
	bIsInverseDirty = true;
}

void FTransform::SetScale(const FVector& scale)
{

	Scale = FVector(scale.X, scale.Y, scale.Z);

	bIsTransformDirty = true;
	bIsInverseDirty = true;
}

//void FTransform::SetRotationDegX(float degree)
//{
//	Rotation.X = DegToRad(degree);
//
//	bIsTransformDirty = true;
//	bIsInverseDirty = true;
//}
//
//void FTransform::SetRotationDegY(float degree)
//{
//	Rotation.Y = DegToRad(degree);
//
//	bIsTransformDirty = true;
//	bIsInverseDirty = true;
//}
//
//void FTransform::SetRotationDegZ(float degree)
//{
//	Rotation.Z = DegToRad(degree);
//
//	bIsTransformDirty = true;
//	bIsInverseDirty = true;
//}

void FTransform::SetRotationDeg(const FVector& Degree)
{
	/*PrevRotation = Rotation;

	Rotation.X = DegToRad(Degree.X);
	Rotation.Y = DegToRad(Degree.Y);
	Rotation.Z = DegToRad(Degree.Z);

	FVector DeltaRot = Rotation - PrevRotation;
	UpdateQuaternion(DeltaRot, bIsLocal);

	bIsTransformDirty = true;
	bIsInverseDirty = true;*/

	// 입력받은 Degree를 라디안으로 변환
	FVector RadAngles = DegToRad(Degree);

	// 오일러 각으로부터 새로운 쿼터니언을 '생성'하여 바로 대입
	this->Quaternion = FQuaternion::CreateFromEulerAngles(RadAngles);
	this->Quaternion.Normalize();

	bIsTransformDirty = true;
}

void FTransform::SetRotationDegByDrag(const FVector& Degree, bool bIsLocal)
{
	/*PrevRotation = Rotation;

	Rotation.X = DegToRad(Degree.X);
	Rotation.Y = DegToRad(Degree.Y);
	Rotation.Z = DegToRad(Degree.Z);

	FVector DeltaRot = Rotation - PrevRotation;
	UpdateQuaternion(DeltaRot, bIsLocal);

	bIsTransformDirty = true;
	bIsInverseDirty = true;*/

	// 입력받은 Degree를 라디안으로 변환
	FVector RadAngles = DegToRad(Degree);

	// 오일러 각으로부터 새로운 쿼터니언을 '생성'하여 바로 대입
	this->Quaternion = FQuaternion::CreateFromEulerAngles(RadAngles);
	this->Quaternion.Normalize();

	bIsTransformDirty = true;
}

//void FTransform::AddRotationDegX(float degree)
//{
//	Rotation.X += DegToRad(degree);
//
//	bIsTransformDirty = true;
//	bIsInverseDirty = true;
//}
//
//void FTransform::AddRotationDegY(float degree)
//{
//	Rotation.Y += DegToRad(degree);
//
//	bIsTransformDirty = true;
//	bIsInverseDirty = true;
//}
//
//void FTransform::AddRotationDegZ(float degree)
//{
//	Rotation.Z += DegToRad(degree);
//
//	bIsTransformDirty = true;
//	bIsInverseDirty = true;
//}

void FTransform::AddRotationDeg(const FVector& DegreeDelta, bool bIsLocal)
{
	/*PrevRotation = Rotation;

	Rotation.X += DegToRad(Degree.X);
	Rotation.Y += DegToRad(Degree.Y);
	Rotation.Z += DegToRad(Degree.Z);

	FVector DeltaRot = Rotation - PrevRotation;
	UpdateQuaternion(DeltaRot, bIsLocal);

	bIsTransformDirty = true;
	bIsInverseDirty = true;*/

	// 입력받은 Degree 변화량을 라디안으로 변환
	FVector RadDelta = DegToRad(DegreeDelta);

	// 오일러 '변화량'으로 델타 쿼터니언을 생성
	FQuaternion DeltaQuaternion = FQuaternion::CreateFromEulerAngles(RadDelta);

	// 기준 축에 따라 곱셈 순서를 결정하여 현재 회전에 누적
	if (bIsLocal) {
		// 로컬 기준
		this->Quaternion = FQuaternion::Multiply(this->Quaternion, DeltaQuaternion);
	}
	else {
		// 월드 기준
		this->Quaternion = FQuaternion::Multiply(DeltaQuaternion, this->Quaternion);
	}

	this->Quaternion.Normalize();

	bIsTransformDirty = true;
}

void FTransform::AddRotationAxis(const FVector& Axis, float AngleRad)
{
	FQuaternion delta = FQuaternion::CreateFromAxisAngle(Axis, AngleRad);
	// 로컬 회전은 큐브의 현재 방향을 기준으로 회전해야 하므로,
	// 월드 축 기준 곱셈을 사용합니다.
	this->Quaternion = FQuaternion::Multiply(delta, this->Quaternion);
	this->Quaternion.Normalize();
}

//void FTransform::LoadRotaion(const FVector& Rotation)
//{
//	this->Rotation = Rotation;
//	PrevRotation = Rotation;
//	bIsTransformDirty = true;
//	bIsInverseDirty = true;
//}

void FTransform::LoadQuaternion(const FQuaternion& Quat)
{
	this->Quaternion = Quat;
	bIsTransformDirty = true;
	bIsInverseDirty = true;
}

void FTransform::ClearRotation()
{
	// Rotation = FVector(0.0f, 0.0f, 0.0f);
	Quaternion = FQuaternion();
	// PrevRotation = Rotation;
	bIsTransformDirty = true;
	bIsInverseDirty = true;
}

void FTransform::SetLocation(float x, float y, float z)
{
	Location = FVector(x, y, z);

	bIsTransformDirty = true;
	bIsInverseDirty = true;
}

void FTransform::SetLocation(const FVector& location)
{
	Location = FVector(location.X, location.Y, location.Z);
	bIsTransformDirty = true;
	bIsInverseDirty = true;
}

void FTransform::AddLocation(const FVector& location)
{
	Location += location;

	bIsTransformDirty = true;
	bIsInverseDirty = true;
}

void FTransform::AddScale(const FVector& scale)
{
	Scale += scale;

	bIsTransformDirty = true;
	bIsInverseDirty = true;
}

void FTransform::Translate(float dx = 0.0f, float dy = 0.0f, float dz = 0.0f)
{
	Location = Location + FVector(dx, dy, dz);

	bIsTransformDirty = true;
	bIsInverseDirty = true;
}

void FTransform::Translate(const FVector& offset)
{
	Location = Location + offset;

	bIsTransformDirty = true;
	bIsInverseDirty = true;
}

//FVector FTransform::GetRotationDegree() const
//{
//	/*return FVector(
//		RadToDeg(Rotation.X),
//		RadToDeg(Rotation.Y),
//		RadToDeg(Rotation.Z)
//	);*/
//}

bool FTransform::TryGetInverseMatrix(FMatrix& Out)
{
	// Rotation은 정규직교 보장된다고 가정
	// 스케일 값 중 절댓값 MATH_EPSILON 이하가 있으면 역행렬 계산 불가
	bool bCheckX = (Scale.X > MATH_EPSILON || Scale.X < -MATH_EPSILON);
	bool bCheckY = (Scale.Y > MATH_EPSILON || Scale.Y < -MATH_EPSILON);
	bool bCheckZ = (Scale.Z > MATH_EPSILON || Scale.Z < -MATH_EPSILON);
	if (!(bCheckX && bCheckY && bCheckZ))
	{
		return false;
	}

	FMatrix Result = FMatrix::Identity();
	if (bIsInverseDirty)
	{
		FMatrix InvScaleMatrix = FMatrix::Identity();
		InvScaleMatrix[0][0] = 1.0f / Scale.X;
		InvScaleMatrix[1][1] = 1.0f / Scale.Y;
		InvScaleMatrix[2][2] = 1.0f / Scale.Z;
		FMatrix InvRotationMatrix = FMatrix::MakeRotationFromQuaternion(Quaternion);
		InvRotationMatrix = FMatrix::Transpose(InvRotationMatrix);
		FMatrix InvTranslationMatrix = FMatrix::Identity();
		InvTranslationMatrix[3][0] = -Location.X;
		InvTranslationMatrix[3][1] = -Location.Y;
		InvTranslationMatrix[3][2] = -Location.Z;

		Inverse = InvTranslationMatrix * InvRotationMatrix * InvScaleMatrix;
		bIsInverseDirty = false;

		Out = Inverse;
	}

	return true;
}

/**
 * @brief 오브젝트의 현재 로컬 X축(오른쪽)이 월드 공간에서 어느 방향을 가리키는지 반환합니다.
 * @return 월드 공간 기준의 오른쪽 방향 벡터 (정규화됨)
 */
FVector FTransform::GetRightVector() const
{
	// 1. 로컬 오른쪽 벡터 정의
	const FVector localRight = FVector(1.0f, 0.0f, 0.0f);

	// 2. 현재 회전 쿼터니언으로 회전 행렬 생성
	FMatrix rotationMatrix = FMatrix::MakeRotationFromQuaternion(this->Quaternion);

	// 3. 회전 행렬을 로컬 벡터에 적용하여 월드 기준 전방 벡터 계산
	FVector worldRight = FVector(
		localRight.X * rotationMatrix[0][0] + localRight.Y * rotationMatrix[1][0] + localRight.Z * rotationMatrix[2][0],
		localRight.X * rotationMatrix[0][1] + localRight.Y * rotationMatrix[1][1] + localRight.Z * rotationMatrix[2][1],
		localRight.X * rotationMatrix[0][2] + localRight.Y * rotationMatrix[1][2] + localRight.Z * rotationMatrix[2][2]
	);

	return worldRight;
}

/**
 * @brief 오브젝트의 현재 로컬 Y축(위쪽)이 월드 공간에서 어느 방향을 가리키는지 반환합니다.
 * @return 월드 공간 기준의 위쪽 방향 벡터 (정규화됨)
 */
FVector FTransform::GetUpVector() const
{
	// 1. 로컬 위쪽 벡터 정의
	const FVector localUp = FVector(0.0f, 1.0f, 0.0f);
	// 2. 현재 회전 쿼터니언으로 회전 행렬 생성
	FMatrix rotationMatrix = FMatrix::MakeRotationFromQuaternion(this->Quaternion);
	// 3. 회전 행렬을 로컬 벡터에 적용하여 월드 기준 전방 벡터 계산
	FVector worldUp = FVector(
		localUp.X * rotationMatrix[0][0] + localUp.Y * rotationMatrix[1][0] + localUp.Z * rotationMatrix[2][0],
		localUp.X * rotationMatrix[0][1] + localUp.Y * rotationMatrix[1][1] + localUp.Z * rotationMatrix[2][1],
		localUp.X * rotationMatrix[0][2] + localUp.Y * rotationMatrix[1][2] + localUp.Z * rotationMatrix[2][2]
	);

	return worldUp;
}

/**
 * @brief 오브젝트의 현재 로컬 Z축(앞쪽)이 월드 공간에서 어느 방향을 가리키는지 반환합니다.
 * @return 월드 공간 기준의 앞쪽 방향 벡터 (정규화됨)
 */
FVector FTransform::GetForwardVector() const
{
	// 1. 로컬 전방 벡터 정의
	const FVector localForward = FVector(0.0f, 0.0f, 1.0f);
	// 2. 현재 회전 쿼터니언으로 회전 행렬 생성
	FMatrix rotationMatrix = FMatrix::MakeRotationFromQuaternion(this->Quaternion);
	// 3. 회전 행렬을 로컬 벡터에 적용하여 월드 기준 전방 벡터 계산
	FVector worldForward = FVector(
		localForward.X * rotationMatrix[0][0] + localForward.Y * rotationMatrix[1][0] + localForward.Z * rotationMatrix[2][0],
		localForward.X * rotationMatrix[0][1] + localForward.Y * rotationMatrix[1][1] + localForward.Z * rotationMatrix[2][1],
		localForward.X * rotationMatrix[0][2] + localForward.Y * rotationMatrix[1][2] + localForward.Z * rotationMatrix[2][2]
	);

	return worldForward;
}

FMatrix& FTransform::GetTransformMatrix()
{
	// SRT중 하나라도 변경되면 행렬 연산
	if (bIsTransformDirty)
	{
		// SRT중 하나라도 변경되면 행렬 연산
		FMatrix scaleMatrix = FMatrix::MakeScale(Scale);
		// FMatrix rotationMatrix = FMatrix::MakeRotation(Rotation); // 기존 오일러 각도 방식
		FMatrix rotationMatrix = FMatrix::MakeRotationFromQuaternion(Quaternion);
		FMatrix translationMatrix = FMatrix::MakeTranslation(Location);

		Transform = scaleMatrix * rotationMatrix * translationMatrix;		

		bIsTransformDirty = false;
	}

	return Transform;
}

void FTransform::UpdateQuaternion(const FVector& DeltaRotation, bool bIsLocal)
{
	if (DeltaRotation.IsNearlyZero())
	{
		return;
	}

	// 1. 변화량으로 델타 쿼터니언 생성
	// (Z -> X -> Y 순서)
	FQuaternion DeltaQuaternion = FQuaternion::CreateFromEulerAngles(DeltaRotation);

	// 2. 현재 회전에 델타 회전을 곱하여 누적
	if (bIsLocal)
	{
		// 로컬 기준
		Quaternion = FQuaternion::Multiply(Quaternion, DeltaQuaternion);
	}
	else
	{
		// 월드 기준
		Quaternion = FQuaternion::Multiply(DeltaQuaternion, Quaternion);
	}

	// 3. 정규화
	Quaternion.Normalize();

	bIsTransformDirty = true;
}

//TODO: 2단계 회전 구현
//FMatrix& FTransform::Get2StepRotationMatrix()
//{
//	FVector DeltaRot = Rotation - PrevRotation;
//
//	FMatrix Rot1 = FMatrix::MakeRotation(PrevRotation);
//	FMatrix Rot2 = FMatrix::MakeRotation(DeltaRot);
//
//	return FMatrix(
//		{ FVector4(Rot2[0][0], Rot2[0][1], Rot2[0][2], Rot2[0][3]) * Rot1,
//		  FVector4(Rot2[1][0], Rot2[1][1], Rot2[1][2], Rot2[1][3]) * Rot1,
//		  FVector4(Rot2[2][0], Rot2[2][1], Rot2[2][2], Rot2[2][3]) * Rot1,
//		  FVector4(Rot2[3][0], Rot2[3][1], Rot2[3][2], Rot2[3][3]) * Rot1
//		});
//}