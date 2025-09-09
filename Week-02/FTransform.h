#pragma once

#include "Math.h"

class FTransform
{
public:
	FTransform();
	~FTransform();

	void SetScale(float x, float y, float z);
	void SetScale(const FVector&);
	
	void SetRotationDeg(const FVector&, bool bIsLocal);
	void SetRotationDegByDrag(const FVector&, bool bIsLocal);
	void AddRotationDeg(const FVector& Degree, bool bIsLocal);
	
	void LoadRotaion(const FVector& Rotation);
	void LoadQuaternion(const FQuaternion& Quat);

	void ClearRotation();

	void SetLocation(float x, float y, float z);
	void SetLocation(const FVector&);

	void AddLocation(const FVector&);

	void Translate(float dx, float dy, float dz);
	void Translate(const FVector&);

	const FVector& GetScale() const { return Scale; }
	const FVector& GetRotationRadians() const { return Rotation; }
	const FQuaternion& GetQuaternion() const { return Quaternion; }
	FVector GetRotationDegree() const;
	const FVector& GetLocation() const { return Location; }
	bool TryGetInverseMatrix(FMatrix& Out);
	
	FMatrix& GetTransformMatrix();

	FMatrix& Get2StepRotationMatrix(); 

private:
	void SetRotationDegX(float degree);
	void SetRotationDegY(float degree);
	void SetRotationDegZ(float degree);

	void AddRotationDegX(float degree);
	void AddRotationDegY(float degree);
	void AddRotationDegZ(float degree);

	void UpdateQuaternion(const FVector& DeltaRotation, bool bIsLocal);

private:
	FMatrix Transform;
	FMatrix Inverse;
	FVector Scale;
	FVector Rotation; // 라디안 단위, 오일러 각도 (Z -> X -> Y 순서)
	FVector Location;
	
	FQuaternion Quaternion; // Rotation 바뀔 때마다 업데이트
	FVector PrevRotation;

	// 트랜스폼 변경 검사
	bool bIsTransformDirty;
	bool bIsInverseDirty;
};