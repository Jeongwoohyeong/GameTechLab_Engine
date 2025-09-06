#pragma once
#include "FTransform.h"

class UCamera
{
public:
	void SetCameraPosition(const float x, const float y, const float z);
	void SetCameraPosition(const FVector&);

	void SetCameraRotation(const float degreeX, const float degreeY, const float degreeZ);	
	void SetCameraRotation(const FVector& degree);

	void CreateMVPMatrix();
private:

private:
	FMatrix ViewMatrix;
	FVector Position;
	FVector Rotation;
};