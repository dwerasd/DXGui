// DxgScrollbar.cpp
#include "DxgScrollbar.h"


namespace dxgui
{

	void C_DXG_SCROLLBAR::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const bool  bV     = (m_eOrient == DXG_ORIENT_VERT);
		const float fTrack = bV ? abs_.h : abs_.w;	// 축 방향 트랙 길이
		const float fBase  = bV ? abs_.y : abs_.x;	// 축 방향 시작 좌표
		const float fMaxS  = this->MaxScroll();

		// thumb 길이 — 뷰포트/콘텐츠 비율. 콘텐츠가 작으면 track 전체.
		float fThumb = this->Needed() ? fTrack * (m_fViewportH / m_fContentH) : fTrack;
		if (fThumb < m_fMinThumb) { fThumb = m_fMinThumb; }
		if (fThumb > fTrack)      { fThumb = fTrack; }
		const float fDenom = fTrack - fThumb;	// thumb 이동 가능 범위(px)

		// 축 방향 마우스 좌표.
		const float fMouse = bV ? _ctx.GetMousePos().y : _ctx.GetMousePos().x;

		// thumb 사각형(축 방향 위치 + 교차축 전체).
		const auto thumbRect = [&](float _start) -> _DXG_RECT
		{
			return bV ? _DXG_RECT(abs_.x, _start, abs_.w, fThumb)
			          : _DXG_RECT(_start, abs_.y, fThumb, abs_.h);
		};

		// ── 입력(드래그 / 트랙 클릭) ── 콘텐츠가 넘칠 때만.
		if (m_bEnabled && this->Needed())
		{
			const float fStart = fBase + ((fMaxS > 0.0f && fDenom > 0.0f) ? (m_fValue / fMaxS) * fDenom : 0.0f);
			const bool bHovThumb = _ctx.IsMouseHovered(thumbRect(fStart));

			if (_ctx.IsMouseClicked(DXG_MOUSE_LEFT))
			{
				if (bHovThumb)
				{
					m_bDrag = true;
					m_fGrabDY = fMouse - fStart;
				}
				else if (_ctx.IsMouseHovered(abs_))
				{
					// 트랙 클릭 — 클릭 지점이 thumb 중앙이 되도록 점프 + 이후 드래그 추종.
					m_bDrag = true;
					m_fGrabDY = fThumb * 0.5f;
					const float fT = fMouse - m_fGrabDY;
					this->SetValue((fDenom > 0.0f) ? ((fT - fBase) / fDenom) * fMaxS : 0.0f);
				}
			}
			if (m_bDrag && _ctx.IsMouseDown(DXG_MOUSE_LEFT))
			{
				const float fT = fMouse - m_fGrabDY;
				this->SetValue((fDenom > 0.0f) ? ((fT - fBase) / fDenom) * fMaxS : 0.0f);
			}
			if (_ctx.IsMouseReleased(DXG_MOUSE_LEFT)) { m_bDrag = false; }
		}

		// ── 렌더(값 확정 후 thumb 재계산) ──
		const float fStart2 = fBase + ((fMaxS > 0.0f && fDenom > 0.0f) ? (m_fValue / fMaxS) * fDenom : 0.0f);
		const _DXG_RECT thumbR2_ = thumbRect(fStart2);
		const bool bHov2 = _ctx.IsMouseHovered(thumbR2_);

		_ctx.FillRect(abs_, m_TrackColor);
		_ctx.FillRect(thumbR2_, (m_bDrag || bHov2) ? m_ThumbHoverColor : m_ThumbColor);
	}

} // namespace dxgui
