// DxgManager.h: 위젯 트리 루트. 최상위 위젯(주로 패널) N개 소유 + 페인트 walk.
// 호스트가 매 프레임: ctx 바인드 → manager.Render(ctx). 입력은 ctx 폴링(위젯이 조회).
// 포커스/캡처는 P2(주문창/Edit) 에서 확장 — P0 는 루트 walk 만.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"

#include <vector>
#include <memory>


namespace dxgui
{

	class C_DXG_MANAGER
	{
	private:
		std::vector<std::unique_ptr<C_DXG_WIDGET>> m_vRoots;

	public:
		C_DXG_MANAGER() = default;

		C_DXG_MANAGER(const C_DXG_MANAGER&) = delete;
		C_DXG_MANAGER& operator=(const C_DXG_MANAGER&) = delete;

		// 최상위 위젯 추가 — 소유권 이전. 반환 = 원시 포인터(소유 아님).
		C_DXG_WIDGET* Add(std::unique_ptr<C_DXG_WIDGET> _pRoot)
		{
			C_DXG_WIDGET* p_ = _pRoot.get();
			m_vRoots.push_back(std::move(_pRoot));
			return p_;
		}
		template <class T>
		T* Emplace()
		{
			auto up_ = std::make_unique<T>();
			T* p_ = up_.get();
			m_vRoots.push_back(std::move(up_));
			return p_;
		}

		void Clear() { m_vRoots.clear(); }
		size_t RootCount() const { return m_vRoots.size(); }

		// 전 루트를 화면 원점(0,0) 기준으로 렌더(가시 위젯만).
		// 2패스: (1) 일반 Render → (2) RenderOverlay(콤보 드롭다운 등 최상위 요소).
		void Render(IDrawContext& _ctx)
		{
			const _DXG_POINT origin_(0.0f, 0.0f);
			for (const std::unique_ptr<C_DXG_WIDGET>& pRoot_ : m_vRoots)
			{
				if (pRoot_ && pRoot_->IsVisible()) { pRoot_->Render(_ctx, origin_); }
			}
			for (const std::unique_ptr<C_DXG_WIDGET>& pRoot_ : m_vRoots)
			{
				if (pRoot_ && pRoot_->IsVisible()) { pRoot_->RenderOverlay(_ctx, origin_); }
			}
		}
	};

} // namespace dxgui
