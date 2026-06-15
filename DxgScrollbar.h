// DxgScrollbar.h: 세로 스크롤바 위젯(track + thumb, 폴링 드래그).
// 독립 사용 + 컨테이너(C_DXG_LISTGRID 등) 임베드 겸용. m_Rect = track 영역.
// 소유자가 매 프레임 SetMetrics(content,viewport) + SetRect 후 Render → GetValue 조회.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"


namespace dxgui
{

	class C_DXG_SCROLLBAR : public C_DXG_WIDGET
	{
	private:
		float m_fContentH;		// 전체 콘텐츠 높이(px)
		float m_fViewportH;		// 보이는 영역 높이(px)
		float m_fValue;			// 스크롤 오프셋(px, 0..MaxScroll)
		float m_fMinThumb;		// thumb 최소 높이(px)

		_DXG_COLOR m_TrackColor;
		_DXG_COLOR m_ThumbColor;
		_DXG_COLOR m_ThumbHoverColor;

		bool  m_bDrag;			// thumb 드래그 중
		float m_fGrabDY;		// 드래그 시작 시 thumb 상단~마우스 오프셋

	public:
		C_DXG_SCROLLBAR()
			: m_fContentH(0.0f)
			, m_fViewportH(0.0f)
			, m_fValue(0.0f)
			, m_fMinThumb(20.0f)
			, m_TrackColor(0xFFEDEFF2u)
			, m_ThumbColor(0xFFB8C2D0u)
			, m_ThumbHoverColor(0xFF8C99AAu)
			, m_bDrag(false)
			, m_fGrabDY(0.0f)
		{
		}

		void SetColors(_DXG_COLOR _track, _DXG_COLOR _thumb, _DXG_COLOR _thumbHover)
		{ m_TrackColor = _track; m_ThumbColor = _thumb; m_ThumbHoverColor = _thumbHover; }
		void SetMinThumb(float _f) { m_fMinThumb = _f; }

		// 콘텐츠/뷰포트 갱신(소유자가 매 프레임). 값은 [0,MaxScroll] 로 클램프.
		void SetMetrics(float _fContentH, float _fViewportH)
		{
			m_fContentH = _fContentH;
			m_fViewportH = _fViewportH;
			this->clampValue_();
		}

		float MaxScroll() const
		{
			const float m = m_fContentH - m_fViewportH;
			return (m > 0.0f) ? m : 0.0f;
		}
		bool  Needed() const { return m_fContentH > m_fViewportH + 0.5f; }

		float GetValue() const { return m_fValue; }
		void  SetValue(float _v) { m_fValue = _v; this->clampValue_(); }
		void  ScrollBy(float _dPx) { this->SetValue(m_fValue + _dPx); }

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_SCROLLBAR; }
		const char*       GetTypeName() const override { return "scrollbar"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;

	private:
		void clampValue_()
		{
			const float maxS = this->MaxScroll();
			if (m_fValue < 0.0f)  { m_fValue = 0.0f; }
			if (m_fValue > maxS)  { m_fValue = maxS; }
		}
	};

} // namespace dxgui
