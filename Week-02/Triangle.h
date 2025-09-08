#pragma once
#include "FVertexStruct.h"

static FVertexSimple GTriangleVertices[] =
{
	// -------------------- 삼각형 정점 (Vertices) --------------------
	{ 0.0f,  0.5f,  0.0f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top
	{ 0.5f, -0.5f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Right
	{ -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f } // Left
};