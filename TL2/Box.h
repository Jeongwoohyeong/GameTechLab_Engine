#pragma once

struct FBox
{
    FVector Min;
    FVector Max;

    FBox() : Min(FVector()), Max(FVector()) {}
    FBox(const FVector& InMin, const FVector& InMax) : Min(InMin), Max(InMax) {}

    static std::vector<FVector> MakeVerticesFromFBox(const FBox& Box);
};