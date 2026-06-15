// DxgLabel.cpp
#include "DxgLabel.h"


namespace dxgui
{

	void C_DXG_LABEL::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible || m_sName.empty() || m_hFont == INVALID_FONT)
		{
			return;
		}
		const _DXG_SIZE sz_ = _ctx.MeasureText(m_hFont, m_sName.c_str(), m_fFontScale);

		float fX_ = _origin.x + m_Rect.x;
		float fY_ = _origin.y + m_Rect.y;
		if (m_Rect.w > 0.0f)
		{
			if (m_Align == DXG_TEXT_ALIGN_CENTER)
			{
				fX_ += (m_Rect.w - sz_.w) * 0.5f;
			}
			else if (m_Align == DXG_TEXT_ALIGN_RIGHT)
			{
				fX_ += (m_Rect.w - sz_.w);
			}
		}
		if (m_Rect.h > 0.0f)
		{
			if (m_VAlign == DXG_VALIGN_CENTER)
			{
				fY_ += (m_Rect.h - sz_.h) * 0.5f;
			}
			else if (m_VAlign == DXG_VALIGN_BOTTOM)
			{
				fY_ += (m_Rect.h - sz_.h);
			}
		}
		_ctx.DrawText(m_hFont, _DXG_POINT(fX_, fY_),
			m_sName.c_str(), m_Color, m_fFontScale);
	}

} // namespace dxgui
