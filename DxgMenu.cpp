// DxgMenu.cpp
#include "DxgMenu.h"


namespace dxgui
{

	void C_DXG_MENU::RenderOverlay(IDrawContext& _ctx, _DXG_POINT /*_origin*/)
	{
		if (!m_bOpen || m_vItems.empty()) { return; }

		// 폭 산출(항목 텍스트 최대 + 패딩) / 높이 산출.
		float fW = m_fMinWidth;
		if (m_hFont != INVALID_FONT)
		{
			for (const _MENU_ITEM& it : m_vItems)
			{
				if (it.bSeparator) { continue; }
				const _DXG_SIZE sz = _ctx.MeasureText(m_hFont, it.sLabel.c_str(), m_fFontScale);
				const float fw = sz.w + m_fCellPad * 2.0f;
				if (fw > fW) { fW = fw; }
			}
		}
		float fH = 0.0f;
		for (const _MENU_ITEM& it : m_vItems) { fH += it.bSeparator ? m_fSepH : m_fItemH; }

		const _DXG_RECT box_(m_fPosX, m_fPosY, fW, fH);
		_ctx.FillRect(box_, m_BgColor);
		_ctx.DrawRectOutline(box_, m_BorderColor, 1.0f);

		// 항목 렌더 + 입력.
		const bool bClick = m_bEnabled && _ctx.IsMouseClicked(DXG_MOUSE_LEFT);
		float fY = m_fPosY;
		for (size_t i = 0; i < m_vItems.size(); ++i)
		{
			const _MENU_ITEM& it = m_vItems[i];
			if (it.bSeparator)
			{
				const float fSy = fY + m_fSepH * 0.5f;
				_ctx.DrawLine(_DXG_POINT(m_fPosX + 4.0f, fSy), _DXG_POINT(m_fPosX + fW - 4.0f, fSy), m_SepColor, 1.0f);
				fY += m_fSepH;
				continue;
			}

			const _DXG_RECT itemR_(m_fPosX, fY, fW, m_fItemH);
			const bool bHov = it.bEnabled && _ctx.IsMouseHovered(itemR_);
			if (bHov) { _ctx.FillRect(itemR_, m_HoverBg); }

			if (m_hFont != INVALID_FONT && !it.sLabel.empty())
			{
				_ctx.PushClipRect(itemR_);
				const _DXG_SIZE sz = _ctx.MeasureText(m_hFont, it.sLabel.c_str(), m_fFontScale);
				const float fTy = fY + (m_fItemH - sz.h) * 0.5f;
				_ctx.DrawText(m_hFont, _DXG_POINT(m_fPosX + m_fCellPad, fTy),
					it.sLabel.c_str(), it.bEnabled ? m_TextColor : m_TextDisabled, m_fFontScale);
				_ctx.PopClipRect();
			}

			if (bClick && bHov)
			{
				std::function<void()> fn = it.onClick;	// Close 전 복사(재진입 안전)
				m_bOpen = false;
				if (fn) { fn(); }
				return;
			}
			fY += m_fItemH;
		}

		// 메뉴 밖 클릭 → 닫기(메뉴 안 비-항목 클릭은 유지).
		if (bClick && !_ctx.IsMouseHovered(box_)) { m_bOpen = false; }
	}

} // namespace dxgui
