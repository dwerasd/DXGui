// DxgBadge.cpp
#include "DxgBadge.h"


namespace dxgui
{

	void C_DXG_BADGE::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const _DXG_COLOR col_ = BadgeColor(m_Kind);

		// pill = 반경 h/2 (FillRoundRect 가 min(w,h)/2 로 클램프하므로 그대로 전달).
		_ctx.FillRoundRect(abs_, abs_.h * 0.5f, ThemeTint(col_));

		if (!m_sName.empty() && m_hFont != INVALID_FONT)
		{
			const _DXG_SIZE sz_ = _ctx.MeasureText(m_hFont, m_sName.c_str(), m_fFontScale);
			_ctx.DrawText(m_hFont,
				_DXG_POINT(abs_.x + (abs_.w - sz_.w) * 0.5f, abs_.y + (abs_.h - sz_.h) * 0.5f),
				m_sName.c_str(), col_, m_fFontScale);
		}
	}

} // namespace dxgui
