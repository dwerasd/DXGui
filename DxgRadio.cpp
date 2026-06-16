// DxgRadio.cpp
#include "DxgRadio.h"


namespace dxgui
{

	void C_DXG_RADIO::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);

		// 라벨 측정(hit 영역 자동 확장).
		float fTextW = 0.0f, fTextH = 0.0f;
		const bool bHasText = (!m_sName.empty() && m_hFont != INVALID_FONT);
		if (bHasText)
		{
			const _DXG_SIZE sz = _ctx.MeasureText(m_hFont, m_sName.c_str(), m_fFontScale);
			fTextW = sz.w; fTextH = sz.h;
		}

		const float fHitW = (abs_.w > 0.0f) ? abs_.w : (m_fSize + 6.0f + fTextW);
		const float fHitH = (abs_.h > 0.0f) ? abs_.h : (m_fSize > fTextH ? m_fSize : fTextH);
		const _DXG_RECT hit_(abs_.x, abs_.y, fHitW, fHitH);

		// 원 — hit 세로 중앙.
		const float fR  = m_fSize * 0.5f;
		const float fCx = abs_.x + fR;
		const float fCy = abs_.y + fHitH * 0.5f;

		const bool bSel = (m_pData != nullptr && *m_pData == m_nValue);
		_ctx.DrawCircle(_DXG_POINT(fCx, fCy), fR, m_RingColor, 1.5f);		// 링
		if (bSel) { _ctx.FillCircle(_DXG_POINT(fCx, fCy), fR - 4.0f, m_DotColor); }	// 선택 점

		// 라벨 — 원 우측.
		if (bHasText)
		{
			_ctx.DrawText(m_hFont, _DXG_POINT(abs_.x + m_fSize + 6.0f, abs_.y + (fHitH - fTextH) * 0.5f),
				m_sName.c_str(), m_TextColor, m_fFontScale);
		}

		// 클릭 — press/release 모두 hit 안일 때 선택.
		if (m_bEnabled && m_pData != nullptr)
		{
			const bool bHov = _ctx.IsMouseHovered(hit_);
			if (_ctx.IsMouseClicked(DXG_MOUSE_LEFT) && bHov) { m_bPressedInside = true; }
			if (_ctx.IsMouseReleased(DXG_MOUSE_LEFT))
			{
				if (m_bPressedInside && bHov && *m_pData != m_nValue)
				{
					*m_pData = m_nValue;
					if (m_OnChange) { m_OnChange(m_nValue); }
				}
				m_bPressedInside = false;
			}
		}
	}

} // namespace dxgui
