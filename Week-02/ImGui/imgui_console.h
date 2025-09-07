#pragma once

namespace ImGui
{
	void ShowExampleAppConsoleWindow(bool* p_open);
	void ExampleConsoleAddLog(const char* fmt, ...);
}

// ImGui의 콘솔 로그
#define UE_LOG(...)    ImGui::ExampleConsoleAddLog(__VA_ARGS__)
