// DxgPanel.cpp
#include "DxgPanel.h"


namespace dxgui
{

	void C_DXG_PANEL::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);

		// 배경(alpha>0 일 때만) + 테두리.
		if (m_BgColor.A() > 0)
		{
			_ctx.FillRect(abs_, m_BgColor);
		}
		if (m_fBorderThickness > 0.0f)
		{
			_ctx.DrawRectOutline(abs_, m_BorderColor, m_fBorderThickness);
		}

		// 자식 — 패널 좌상단을 원점으로 렌더. 클립 옵션 시 패널 영역으로 제한.
		const bool bClip_ = m_bClipChildren && abs_.w > 0.0f && abs_.h > 0.0f;
		if (bClip_) { _ctx.PushClipRect(abs_); }
		const _DXG_POINT childOrigin_ = abs_.GetTopLeft();
		for (const std::unique_ptr<C_DXG_WIDGET>& pChild_ : m_vChildren)
		{
			if (pChild_ && pChild_->IsVisible())
			{
				pChild_->Render(_ctx, childOrigin_);
			}
		}
		if (bClip_) { _ctx.PopClipRect(); }
	}

	// 오버레이 — 자식들의 떠있는 요소(콤보 드롭다운 등)를 클립 없이 전파.
	void C_DXG_PANEL::RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }
		const _DXG_RECT abs_ = AbsRect(_origin);
		const _DXG_POINT childOrigin_ = abs_.GetTopLeft();
		for (const std::unique_ptr<C_DXG_WIDGET>& pChild_ : m_vChildren)
		{
			if (pChild_ && pChild_->IsVisible())
			{
				pChild_->RenderOverlay(_ctx, childOrigin_);
			}
		}
	}

} // namespace dxgui
