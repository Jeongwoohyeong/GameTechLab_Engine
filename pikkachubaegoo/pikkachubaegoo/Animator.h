#pragma once
#include "SpriteStruct.h"
#include "PlayerState.h"
#include <vector>
#include <string>

class UAnimation;
class USpriteSheet;

class UAnimator
{
public:
	UAnimator();
	~UAnimator();

	void Initialize(UAnimation* list, USpriteSheet* sheet);
	void Update(float deltaTime);
	void SetState(PlayerState newState);
	const FSpriteFrame* GetCurrentFrame() const;
private:
	UAnimation* animList;
	USpriteSheet* spriteSheet;
	PlayerState currentState;
	std::vector<const FSpriteFrame*> currentFrame;
	int currentFrameIndex;
	float elapsedTime;
	float fps;
};