// DxgTabs.cpp
#include "DxgTabs.h"


namespace dxgui
{

	void C_DXG_TABS::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const float fContentTop = abs_.y + m_fTabH;
		const _DXG_RECT contentR_(abs_.x, fContentTop, abs_.w, abs_.h - m_fTabH);

		// 콘텐츠 배경.
		_ctx.FillRect(contentR_, m_ContentBg);

		// 스트립 배경.
		_ctx.FillRect(_DXG_RECT(abs_.x, abs_.y, abs_.w, m_fTabH), m_StripBg);

		// 탭 버튼.
		const int nTabs = static_cast<int>(m_vTabs.size());
		float fTx = abs_.x;
		for (int i = 0; i < nTabs; ++i)
		{
			float fTabW = m_fMinTabW;
			if (m_hFont != INVALID_FONT && !m_vTabs[i].sLabel.empty())
			{
				const _DXG_SIZE sz = _ctx.MeasureText(m_hFont, m_vTabs[i].sLabel.c_str(), m_fFontScale);
				const float fw = sz.w + m_fTabPad * 2.0f;
				if (fw > fTabW) { fTabW = fw; }
			}
			const _DXG_RECT tabR_(fTx, abs_.y, fTabW, m_fTabH);
			const bool bActive = (i == m_nActive);
			const bool bHov = _ctx.IsMouseHovered(tabR_);

			_ctx.FillRect(tabR_, bActive ? m_TabActiveBg : (bHov ? m_TabActiveBg : m_TabBg));
			// 탭 구분 세로선.
			if (i > 0) { _ctx.DrawLine(_DXG_POINT(fTx, abs_.y + 4.0f), _DXG_POINT(fTx, abs_.y + m_fTabH - 4.0f), m_Border, 1.0f); }
			// 활성 탭 하단 강조선(콘텐츠와 연결되는 액센트).
			if (bActive)
			{
				_ctx.FillRect(_DXG_RECT(fTx, fContentTop - 2.0f, fTabW, 2.0f), m_Accent);
			}
			// 라벨(가운데).
			if (m_hFont != INVALID_FONT && !m_vTabs[i].sLabel.empty())
			{
				_ctx.PushClipRect(tabR_);
				const _DXG_SIZE sz = _ctx.MeasureText(m_hFont, m_vTabs[i].sLabel.c_str(), m_fFontScale);
				const float fLx = fTx + (fTabW - sz.w) * 0.5f;
				const float fLy = abs_.y + (m_fTabH - sz.h) * 0.5f;
				_ctx.DrawText(m_hFont, _DXG_POINT(fLx, fLy), m_vTabs[i].sLabel.c_str(),
					bActive ? m_TabActiveText : m_TabText, m_fFontScale);
				_ctx.PopClipRect();
			}

			// 탭 클릭 → 활성 전환.
			if (m_bEnabled && bHov && _ctx.IsMouseClicked(DXG_MOUSE_LEFT) && !bActive)
			{
				m_nActive = i;
				if (m_OnChange) { m_OnChange(i); }
			}
			fTx += fTabW;
		}

		// 콘텐츠 외곽 테두리.
		_ctx.DrawRectOutline(contentR_, m_Border, 1.0f);

		// 활성 페이지(콘텐츠 영역 클립).
		if (m_nActive >= 0 && m_nActive < nTabs && m_vTabs[m_nActive].pPage
			&& m_vTabs[m_nActive].pPage->IsVisible()
			&& contentR_.w > 0.0f && contentR_.h > 0.0f)
		{
			_ctx.PushClipRect(contentR_);
			m_vTabs[m_nActive].pPage->Render(_ctx, contentR_.GetTopLeft());
			_ctx.PopClipRect();
		}
	}

	void C_DXG_TABS::RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }
		const int nTabs = static_cast<int>(m_vTabs.size());
		if (m_nActive < 0 || m_nActive >= nTabs || !m_vTabs[m_nActive].pPage) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const _DXG_POINT contentTL_(abs_.x, abs_.y + m_fTabH);
		// 오버레이는 클립 없이(콤보 드롭다운 등 콘텐츠 밖으로 떠도 보이게).
		m_vTabs[m_nActive].pPage->RenderOverlay(_ctx, contentTL_);
	}

} // namespace dxgui
