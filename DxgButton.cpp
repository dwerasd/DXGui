// DxgButton.cpp
#include "DxgButton.h"


namespace dxgui
{

	void C_DXG_BUTTON::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const bool bHover_ = m_bEnabled && _ctx.IsMouseHovered(abs_);
		const bool bDown_  = bHover_ && _ctx.IsMouseDown(DXG_MOUSE_LEFT);

		// ── 배경 (NORMAL 스타일만) ──
		if (m_Style == DXG_BTN_STYLE_NORMAL)
		{
			const _DXG_COLOR bg_ = !m_bEnabled ? _DXG_COLOR(0xFF888888u)
				: (bDown_  ? m_BgPressedColor
				: (bHover_ ? m_BgHoverColor : m_BgColor));
			_ctx.FillRect(abs_, bg_);
			if (m_fBorderThickness > 0.0f)
			{
				_ctx.DrawRectOutline(abs_, m_BorderColor, m_fBorderThickness);
			}
		}

		// ── 텍스트 ──
		// 가로/세로 정렬은 m_Align / m_VAlign 으로 일반화.
		// (스타일별 default: NORMAL=CENTER/CENTER, MENU_TEXT=LEFT/CENTER — SetStyle 에서 세팅.)
		if (!m_sName.empty() && m_hFont != INVALID_FONT)
		{
			const _DXG_SIZE sz_ = _ctx.MeasureText(m_hFont, m_sName.c_str(), m_fFontScale);
			float fTX_ = abs_.x;
			if (m_Align == DXG_TEXT_ALIGN_CENTER) { fTX_ += (abs_.w - sz_.w) * 0.5f; }
			else if (m_Align == DXG_TEXT_ALIGN_RIGHT) { fTX_ += (abs_.w - sz_.w); }
			float fTY_ = abs_.y;
			if (m_VAlign == DXG_VALIGN_CENTER) { fTY_ += (abs_.h - sz_.h) * 0.5f; }
			else if (m_VAlign == DXG_VALIGN_BOTTOM) { fTY_ += (abs_.h - sz_.h); }
			const _DXG_COLOR col_ = bHover_ ? m_TextHoverColor : m_TextColor;
			_ctx.DrawText(m_hFont, _DXG_POINT(fTX_, fTY_),
				m_sName.c_str(), col_, m_fFontScale);
		}

		// ── 클릭 판정 ──
		if (m_bEnabled && bHover_ && _ctx.IsMouseReleased(DXG_MOUSE_LEFT))
		{
			if (m_OnClick) { m_OnClick(); }
		}
	}

} // namespace dxgui
