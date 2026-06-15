// DxgScrollbar.cpp
#include "DxgScrollbar.h"


namespace dxgui
{

	void C_DXG_SCROLLBAR::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const float fTrack = abs_.h;
		const float fMaxS  = this->MaxScroll();

		// thumb 높이 — 뷰포트/콘텐츠 비율. 콘텐츠가 작으면 track 전체.
		float fThumbH = this->Needed()
			? fTrack * (m_fViewportH / m_fContentH)
			: fTrack;
		if (fThumbH < m_fMinThumb) { fThumbH = m_fMinThumb; }
		if (fThumbH > fTrack)      { fThumbH = fTrack; }
		const float fDenom = fTrack - fThumbH;	// thumb 이동 가능 범위(px)

		// ── 입력(드래그 / 트랙 클릭) ── 콘텐츠가 넘칠 때만.
		if (m_bEnabled && this->Needed())
		{
			float fThumbY = abs_.y + ((fMaxS > 0.0f && fDenom > 0.0f) ? (m_fValue / fMaxS) * fDenom : 0.0f);
			const _DXG_RECT thumbR_(abs_.x, fThumbY, abs_.w, fThumbH);
			const bool bHovThumb = _ctx.IsMouseHovered(thumbR_);

			if (_ctx.IsMouseClicked(DXG_MOUSE_LEFT))
			{
				if (bHovThumb)
				{
					m_bDrag = true;
					m_fGrabDY = _ctx.GetMousePos().y - fThumbY;
				}
				else if (_ctx.IsMouseHovered(abs_))
				{
					// 트랙 클릭 — 클릭 지점이 thumb 중앙이 되도록 점프 + 이후 드래그 추종.
					m_bDrag = true;
					m_fGrabDY = fThumbH * 0.5f;
					const float fTy = _ctx.GetMousePos().y - m_fGrabDY;
					this->SetValue((fDenom > 0.0f) ? ((fTy - abs_.y) / fDenom) * fMaxS : 0.0f);
				}
			}
			if (m_bDrag && _ctx.IsMouseDown(DXG_MOUSE_LEFT))
			{
				const float fTy = _ctx.GetMousePos().y - m_fGrabDY;
				this->SetValue((fDenom > 0.0f) ? ((fTy - abs_.y) / fDenom) * fMaxS : 0.0f);
			}
			if (_ctx.IsMouseReleased(DXG_MOUSE_LEFT)) { m_bDrag = false; }
		}

		// ── 렌더(값 확정 후 thumb 재계산) ──
		const float fThumbY2 = abs_.y + ((fMaxS > 0.0f && fDenom > 0.0f) ? (m_fValue / fMaxS) * fDenom : 0.0f);
		const _DXG_RECT thumbR2_(abs_.x, fThumbY2, abs_.w, fThumbH);
		const bool bHov2 = _ctx.IsMouseHovered(thumbR2_);

		_ctx.FillRect(abs_, m_TrackColor);
		_ctx.FillRect(thumbR2_, (m_bDrag || bHov2) ? m_ThumbHoverColor : m_ThumbColor);
	}

} // namespace dxgui
