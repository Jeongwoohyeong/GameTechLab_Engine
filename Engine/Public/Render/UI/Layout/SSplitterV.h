#pragma once
#include "Render/UI/Layout/SSplitter.h"

// Vertical splitter: Left(LT) / Right(RB)
class SSplitterV : public SSplitter
{
public:
	using SSplitter::SSplitter;
	void LayoutChildren() override
	{
		if (!SideLT || !SideRB) return;
		const float leftW = Rect.W * SplitRatio;
		const float rightW = Rect.W - leftW;
		SideLT->SetRect({ Rect.X, Rect.Y, leftW, Rect.H });
		SideRB->SetRect({ Rect.X + leftW, Rect.Y, rightW, Rect.H });
		SideLT->LayoutChildren();
		SideRB->LayoutChildren();
	}
};
