#include "pch.h"
#include "Render/UI/Layout/SSplitter.h"

SSplitter::~SSplitter()
{
	delete SideLT;
	delete SideRB;
}

void SSplitter::SetChildren(SWindow* LT, SWindow* RB)
{
	if (SideLT != LT) { delete SideLT; SideLT = LT; }
	if (SideRB != RB) { delete SideRB; SideRB = RB; }
	LayoutChildren();
}

void SSplitter::Tick(float dt)
{
	if (SideLT) SideLT->Tick(dt);
	if (SideRB) SideRB->Tick(dt);
}

void SSplitter::Draw()
{
	if (SideLT) SideLT->Draw();
	if (SideRB) SideRB->Draw();
}
