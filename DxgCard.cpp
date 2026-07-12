// DxgCard.cpp
#include "DxgCard.h"


namespace dxgui
{

	void C_DXG_CARD::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const float fR_ = Theme().radiusCard;

		_ctx.FillRoundRect(abs_, fR_, m_CardBg);
		_ctx.DrawRoundRectOutline(abs_, fR_, m_CardBorder, 1.0f);

		if (!m_sTitle.empty() && m_hTitleFont != INVALID_FONT)
		{
			const _DXG_SIZE sz_ = _ctx.MeasureText(m_hTitleFont, m_sTitle.c_str(), 1.0f);
			_ctx.DrawText(m_hTitleFont,
				_DXG_POINT(abs_.x + Theme().pad, abs_.y + (m_fTitleH - sz_.h) * 0.5f),
				m_sTitle.c_str(), m_TitleColor, 1.0f);
			_ctx.DrawLine(_DXG_POINT(abs_.x + 1.0f, abs_.y + m_fTitleH),
				_DXG_POINT(abs_.x + abs_.w - 1.0f, abs_.y + m_fTitleH), Theme().divider, 1.0f);
		}

		// 자식 렌더/클리핑은 베이스 위임(패널 배경/테두리는 ctor 에서 비활성).
		C_DXG_PANEL::Render(_ctx, _origin);
	}

} // namespace dxgui
