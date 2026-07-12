// DxgButton.cpp
#include "DxgButton.h"
#include "DxgIcons.h"


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

		// ── 아이콘 폰트 지연 등록 ──
		// RegisterFont 는 호출마다 엔트리를 추가한다(중복 제거 없음). 프레임마다 등록하면 누적되므로
		// 위젯당 1회만 등록하고, px 가 바뀔 때만 재등록한다.
		float fIconW_ = 0.0f;
		float fIconH_ = 0.0f;
		const wchar_t szIcon_[2] = { m_cIcon, L'\0' };
		if (m_cIcon != 0)
		{
			float fPx_ = m_fIconPx;
			if (fPx_ <= 0.0f)
			{
				fPx_ = (m_hFont != INVALID_FONT) ? _ctx.GetFontHeight(m_hFont, m_fFontScale) : 16.0f;
			}
			const uint32_t uPx_ = static_cast<uint32_t>(fPx_ + 0.5f);
			if (m_hIconFont == INVALID_FONT || m_uIconFontPx != uPx_)
			{
				m_hIconFont = _ctx.RegisterFont(icons::Face, uPx_, false);
				m_uIconFontPx = uPx_;
			}
			if (m_hIconFont != INVALID_FONT)
			{
				const _DXG_SIZE szi_ = _ctx.MeasureText(m_hIconFont, szIcon_, 1.0f);
				fIconW_ = szi_.w;
				fIconH_ = szi_.h;
			}
		}

		// ── 아이콘 + 텍스트 ──
		// [아이콘][4px][텍스트] 수평 그룹을 기존 정렬 규칙(m_Align/m_VAlign) 안에 배치.
		// 아이콘이 없으면 그룹 = 텍스트 단독이라 기존 배치와 바이트 동일.
		const bool bText_ = (!m_sName.empty() && m_hFont != INVALID_FONT);
		_DXG_SIZE szT_(0.0f, 0.0f);
		if (bText_) { szT_ = _ctx.MeasureText(m_hFont, m_sName.c_str(), m_fFontScale); }

		const float fGap_  = (fIconW_ > 0.0f && bText_) ? 4.0f : 0.0f;
		const float fGrpW_ = fIconW_ + fGap_ + szT_.w;

		float fGX_ = abs_.x;
		if (m_Align == DXG_TEXT_ALIGN_CENTER) { fGX_ += (abs_.w - fGrpW_) * 0.5f; }
		else if (m_Align == DXG_TEXT_ALIGN_RIGHT) { fGX_ += (abs_.w - fGrpW_); }

		if (fIconW_ > 0.0f)
		{
			float fIY_ = abs_.y;
			if (m_VAlign == DXG_VALIGN_CENTER) { fIY_ += (abs_.h - fIconH_) * 0.5f; }
			else if (m_VAlign == DXG_VALIGN_BOTTOM) { fIY_ += (abs_.h - fIconH_); }
			_ctx.DrawText(m_hIconFont, _DXG_POINT(fGX_, fIY_), szIcon_, m_IconColor, 1.0f);
		}

		if (bText_)
		{
			float fTY_ = abs_.y;
			if (m_VAlign == DXG_VALIGN_CENTER) { fTY_ += (abs_.h - szT_.h) * 0.5f; }
			else if (m_VAlign == DXG_VALIGN_BOTTOM) { fTY_ += (abs_.h - szT_.h); }
			const _DXG_COLOR col_ = bHover_ ? m_TextHoverColor : m_TextColor;
			_ctx.DrawText(m_hFont, _DXG_POINT(fGX_ + fIconW_ + fGap_, fTY_),
				m_sName.c_str(), col_, m_fFontScale);
		}

		// ── 클릭 판정 ──
		if (m_bEnabled && bHover_ && _ctx.IsMouseReleased(DXG_MOUSE_LEFT))
		{
			if (m_OnClick) { m_OnClick(); }
		}
	}

} // namespace dxgui
