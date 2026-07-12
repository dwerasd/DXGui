// DxgNavList.h: 사이드바 내비 목록.
// 문법 = 세로 항목 목록([아이콘][라벨]), 선택 항목만 [accent 15% 틴트 라운드 pill + accent 라벨],
// hover 는 divider 톤 pill, 평시는 textSub 라벨. 배지와 동일한 틴트 어휘라 토큰 재사용으로 성립.
// 클릭은 UP(IsMouseReleased) 구동 - DOWN 구동 시 팝업/창 생성 후 아래 위젯으로 클릭 관통.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"
#include "DxgTheme.h"

#include <string>
#include <vector>
#include <functional>


namespace dxgui
{

	class C_DXG_NAV_LIST : public C_DXG_WIDGET
	{
	private:
		struct _NAV_ITEM
		{
			wchar_t      cGlyph;
			std::wstring sLabel;
		};

		std::vector<_NAV_ITEM> m_vItems;
		int        m_nSel;
		float      m_fItemH;
		FontHandle m_hLabel;
		FontHandle m_hIconFont;
		uint32_t   m_uIconFontPx;
		std::function<void(int)> m_OnSelect;

	public:
		C_DXG_NAV_LIST()
			: m_nSel(-1)
			, m_fItemH(40.0f)
			, m_hLabel(INVALID_FONT)
			, m_hIconFont(INVALID_FONT)
			, m_uIconFontPx(0u)
		{
		}

		// 반환 = 항목 인덱스.
		int AddItem(wchar_t _cGlyph, const std::wstring& _sLabel)
		{
			m_vItems.push_back(_NAV_ITEM{ _cGlyph, _sLabel });
			return static_cast<int>(m_vItems.size()) - 1;
		}
		void ClearItems()             { m_vItems.clear(); m_nSel = -1; }
		void SetSelected(int _nIdx)   { m_nSel = _nIdx; }	// -1 = 선택 없음
		int  Selected() const         { return m_nSel; }
		void SetItemHeight(float _h)  { m_fItemH = _h; }
		void SetFonts(FontHandle _hLabel) { m_hLabel = _hLabel; }
		// 클릭 콜백 - 스포너 항목(팝업 창)은 호출부가 선택을 원복한다.
		void SetOnSelect(std::function<void(int)> _fn) { m_OnSelect = std::move(_fn); }

		size_t ItemCount() const { return m_vItems.size(); }

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_NAVLIST; }
		const char*       GetTypeName() const override { return "navlist"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
	};

} // namespace dxgui
