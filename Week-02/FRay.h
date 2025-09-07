#pragma once
#include "Math.h"

class FRay
{
public:
    FVector Origin;
	FVector Direction;

    FRay(const FVector& _Origin, const FVector& _Direction) : Origin(_Origin), Direction(_Direction) {}

    inline FVector At(double T) const
    {
        return Origin + T * Direction;
    }
};
