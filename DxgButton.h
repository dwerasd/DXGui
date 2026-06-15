// DxgButton.h: 버튼 위젯.
// 두 스타일: NORMAL(파란 사각형 배경) / MENU_TEXT(투명 배경, hover 시 텍스트 색만 변경).
// onClick = std::function<void()> 콜백 등록.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"

#include <functional>


namespace dxgui
{

	enum E_DXG_BUTTON_STYLE : uint8_t
	{
		DXG_BTN_STYLE_NORMAL    = 0,  // 사각형 + 텍스트 (일반 버튼)
		DXG_BTN_STYLE_MENU_TEXT = 1,  // 텍스트만, hover 시 색상 변경 (메뉴 항목 스타일)
	};


	class C_DXG_BUTTON : public C_DXG_WIDGET
	{
	private:
		E_DXG_BUTTON_STYLE m_Style;
		E_DXG_TEXT_ALIGN   m_Align;               // 가로 정렬 — MENU_TEXT 기본 LEFT, NORMAL 기본 CENTER
		E_DXG_VTEXT_ALIGN  m_VAlign;              // 세로 정렬 — 기본 CENTER
		FontHandle         m_hFont;
		_DXG_COLOR         m_TextColor;
		_DXG_COLOR         m_TextHoverColor;
		_DXG_COLOR         m_BgColor;             // NORMAL 스타일 배경
		_DXG_COLOR         m_BgHoverColor;
		_DXG_COLOR         m_BgPressedColor;
		_DXG_COLOR         m_BorderColor;
		float              m_fBorderThickness;
		std::function<void()> m_OnClick;

	public:
		C_DXG_BUTTON()
			: m_Style(DXG_BTN_STYLE_NORMAL)
			, m_Align(DXG_TEXT_ALIGN_CENTER)
			, m_VAlign(DXG_VALIGN_CENTER)
			, m_hFont(INVALID_FONT)
			, m_TextColor(0xFFFFFFFFu)
			, m_TextHoverColor(0xFFFFFFFFu)
			, m_BgColor(0xFF236EE0u)
			, m_BgHoverColor(0xFF378BFFu)
			, m_BgPressedColor(0xFF1A55B0u)
			, m_BorderColor(0xFF1A55B0u)
			, m_fBorderThickness(0.0f)
		{
		}

		// 설정
		void SetStyle(E_DXG_BUTTON_STYLE _s)
		{
			m_Style = _s;
			// MENU_TEXT 는 좌측 정렬 기본 — Set 직후 SetAlign 호출 시 그 값 우선.
			m_Align = (_s == DXG_BTN_STYLE_MENU_TEXT) ? DXG_TEXT_ALIGN_LEFT : DXG_TEXT_ALIGN_CENTER;
		}
		void SetAlign(E_DXG_TEXT_ALIGN _a)          { m_Align = _a; }
		void SetVAlign(E_DXG_VTEXT_ALIGN _a)        { m_VAlign = _a; }
		void SetFont(FontHandle _h)                 { m_hFont = _h; }
		void SetTextColor(_DXG_COLOR _c)            { m_TextColor = _c; }
		void SetTextHoverColor(_DXG_COLOR _c)       { m_TextHoverColor = _c; }
		void SetBgColor(_DXG_COLOR _c)              { m_BgColor = _c; }
		void SetBgHoverColor(_DXG_COLOR _c)         { m_BgHoverColor = _c; }
		void SetBgPressedColor(_DXG_COLOR _c)       { m_BgPressedColor = _c; }
		void SetBorder(_DXG_COLOR _c, float _t)     { m_BorderColor = _c; m_fBorderThickness = _t; }
		void SetOnClick(std::function<void()> _fn)  { m_OnClick = std::move(_fn); }

		E_DXG_BUTTON_STYLE GetStyle()  const        { return m_Style; }
		E_DXG_TEXT_ALIGN   GetAlign()  const        { return m_Align; }
		E_DXG_VTEXT_ALIGN  GetVAlign() const        { return m_VAlign; }
		FontHandle         GetFont()   const        { return m_hFont; }

		// 타입
		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_BUTTON; }
		const char*       GetTypeName() const override { return "button"; }

		// 렌더 + 입력 처리 — hover/press/click 판정 후 onClick 호출.
		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
	};

} // namespace dxgui
