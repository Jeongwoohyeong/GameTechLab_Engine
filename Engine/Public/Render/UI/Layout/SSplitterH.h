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

	bool HitTestHandle(FPoint p) const override
	{
		float y = Rect.Y + Rect.H * SplitRatio;
		return (p.Y >= y - HANDLE_THICKNESS && p.Y <= y + HANDLE_THICKNESS && p.X >= Rect.X && p.X <= Rect.X + Rect.W);
	}
	void DragTo(FPoint p) override
	{
		if (Rect.H <= 0.0f) return;
		float r = (p.Y - Rect.Y) / Rect.H;
		SetSplitRatio(r);
		LayoutChildren();
	}
};
