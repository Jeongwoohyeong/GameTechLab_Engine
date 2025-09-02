#pragma once
#include "Vector.h"

struct FRect {
    FRect() : min(FVector3(0, 0)), max(FVector3(0, 0)) {}
    FRect(const FVector3& inMin, const FVector3& inMax) : min(inMin), max(inMax) {}
    FRect(const FVector3& center, float width, float height) 
    {
        min.x = center.x - width / 2.0f;
        min.y = center.y - height / 2.0f;
        max.x = center.x + width / 2.0f;
        max.y = center.y + height / 2.0f;
    }

    FVector3 min; // Bottom-Left
    FVector3 max; // Top-Right

    float GetWidth() const { return max.x - min.x; }
    float GetHeight() const { return max.y - min.y; }

    void SetCenter(const FVector3& newCenter)
    {
        float halfWidth = GetWidth() / 2.0f;
        float halfHeight = GetHeight() / 2.0f;

        min.x = newCenter.x - halfWidth;
        min.y = newCenter.y - halfHeight;
        max.x = newCenter.x + halfWidth;
        max.y = newCenter.y + halfHeight;
    }

    // AABB Collision Check
    bool Intersects(const FRect& other) const 
    {
        return !(min.x > other.max.x || max.x < other.min.x ||
            min.y > other.max.y || max.y < other.min.y);
    }
};
