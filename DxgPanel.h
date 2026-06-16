// DxgPanel.h: 컨테이너 위젯. 배경/테두리 + 자식 위젯 N개 소유.
// 자신의 절대 좌상단을 원점으로 자식들을 Render(클립 적용). 좌표는 패널 기준 px.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"

#include <vector>
#include <memory>


namespace dxgui
{

	class C_DXG_PANEL : public C_DXG_WIDGET
	{
	private:
		std::vector<std::unique_ptr<C_DXG_WIDGET>> m_vChildren;
		_DXG_COLOR m_BgColor;          // alpha 0 = 배경 미채움
		_DXG_COLOR m_BorderColor;
		float      m_fBorderThickness; // 0 = 테두리 없음
		bool       m_bClipChildren;    // true = 패널 영역으로 자식 클리핑

	public:
		C_DXG_PANEL()
			: m_BgColor(0xFFF4F4F4u)
			, m_BorderColor(0xFFB0B0B0u)
			, m_fBorderThickness(0.0f)
			, m_bClipChildren(true)
		{
		}

		void SetBgColor(_DXG_COLOR _c)          { m_BgColor = _c; }
		void SetBorder(_DXG_COLOR _c, float _t) { m_BorderColor = _c; m_fBorderThickness = _t; }
		void SetClipChildren(bool _b)           { m_bClipChildren = _b; }

		// 자식 추가 — 소유권 이전. 반환 = 등록된 원시 포인터(설정/조회용, 소유 아님).
		C_DXG_WIDGET* Add(std::unique_ptr<C_DXG_WIDGET> _pChild)
		{
			C_DXG_WIDGET* p_ = _pChild.get();
			m_vChildren.push_back(std::move(_pChild));
			return p_;
		}
		// 타입 지정 생성 + 추가 헬퍼. 반환 = 생성된 위젯(소유 아님).
		template <class T>
		T* Emplace()
		{
			auto up_ = std::make_unique<T>();
			T* p_ = up_.get();
			m_vChildren.push_back(std::move(up_));
			return p_;
		}

		void Clear() { m_vChildren.clear(); }
		size_t ChildCount() const { return m_vChildren.size(); }

		// 타입
		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_PANEL; }
		const char*       GetTypeName() const override { return "panel"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
		void RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin) override;

		// 포커스 순회 — 가시 자식 전부 재귀 수집(추가 순서).
		void CollectFocusable(std::vector<C_DXG_WIDGET*>& _out) override
		{
			if (!m_bVisible) { return; }
			for (const std::unique_ptr<C_DXG_WIDGET>& pChild_ : m_vChildren)
			{
				if (pChild_) { pChild_->CollectFocusable(_out); }
			}
		}
	};

} // namespace dxgui
