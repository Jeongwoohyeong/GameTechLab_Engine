#include "pch.h"
#include "Box.h"

std::vector<FVector> FBox::MakeVerticesFromFBox(const FBox& Box)
{
    const FVector& Min = Box.Min;
    const FVector& Max = Box.Max;

    return {
        {Min.X, Min.Y, Min.Z},
        {Max.X, Min.Y, Min.Z},
        {Min.X, Max.Y, Min.Z},
        {Max.X, Max.Y, Min.Z},
        {Min.X, Min.Y, Max.Z},
        {Max.X, Min.Y, Max.Z},
        {Min.X, Max.Y, Max.Z},
        {Max.X, Max.Y, Max.Z}
    };
}