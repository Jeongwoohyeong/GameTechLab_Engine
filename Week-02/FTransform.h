#pragma once

#include "Math.h"

class FTransform
{
public:
	FTransform();
	~FTransform();

	void SetScale(float x, float y, float z);
	void SetScale(const FVector&);
	
	void SetRotationDeg(const FVector&);
	void SetRotationDegByDrag(const FVector&, bool bIsLocal);
	void AddRotationDeg(const FVector& DegreeDelta, bool bIsLocal);
	void AddRotationAxis(const FVector& Axis, float AngleRad);
	void MultiplyQuaternion(const FQuaternion& QuatDelta, bool bIsLocal);

	// void LoadRotaion(const FVector& Rotation);
	void LoadQuaternion(const FQuaternion& Quat);

	void ClearRotation();

	void SetLocation(float x, float y, float z);
	void SetLocation(const FVector&);

	void AddLocation(const FVector&);
	void AddScale(const FVector&);

	void Translate(float dx, float dy, float dz);
	void Translate(const FVector&);

	const FVector& GetScale() const { return Scale; }
	// const FVector& GetRotationRadians() const { return Rotation; }
	const FQuaternion& GetQuaternion() const { return Rotation; }
	// FVector GetRotationDegree() const;
	const FVector& GetLocation() const { return Location; }
	bool TryGetInverseMatrix(FMatrix& Out);

	FVector GetRightVector() const;
	FVector GetUpVector() const;
	FVector GetForwardVector() const;
	
	FMatrix& GetTransformMatrix();

	// FMatrix& Get2StepRotationMatrix(); 

private:
	/*void SetRotationDegX(float degree);
	void SetRotationDegY(float degree);
	void SetRotationDegZ(float degree);

	void AddRotationDegX(float degree);
	void AddRotationDegY(float degree);
	void AddRotationDegZ(float degree);*/

	// void UpdateQuaternion(const FVector& DeltaRotation, bool bIsLocal);

private:
	FMatrix Transform;
	FMatrix Inverse;
	FVector Scale;
	FQuaternion Rotation;
	FVector Location;

	// 트랜스폼 변경 검사
	bool bIsTransformDirty;
	bool bIsInverseDirty;
};