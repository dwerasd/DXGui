// DxgManager.h: 위젯 트리 루트. 최상위 위젯(주로 패널) N개 소유 + 페인트 walk.
// 호스트가 매 프레임: ctx 바인드 → manager.Render(ctx). 입력은 ctx 폴링(위젯이 조회).
// 포커스/캡처는 P2(주문창/Edit) 에서 확장 — P0 는 루트 walk 만.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"
#include "DxgKeys.h"
#include "DxgComboBox.h"	// CloseAllPopups — 루트 콤보 드롭다운 강제 닫기

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

		// 열린 팝업(루트 콤보 드롭다운) 강제 닫기 — 호스트가 창 비활성(포그라운드 상실) 시 호출.
		// 모달 감지와 동일하게 루트 한정(현 프레임워크 규약). 가상함수 미추가(vtable/ABI 불변).
		void CloseAllPopups()
		{
			for (const std::unique_ptr<C_DXG_WIDGET>& pRoot_ : m_vRoots)
			{
				if (pRoot_ && pRoot_->GetType() == DXG_WIDGET_COMBOBOX)
				{
					static_cast<C_DXG_COMBOBOX*>(pRoot_.get())->Close();
				}
			}
		}

		// 디자인모드 — 키 부여된 최상위(루트) 위젯 수집. 루트는 원점(0,0) 렌더라 GetRect()=절대좌표.
		void CollectKeyedRoots(std::vector<C_DXG_WIDGET*>& _out) const
		{
			for (const std::unique_ptr<C_DXG_WIDGET>& pRoot_ : m_vRoots)
			{
				if (pRoot_ && pRoot_->IsVisible() && !pRoot_->GetKey().empty()) { _out.push_back(pRoot_.get()); }
			}
		}
		// 점(_x,_y) 아래 최상위 키 위젯(나중 추가=위 → 마지막 매치 우선).
		C_DXG_WIDGET* HitTestKeyedRoot(float _x, float _y) const
		{
			C_DXG_WIDGET* pHit_ = nullptr;
			for (const std::unique_ptr<C_DXG_WIDGET>& pRoot_ : m_vRoots)
			{
				if (pRoot_ && pRoot_->IsVisible() && !pRoot_->GetKey().empty()
					&& pRoot_->GetRect().Contains(_x, _y)) { pHit_ = pRoot_.get(); }
			}
			return pHit_;
		}

		// 재귀 — 컨테이너(패널/탭) 내부까지 내려가 (_x,_y) 아래 가장 깊은 키 위젯(설정창 우클릭/J).
		C_DXG_WIDGET* HitTestKeyedDeep(float _x, float _y) const
		{
			C_DXG_WIDGET* pHit_ = nullptr;
			const _DXG_POINT o_(0.0f, 0.0f);	// 루트는 원점(0,0) 렌더
			for (const std::unique_ptr<C_DXG_WIDGET>& pRoot_ : m_vRoots)
			{
				if (pRoot_) { C_DXG_WIDGET* h_ = pRoot_->HitTestKeyed(_x, _y, o_); if (h_ != nullptr) { pHit_ = h_; } }
			}
			return pHit_;
		}
		// 재귀 — 전 키 위젯 수집(탭 비활성 페이지 포함). 열 너비 복원 등.
		void CollectKeyedDeep(std::vector<C_DXG_WIDGET*>& _out) const
		{
			for (const std::unique_ptr<C_DXG_WIDGET>& pRoot_ : m_vRoots) { if (pRoot_) { pRoot_->CollectKeyed(_out); } }
		}

		// 전 루트를 화면 원점(0,0) 기준으로 렌더(가시 위젯만).
		// 2패스: (1) 일반 Render → (2) RenderOverlay(콤보 드롭다운 등 최상위 요소).
		// _origin: 콘텐츠 렌더 원점(스크롤 시 호스트가 (0,-scrollY) 전달). 기본 (0,0).
		void Render(IDrawContext& _ctx, _DXG_POINT _origin = _DXG_POINT(0.0f, 0.0f))
		{
			const _DXG_POINT origin_ = _origin;

			// 모달 감지(메뉴 등 열린 팝업) — pass1 동안 입력 캡처로 아래 위젯 클릭 누수 차단.
			bool bModal_ = false;
			for (const std::unique_ptr<C_DXG_WIDGET>& pRoot_ : m_vRoots)
			{
				if (pRoot_ && pRoot_->IsModalActive()) { bModal_ = true; break; }
			}

			// Tab 포커스 순회 — 모달 아닐 때만. 편집 위젯이 Tab 을 자체 소비하지 않고
			// 매니저가 소유(다음/이전 포커스 가능 위젯으로 이동). pass1 전에 수행해
			// 새로 포커스된 위젯이 이번 프레임부터 포커스 상태로 렌더되게 한다.
			if (!bModal_ && _ctx.IsKeyPressed(DXG_VK_TAB))
			{
				std::vector<C_DXG_WIDGET*> vFoc_;
				for (const std::unique_ptr<C_DXG_WIDGET>& pRoot_ : m_vRoots)
				{
					if (pRoot_ && pRoot_->IsVisible()) { pRoot_->CollectFocusable(vFoc_); }
				}
				if (!vFoc_.empty())
				{
					const int n_ = static_cast<int>(vFoc_.size());
					int nCur_ = -1;
					for (int i = 0; i < n_; ++i) { if (vFoc_[i]->IsFocused()) { nCur_ = i; break; } }
					const bool bBack_ = _ctx.IsKeyDown(DXG_VK_SHIFT);	// Shift+Tab = 역방향
					int nNext_;
					if (nCur_ < 0) { nNext_ = bBack_ ? (n_ - 1) : 0; }
					else           { nNext_ = ((nCur_ + (bBack_ ? -1 : 1)) % n_ + n_) % n_; }
					for (C_DXG_WIDGET* pW_ : vFoc_) { pW_->SetFocused(false); }	// 이전 포커스 커밋
					vFoc_[nNext_]->SetFocused(true);
				}
			}

			_ctx.SetInputCapture(bModal_);
			for (const std::unique_ptr<C_DXG_WIDGET>& pRoot_ : m_vRoots)
			{
				if (pRoot_ && pRoot_->IsVisible()) { pRoot_->Render(_ctx, origin_); }
			}
			_ctx.SetInputCapture(false);	// 오버레이(메뉴 등)는 실 입력 받음
			for (const std::unique_ptr<C_DXG_WIDGET>& pRoot_ : m_vRoots)
			{
				if (pRoot_ && pRoot_->IsVisible()) { pRoot_->RenderOverlay(_ctx, origin_); }
			}
		}
	};

} // namespace dxgui
