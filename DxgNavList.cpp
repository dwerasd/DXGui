// DxgNavList.cpp
#include "DxgNavList.h"
#include "DxgIcons.h"


namespace dxgui
{

	void C_DXG_NAV_LIST::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const int nCnt_ = static_cast<int>(m_vItems.size());
		if (nCnt_ <= 0) { return; }

		// 아이콘 폰트 1회 지연 등록(위젯당 1회 - RegisterFont 는 중복 제거를 하지 않음).
		const uint32_t uPx_ = 16u;
		if (m_hIconFont == INVALID_FONT || m_uIconFontPx != uPx_)
		{
			m_hIconFont = _ctx.RegisterFont(icons::Face, uPx_, false);
			m_uIconFontPx = uPx_;
		}

		int nClicked_ = -1;
		for (int i = 0; i < nCnt_; ++i)
		{
			const _DXG_RECT rc_(abs_.x + 8.0f,
				abs_.y + static_cast<float>(i) * m_fItemH,
				abs_.w - 16.0f, m_fItemH - 4.0f);

			const bool bHover_ = m_bEnabled && _ctx.IsMouseHovered(rc_);
			const bool bSel_   = (i == m_nSel);

			if (bSel_)
			{
				_ctx.FillRoundRect(rc_, Theme().radiusCtl, ThemeTint(Theme().accent));
			}
			else if (bHover_)
			{
				_ctx.FillRoundRect(rc_, Theme().radiusCtl, Theme().divider);
			}

			const _DXG_COLOR col_ = bSel_ ? Theme().accent : Theme().textSub;

			float fX_ = rc_.x + 10.0f;
			if (m_vItems[i].cGlyph != 0 && m_hIconFont != INVALID_FONT)
			{
				const wchar_t szIcon_[2] = { m_vItems[i].cGlyph, L'\0' };
				const _DXG_SIZE szi_ = _ctx.MeasureText(m_hIconFont, szIcon_, 1.0f);
				_ctx.DrawText(m_hIconFont,
					_DXG_POINT(fX_, rc_.y + (rc_.h - szi_.h) * 0.5f), szIcon_, col_, 1.0f);
				fX_ += szi_.w + 10.0f;
			}
			if (!m_vItems[i].sLabel.empty() && m_hLabel != INVALID_FONT)
			{
				const _DXG_SIZE szl_ = _ctx.MeasureText(m_hLabel, m_vItems[i].sLabel.c_str(), m_fFontScale);
				_ctx.DrawText(m_hLabel,
					_DXG_POINT(fX_, rc_.y + (rc_.h - szl_.h) * 0.5f),
					m_vItems[i].sLabel.c_str(), col_, m_fFontScale);
			}

			// 클릭 = UP 구동(DOWN 구동 시 이번 프레임에 열리는 창/팝업으로 클릭이 관통한다).
			if (m_bEnabled && bHover_ && _ctx.IsMouseReleased(DXG_MOUSE_LEFT)) { nClicked_ = i; }
		}

		if (nClicked_ >= 0)
		{
			m_nSel = nClicked_;
			if (m_OnSelect) { m_OnSelect(nClicked_); }
		}
	}

} // namespace dxgui
