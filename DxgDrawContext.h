// DxgDrawContext.h: GUI 렌더링 + 입력 조회 추상 인터페이스(=Mint3 MDrawContext 역할).
// 위젯/차트는 본 인터페이스만 알고 백엔드(D2D/DX11/DX12) 무관하게 동작.
// 구현체는 UI 밖, 각 렌더러 Wrapp 에 존재:
//   C_DRAW_CONTEXT_D2D  (D2DWrapp)  — 현 프로젝트.
//   C_DRAW_CONTEXT_DX11 (DX11Wrapp) — 향후.
// 좌표=float px, 색=u32 ARGB, 폰트=FontHandle(RegisterFont 반환), 문자열=wchar_t(UTF-16).
// 위젯 + 차트 프리미티브의 합집합(차트가 quad/triangle/circle 사용).
// <windows.h>/D2D/D3D 비포함 — VK_* 는 int 로 받음(DxgKeys.h).
#pragma once

#include "DxgTypes.h"

#include <cstdint>


namespace dxgui
{

	// 폰트 핸들 — RegisterFont 가 반환. -1=무효.
	using FontHandle = int32_t;
	inline constexpr FontHandle INVALID_FONT = -1;


	enum E_DXG_TEXT_ALIGN : uint8_t
	{
		DXG_TEXT_ALIGN_LEFT   = 0,
		DXG_TEXT_ALIGN_CENTER = 1,
		DXG_TEXT_ALIGN_RIGHT  = 2,
	};


	// 세로 정렬 — m_Rect.h 가 양수일 때만 의미. h<=0 이면 TOP 동작과 동일.
	enum E_DXG_VTEXT_ALIGN : uint8_t
	{
		DXG_VALIGN_TOP    = 0,
		DXG_VALIGN_CENTER = 1,
		DXG_VALIGN_BOTTOM = 2,
	};


	enum E_DXG_MOUSE_BUTTON : uint8_t
	{
		DXG_MOUSE_LEFT   = 0,
		DXG_MOUSE_RIGHT  = 1,
		DXG_MOUSE_MIDDLE = 2,
	};


	// 추상 DrawContext — 위젯/차트는 본 인터페이스만 참조.
	class IDrawContext
	{
	public:
		virtual ~IDrawContext() = default;

		// ── 프레임 라이프사이클 ──
		// BeginFrame/EndFrame 사이에서 DrawXxx 호출. 입력 큐(키/문자) 는 BeginFrame 에 클리어.
		virtual void BeginFrame() = 0;
		virtual void EndFrame()   = 0;

		// ── 폰트 등록 / 측정 ──
		// _pFace: 폰트 페이스("맑은 고딕" 등). _uPxHeight: 1.0 스케일 기준 픽셀 높이.
		// 반환: FontHandle(재사용 가능). 실패 시 INVALID_FONT.
		virtual FontHandle RegisterFont(const wchar_t* _pFace,
			uint32_t _uPxHeight, bool _bBold) = 0;

		// 텍스트 픽셀 너비/높이 측정 — hit-test/정렬/캐럿용.
		virtual _DXG_SIZE MeasureText(FontHandle _hFont,
			const wchar_t* _pText, float _fScale) = 0;

		// 폰트 한 줄 높이.
		virtual float GetFontHeight(FontHandle _hFont, float _fScale) = 0;

		// ── 텍스트 렌더 ──
		virtual void DrawText(FontHandle _hFont, _DXG_POINT _pos,
			const wchar_t* _pText, _DXG_COLOR _color, float _fScale) = 0;

		// ── 도형(위젯) ──
		virtual void FillRect(_DXG_RECT _rect, _DXG_COLOR _color) = 0;
		virtual void DrawRectOutline(_DXG_RECT _rect, _DXG_COLOR _color,
			float _fThickness) = 0;
		virtual void DrawLine(_DXG_POINT _a, _DXG_POINT _b,
			_DXG_COLOR _color, float _fThickness) = 0;

		// ── 클립 영역(자식 위젯 클리핑) ──
		virtual void PushClipRect(_DXG_RECT _rect) = 0;
		virtual void PopClipRect() = 0;

		// ── 차트용 추가 프리미티브(합집합) ──
		// D2D 는 이미 보유. DX11/DX12 는 backend 에서 path/sprite 로 구현.
		virtual void FillQuad(const _DXG_POINT _pts[4], _DXG_COLOR _color) = 0;	 // 밴드/영역
		virtual void FillTriangle(const _DXG_POINT _pts[3], _DXG_COLOR _color) = 0; // 화살표/마커
		virtual void FillCircle(_DXG_POINT _c, float _fRadius, _DXG_COLOR _color) = 0;
		virtual void DrawCircle(_DXG_POINT _c, float _fRadius, _DXG_COLOR _color,
			float _fThickness) = 0;

		// ── 입력 조회(폴링) ──
		// 호스트가 매 프레임 갱신한 상태를 위젯이 조회.
		virtual _DXG_POINT GetMousePos() const = 0;
		virtual bool IsMouseHovered(_DXG_RECT _rect) const = 0;
		virtual bool IsMouseClicked(E_DXG_MOUSE_BUTTON _btn) const = 0;
		virtual bool IsMouseDown(E_DXG_MOUSE_BUTTON _btn) const = 0;
		virtual bool IsMouseReleased(E_DXG_MOUSE_BUTTON _btn) const = 0;

		// Windows VK_* 코드(DxgKeys.h). true = 이번 프레임에 키 다운.
		virtual bool IsKeyPressed(int _nVK) const = 0;

		// 이번 프레임에 들어온 텍스트 입력(IME 결과 포함). 없으면 nullptr.
		virtual const wchar_t* PollTextInput() const = 0;

		// 이번 프레임 휠 누적(노치 단위, +위/-아래). 0=없음. (그리드/리스트 스크롤)
		virtual float GetWheelDelta() const = 0;

		// IME 조합중(미확정) 문자열 — 에디트박스 인라인 프리뷰용. 없으면 nullptr.
		// 확정 문자열은 PollTextInput 으로 들어옴(호스트가 IME 결과를 큐에 push).
		virtual const wchar_t* PollComposition() const = 0;

		// 입력 캡처(모달) — 매니저가 메뉴 등 모달 오버레이 동안 켠다. 켜진 동안 마우스
		// 조회(Hovered/Clicked/Down/Released)와 휠은 차단(아래 위젯 입력 억제). GetMousePos 는 유지.
		virtual void SetInputCapture(bool _bCapture) = 0;
	};

} // namespace dxgui
