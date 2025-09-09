#pragma once

#include "Math.h"


class FTransform
{
public:
	FTransform();
	~FTransform();

	void SetScale(float x, float y, float z);
	void SetScale(const FVector&);

	void SetRotationDegX(float degree);
	void SetRotationDegY(float degree);
	void SetRotationDegZ(float degree);
	void SetRotationDeg(const FVector&);

	void SetRotationDegByDrag(const FVector&);

	void AddRotationDegX(float degree);
	void AddRotationDegY(float degree);
	void AddRotationDegZ(float degree);

	void SetLocation(float x, float y, float z);
	void SetLocation(const FVector&);

	void AddLocation(const FVector&);

	void Translate(float dx, float dy, float dz);
	void Translate(const FVector&);

	const FVector& GetScale() const { return Scale; }
	const FVector& GetRotationRadians() const { return Rotation; }
	FVector GetRotationDegree() const;
	const FVector& GetLocation() const { return Location; }
	bool TryGetInverseMatrix(FMatrix& Out);
	FMatrix& GetTransformMatrix();
	
	FMatrix& Get2StepRotationMatrix(); 

private:

private:
	FMatrix Transform;
	FMatrix Inverse;
	FVector Scale;
	FVector Rotation;
	FVector Location;

	FVector PrevRotation;

	// 트랜스폼 변경 검사
	bool bIsTransformDirty;
	bool bIsInverseDirty;
};