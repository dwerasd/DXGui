// DxgKeys.h: 위젯이 IsKeyPressed(int) 에 넘기는 가상키 상수.
// <Windows.h> 비포함(코어 컴파일 격리) — 값은 Windows VK_* 와 동일(호스트가 VK 그대로 공급).
#pragma once


namespace dxgui
{

	inline constexpr int DXG_VK_BACK   = 0x08;	// VK_BACK
	inline constexpr int DXG_VK_TAB    = 0x09;	// VK_TAB
	inline constexpr int DXG_VK_RETURN = 0x0D;	// VK_RETURN

} // namespace dxgui
