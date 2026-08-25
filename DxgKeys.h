// DxgKeys.h: 위젯이 IsKeyPressed(int) 에 넘기는 가상키 상수.
// <Windows.h> 비포함(코어 컴파일 격리) — 값은 Windows VK_* 와 동일(호스트가 VK 그대로 공급).
#pragma once


namespace dxgui
{

	inline constexpr int DXG_VK_SHIFT   = 0x10;	// VK_SHIFT
	inline constexpr int DXG_VK_CONTROL = 0x11;	// VK_CONTROL
	inline constexpr int DXG_VK_BACK   = 0x08;	// VK_BACK
	inline constexpr int DXG_VK_TAB    = 0x09;	// VK_TAB
	inline constexpr int DXG_VK_RETURN = 0x0D;	// VK_RETURN
	inline constexpr int DXG_VK_LEFT   = 0x25;	// VK_LEFT
	inline constexpr int DXG_VK_RIGHT  = 0x27;	// VK_RIGHT
	inline constexpr int DXG_VK_DELETE = 0x2E;	// VK_DELETE
	inline constexpr int DXG_VK_HOME   = 0x24;	// VK_HOME
	inline constexpr int DXG_VK_END    = 0x23;	// VK_END
	inline constexpr int DXG_VK_UP     = 0x26;	// VK_UP
	inline constexpr int DXG_VK_DOWN   = 0x28;	// VK_DOWN
	inline constexpr int DXG_VK_ESCAPE = 0x1B;	// VK_ESCAPE

} // namespace dxgui
