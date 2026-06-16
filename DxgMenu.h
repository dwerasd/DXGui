// DxgMenu.h: 컨텍스트(팝업) 메뉴 위젯. 임의 위치(우클릭 등)에 떠서 항목 선택.
// 인라인 렌더 없음 — 오버레이 패스(최상위)에서만 그려진다. 매니저 top-level 로 추가.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"

#include <vector>
#include <string>
#include <functional>


namespace dxgui
{

	class C_DXG_MENU : public C_DXG_WIDGET
	{
	private:
		struct _MENU_ITEM
		{
			std::wstring          sLabel;
			std::function<void()> onClick;
			bool                  bSeparator;
			bool                  bEnabled;
		};
		std::vector<_MENU_ITEM> m_vItems;

		bool       m_bOpen;
		float      m_fPosX, m_fPosY;	// 팝업 좌상단(절대 px)
		FontHandle m_hFont;
		float      m_fItemH, m_fSepH, m_fCellPad, m_fMinWidth;

		_DXG_COLOR m_BgColor, m_BorderColor, m_TextColor, m_TextDisabled, m_HoverBg, m_SepColor;

	public:
		C_DXG_MENU()
			: m_bOpen(false)
			, m_fPosX(0.0f), m_fPosY(0.0f)
			, m_hFont(INVALID_FONT)
			, m_fItemH(26.0f), m_fSepH(9.0f), m_fCellPad(12.0f), m_fMinWidth(140.0f)
			, m_BgColor(0xFFFFFFFFu)
			, m_BorderColor(0xFF9AA6B4u)
			, m_TextColor(0xFF222838u)
			, m_TextDisabled(0xFFAAB2BEu)
			, m_HoverBg(0xFFEAF1FBu)
			, m_SepColor(0xFFDDE2E9u)
		{
		}

		void SetFont(FontHandle _h)  { m_hFont = _h; }
		void SetItemHeight(float _h) { m_fItemH = _h; }
		void SetMinWidth(float _w)   { m_fMinWidth = _w; }

		void AddItem(const std::wstring& _sLabel, std::function<void()> _fn, bool _bEnabled = true)
		{ m_vItems.push_back(_MENU_ITEM{ _sLabel, std::move(_fn), false, _bEnabled }); }
		void AddSeparator()
		{ m_vItems.push_back(_MENU_ITEM{ std::wstring(), std::function<void()>(), true, false }); }
		void ClearItems() { m_vItems.clear(); m_bOpen = false; }

		void ShowAt(float _x, float _y) { m_fPosX = _x; m_fPosY = _y; m_bOpen = true; }
		void Close() { m_bOpen = false; }
		bool IsOpen() const { return m_bOpen; }

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_MENU; }
		const char*       GetTypeName() const override { return "menu"; }

		void Render(IDrawContext& /*_ctx*/, _DXG_POINT /*_origin*/) override {}	// 인라인 없음
		void RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin) override;
	};

} // namespace dxgui
