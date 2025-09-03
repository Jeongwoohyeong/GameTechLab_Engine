#include "Animator.h"
#include "Animation.h"
#include "SpriteSheet.h"

UAnimator::UAnimator()
{
	animList = nullptr;
	spriteSheet = nullptr;
	currentState = PlayerState::Idle;	
	currentFrameIndex = 0;
	elapsedTime = 0.0f;
	fps = 10.0f;
}

UAnimator::~UAnimator()
{
}

void UAnimator::Initialize(UAnimation* list, USpriteSheet* sheet)
{
	animList = list;
	spriteSheet = sheet;
	SetState(PlayerState::Idle);
}

void UAnimator::Update(float deltaTime)
{
	if (currentFrame.empty())
	{
		return;
	}

	elapsedTime += deltaTime;
	float timePerFrame = 1.0f / fps;

	int size = currentFrame.size();
	while (elapsedTime >= timePerFrame)
	{
		elapsedTime -= timePerFrame;
		currentFrameIndex = (currentFrameIndex + 1) % size;
	}

	return;
}

void UAnimator::SetState(PlayerState newState)
{
	if (currentState == newState)
	{
		return;
	}

	currentState = newState;

	std::string animName = "pikachu/pikachu_0";

	/*
		0 : walking
		1 : jump
		2 : spiking
		3 : sliding
		5 : 승리포즈
		6 : stun
	*/
	switch (newState)
	{
		case PlayerState::Idle:
			animName = "pikachu/pikachu_0";
			break;
		case PlayerState::Walking:
			animName = "pikachu/pikachu_0";
			break;
		case PlayerState::Jumping:
			animName = "pikachu/pikachu_1";
			break;
		case PlayerState::Sliding:
			animName = "pikachu/pikachu_3";
			break;
		case PlayerState::Stunned:
			animName = "pikachu/pikachu_6";
			break;
		case PlayerState::UpSpiking:
		case PlayerState::DownSpiking:
		case PlayerState::Spiking:
			animName = "pikachu/pikachu_2";
			break;
		default:
			break;
	}

	const std::vector<std::string>* clipList = animList->GetClipList(animName);
	if (!clipList)
	{
		return;
	}

	currentFrame.clear();

	for (const std::string& name : *clipList)
	{		
		const FSpriteFrame* frame = spriteSheet->GetFrame(name);
		if (frame)
		{
			currentFrame.push_back(frame);
		}
	}

	currentFrameIndex = 0;
}

const FSpriteFrame* UAnimator::GetCurrentFrame() const
{
	if (currentFrame.empty())
	{
		return nullptr;
	}

	return currentFrame[currentFrameIndex];
}