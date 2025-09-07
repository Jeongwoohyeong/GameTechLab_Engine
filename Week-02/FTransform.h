#pragma once

#include "Math.h"


class FTransform
{
public:
	FTransform();
	~FTransform();

	void SetScale(float x, float y, float z);
	void SetScale(const FVector&);

	void SetRotationX(float degree);
	void SetRotationY(float degree);
	void SetRotationZ(float degree);
	void SetRotation(const FVector&);

	void AddRotationX(float degree);
	void AddRotationY(float degree);
	void AddRotationZ(float degree);

	void SetLocation(float x, float y, float z);
	void SetLocation(const FVector&);

	void Translate(float dx, float dy, float dz);
	void Translate(const FVector&);

	const FVector& GetScale() const { return Scale; }
	const FVector& GetRotationRadians() const { return Rotation; }
	FVector GetRotationDegree() const;
	const FVector& GetLocation() const { return Location; }
	const FMatrix& GetInverseMatrix();
	FMatrix& GetTransformMatrix();
private:

private:
	FMatrix Transform;
	FMatrix Inverse;
	FVector Scale;
	FVector Rotation;
	FVector Location;	

	// 트랜스폼 변경 검사
	bool isDirty;
};