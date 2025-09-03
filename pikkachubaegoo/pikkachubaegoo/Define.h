#pragma once

#include <windows.h>
#define ATLASSPRITE_WIDTH 476
#define ATLASSPRITE_HEIGHT 885

inline void LogDebug(const wchar_t* format, ...)
{
	wchar_t buffer[1024];
	va_list args;
	va_start(args, format);
	vswprintf_s(buffer, 1024, format, args);
	va_end(args);

	OutputDebugString(buffer);
}
constexpr float GROUND_LEVEL = -0.9f;
constexpr int MAX_SCORE = 2;

// BALL
constexpr float BALL_MIN_VELOCITY_X = 0.4f;
constexpr float BALL_MIN_VELOCITY_Y = 2.0f;
constexpr float BALL_SPIKE_VELOCITY_X = 1.2f;
constexpr float BALL_SPIKE_VELOCITY_Y = 3.0f;
constexpr float BALL_ROTATE_RATIO = 360.0f;

constexpr float BALL_BASE_POSITION_X = 0.0f;
constexpr float BALL_BASE_POSITION_Y = 0.9f;


// PLAYER
constexpr float PLAYER_BASE_POSITION_X = 0.7f;
constexpr float PLAYER_BASE_POSITION_Y = -0.8f;
constexpr int PLAYER1_INDEX = 0;
constexpr int PLAYER2_INDEX = 1;


// WALL
constexpr float WALL_BASE_POSITION_X = 0.0f;
constexpr float WALL_BASE_POSITION_Y = -0.6f;
constexpr float WALL_SCALE_X = 0.05f;
constexpr float WALL_SCALE_Y = 0.35f;

