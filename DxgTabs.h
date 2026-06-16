// DxgTabs.h: 탭 컨트롤 위젯. 상단 탭 스트립 + 활성 페이지(자식 위젯) 1개만 렌더.
// 페이지는 임의 위젯(주로 C_DXG_PANEL). 탭 클릭으로 전환. 오버레이는 활성 페이지로 전파.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"

#include <vector>
#include <string>
#include <memory>
#include <functional>


namespace dxgui
{

	class C_DXG_TABS : public C_DXG_WIDGET
	{
	private:
		struct _TAB
		{
			std::wstring                  sLabel;
			std::unique_ptr<C_DXG_WIDGET> pPage;
		};
		std::vector<_TAB> m_vTabs;
		int        m_nActive;
		FontHandle m_hFont;
		float      m_fTabH, m_fTabPad, m_fMinTabW;

		_DXG_COLOR m_StripBg, m_TabBg, m_TabActiveBg, m_TabText, m_TabActiveText;
		_DXG_COLOR m_Accent, m_Border, m_ContentBg;

		std::function<void(int)> m_OnChange;

	public:
		C_DXG_TABS()
			: m_nActive(-1)
			, m_hFont(INVALID_FONT)
			, m_fTabH(28.0f), m_fTabPad(16.0f), m_fMinTabW(48.0f)
			, m_StripBg(0xFFEDF1F6u)
			, m_TabBg(0xFFE2E8F0u)
			, m_TabActiveBg(0xFFFFFFFFu)
			, m_TabText(0xFF5A6678u)
			, m_TabActiveText(0xFF1A55B0u)
			, m_Accent(0xFF236EE0u)
			, m_Border(0xFFB8C2D0u)
			, m_ContentBg(0xFFFFFFFFu)
		{
		}

		void SetFont(FontHandle _h)      { m_hFont = _h; }
		void SetTabHeight(float _h)      { m_fTabH = _h; }
		void SetOnChange(std::function<void(int)> _fn) { m_OnChange = std::move(_fn); }

		// 탭 추가 — 페이지 소유권 이전. 반환 = 페이지 원시 포인터(설정용, 소유 아님).
		C_DXG_WIDGET* AddTab(const std::wstring& _sLabel, std::unique_ptr<C_DXG_WIDGET> _pPage)
		{
			C_DXG_WIDGET* p_ = _pPage.get();
			m_vTabs.push_back(_TAB{ _sLabel, std::move(_pPage) });
			if (m_nActive < 0) { m_nActive = 0; }
			return p_;
		}
		// 타입 지정 페이지 생성 + 추가. 반환 = 생성 페이지(소유 아님).
		template <class T>
		T* AddTab(const std::wstring& _sLabel)
		{
			auto up_ = std::make_unique<T>();
			T* p_ = up_.get();
			this->AddTab(_sLabel, std::move(up_));
			return p_;
		}

		int    Active() const { return m_nActive; }
		void   SetActive(int _i) { if (_i >= 0 && _i < static_cast<int>(m_vTabs.size())) { m_nActive = _i; } }
		size_t TabCount() const { return m_vTabs.size(); }

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_TABS; }
		const char*       GetTypeName() const override { return "tabs"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
		void RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin) override;

		// 포커스 순회 — 활성 페이지만 수집(비활성 탭은 숨김).
		void CollectFocusable(std::vector<C_DXG_WIDGET*>& _out) override
		{
			if (!m_bVisible) { return; }
			if (m_nActive >= 0 && m_nActive < static_cast<int>(m_vTabs.size())
				&& m_vTabs[m_nActive].pPage)
			{
				m_vTabs[m_nActive].pPage->CollectFocusable(_out);
			}
		}
	};

} // namespace dxgui
