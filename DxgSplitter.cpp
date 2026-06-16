// DxgSplitter.cpp
#include "DxgSplitter.h"


namespace dxgui
{

	void C_DXG_SPLITTER::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		if (abs_.w <= 0.0f || abs_.h <= 0.0f) { return; }

		_DXG_RECT a_, bar_, b_;
		this->regions_(abs_, a_, bar_, b_);

		// ── 바 드래그(비율 조절) ──
		if (m_bEnabled)
		{
			const bool bHovBar = _ctx.IsMouseHovered(bar_);
			if (_ctx.IsMouseClicked(DXG_MOUSE_LEFT) && bHovBar) { m_bDrag = true; }
			if (m_bDrag && _ctx.IsMouseDown(DXG_MOUSE_LEFT))
			{
				const _DXG_POINT m = _ctx.GetMousePos();
				if (m_bVertBar)
				{
					const float fAvail = abs_.w - m_fBarThick;
					if (fAvail > 0.0f) { m_fRatio = (m.x - abs_.x - m_fBarThick * 0.5f) / fAvail; }
				}
				else
				{
					const float fAvail = abs_.h - m_fBarThick;
					if (fAvail > 0.0f) { m_fRatio = (m.y - abs_.y - m_fBarThick * 0.5f) / fAvail; }
				}
				this->clampRatio_();
				this->regions_(abs_, a_, bar_, b_);	// 비율 갱신 후 재산출
			}
			if (_ctx.IsMouseReleased(DXG_MOUSE_LEFT)) { m_bDrag = false; }
		}

		// 페이지가 영역을 채우도록 rect 갱신(드래그 리사이즈 반영). 자식은 페이지 기준 상대좌표.
		if (m_pPaneA) { m_pPaneA->SetRect(_DXG_RECT(0.0f, 0.0f, a_.w, a_.h)); }
		if (m_pPaneB) { m_pPaneB->SetRect(_DXG_RECT(0.0f, 0.0f, b_.w, b_.h)); }

		// ── 페이지 A/B 렌더(각 영역 클립) ──
		if (m_pPaneA && m_pPaneA->IsVisible() && a_.w > 0.0f && a_.h > 0.0f)
		{
			_ctx.PushClipRect(a_);
			m_pPaneA->Render(_ctx, a_.GetTopLeft());
			_ctx.PopClipRect();
		}
		if (m_pPaneB && m_pPaneB->IsVisible() && b_.w > 0.0f && b_.h > 0.0f)
		{
			_ctx.PushClipRect(b_);
			m_pPaneB->Render(_ctx, b_.GetTopLeft());
			_ctx.PopClipRect();
		}

		// ── 분할바 + 그립(중앙 점 3개) ──
		const bool bHov = m_bDrag || _ctx.IsMouseHovered(bar_);
		_ctx.FillRect(bar_, bHov ? m_BarHoverColor : m_BarColor);
		const float fCx = bar_.x + bar_.w * 0.5f;
		const float fCy = bar_.y + bar_.h * 0.5f;
		for (int k = -1; k <= 1; ++k)
		{
			if (m_bVertBar) { _ctx.FillRect(_DXG_RECT(fCx - 1.0f, fCy + static_cast<float>(k) * 5.0f - 1.0f, 2.0f, 2.0f), m_BarGripColor); }
			else            { _ctx.FillRect(_DXG_RECT(fCx + static_cast<float>(k) * 5.0f - 1.0f, fCy - 1.0f, 2.0f, 2.0f), m_BarGripColor); }
		}
	}

	void C_DXG_SPLITTER::RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }
		const _DXG_RECT abs_ = AbsRect(_origin);
		_DXG_RECT a_, bar_, b_;
		this->regions_(abs_, a_, bar_, b_);
		if (m_pPaneA && m_pPaneA->IsVisible()) { m_pPaneA->RenderOverlay(_ctx, a_.GetTopLeft()); }
		if (m_pPaneB && m_pPaneB->IsVisible()) { m_pPaneB->RenderOverlay(_ctx, b_.GetTopLeft()); }
	}

} // namespace dxgui
