#pragma once
#include "Render/UI/Layout/SSplitter.h"

// Horizontal splitter: Top(LT) / Bottom(RB)
class SSplitterH : public SSplitter
{
public:
	using SSplitter::SSplitter;
	void LayoutChildren() override
	{
		if (!SideLT || !SideRB) return;
		const float topH = Rect.H * SplitRatio;
		const float bottomH = Rect.H - topH;
		SideLT->SetRect({ Rect.X, Rect.Y, Rect.W, topH });
		SideRB->SetRect({ Rect.X, Rect.Y + topH, Rect.W, bottomH });
		SideLT->LayoutChildren();
		SideRB->LayoutChildren();
	}
};
