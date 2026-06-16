// DxgSplitter.h: 스플리터 위젯. 두 페이지(자식 위젯)를 분할바로 나누고 드래그로 비율 조절.
// 수직바(좌/우) 또는 수평바(상/하). 페이지=임의 위젯(주로 패널). 오버레이는 양 페이지로 전파.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"

#include <memory>


namespace dxgui
{

	class C_DXG_SPLITTER : public C_DXG_WIDGET
	{
	private:
		std::unique_ptr<C_DXG_WIDGET> m_pPaneA;
		std::unique_ptr<C_DXG_WIDGET> m_pPaneB;
		bool  m_bVertBar;	// true=수직바(좌 A / 우 B), false=수평바(상 A / 하 B)
		float m_fRatio;		// pane A 비율(0..1)
		float m_fBarThick;
		float m_fMinRatio;	// 양끝 최소 비율
		bool  m_bDrag;

		_DXG_COLOR m_BarColor, m_BarHoverColor, m_BarGripColor;

	public:
		C_DXG_SPLITTER()
			: m_bVertBar(true)
			, m_fRatio(0.5f)
			, m_fBarThick(6.0f)
			, m_fMinRatio(0.1f)
			, m_bDrag(false)
			, m_BarColor(0xFFE2E8F0u)
			, m_BarHoverColor(0xFFC4CCD6u)
			, m_BarGripColor(0xFF9AA6B4u)
		{
		}

		void SetVertical(bool _b)        { m_bVertBar = _b; }	// true=수직바(좌/우)
		void SetRatio(float _f)          { m_fRatio = _f; this->clampRatio_(); }
		void SetBarThickness(float _f)   { m_fBarThick = _f; }
		float Ratio() const { return m_fRatio; }

		C_DXG_WIDGET* SetPaneA(std::unique_ptr<C_DXG_WIDGET> _p) { C_DXG_WIDGET* r = _p.get(); m_pPaneA = std::move(_p); return r; }
		C_DXG_WIDGET* SetPaneB(std::unique_ptr<C_DXG_WIDGET> _p) { C_DXG_WIDGET* r = _p.get(); m_pPaneB = std::move(_p); return r; }
		template <class T> T* EmplaceA() { auto u = std::make_unique<T>(); T* p = u.get(); m_pPaneA = std::move(u); return p; }
		template <class T> T* EmplaceB() { auto u = std::make_unique<T>(); T* p = u.get(); m_pPaneB = std::move(u); return p; }

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_SPLITTER; }
		const char*       GetTypeName() const override { return "splitter"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
		void RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin) override;

		// 포커스 순회 — 양 페이지(A→B) 재귀 수집.
		void CollectFocusable(std::vector<C_DXG_WIDGET*>& _out) override
		{
			if (!m_bVisible) { return; }
			if (m_pPaneA) { m_pPaneA->CollectFocusable(_out); }
			if (m_pPaneB) { m_pPaneB->CollectFocusable(_out); }
		}

	private:
		void clampRatio_()
		{
			if (m_fRatio < m_fMinRatio)        { m_fRatio = m_fMinRatio; }
			if (m_fRatio > 1.0f - m_fMinRatio) { m_fRatio = 1.0f - m_fMinRatio; }
		}
		// 양 페이지 영역 + 바 영역 산출(절대). _pA/_pB/_pBar 에 채움.
		void regions_(const _DXG_RECT& _abs, _DXG_RECT& _a, _DXG_RECT& _bar, _DXG_RECT& _b) const
		{
			if (m_bVertBar)
			{
				const float fAvail = _abs.w - m_fBarThick;
				const float fAw = fAvail * m_fRatio;
				_a   = _DXG_RECT(_abs.x, _abs.y, fAw, _abs.h);
				_bar = _DXG_RECT(_abs.x + fAw, _abs.y, m_fBarThick, _abs.h);
				_b   = _DXG_RECT(_abs.x + fAw + m_fBarThick, _abs.y, fAvail - fAw, _abs.h);
			}
			else
			{
				const float fAvail = _abs.h - m_fBarThick;
				const float fAh = fAvail * m_fRatio;
				_a   = _DXG_RECT(_abs.x, _abs.y, _abs.w, fAh);
				_bar = _DXG_RECT(_abs.x, _abs.y + fAh, _abs.w, m_fBarThick);
				_b   = _DXG_RECT(_abs.x, _abs.y + fAh + m_fBarThick, _abs.w, fAvail - fAh);
			}
		}
	};

} // namespace dxgui
