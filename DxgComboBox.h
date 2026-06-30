// DxgComboBox.h: 드롭다운 콤보박스 위젯.
// 닫힘=선택항목+화살표. 클릭=드롭다운 토글. 드롭다운은 오버레이 패스(최상위)로 렌더.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"

#include <vector>
#include <string>
#include <functional>


namespace dxgui
{

	class C_DXG_COMBOBOX : public C_DXG_WIDGET
	{
	private:
		std::vector<std::wstring> m_vItems;
		int        m_nSel;
		bool       m_bOpen;
		bool       m_bJustOpened;	// 연 프레임 표식 — 오버레이의 외부클릭 닫기를 같은 클릭에서 1프레임 무시
		FontHandle m_hFont;
		float      m_fItemH;		// 드롭다운 항목 높이
		float      m_fCellPad;
		float      m_fScrollY;		// 드롭다운 스크롤 오프셋(px) - 항목이 박스보다 길 때

		_DXG_COLOR m_BgColor;
		_DXG_COLOR m_TextColor;
		_DXG_COLOR m_BorderColor;
		_DXG_COLOR m_BorderOpenColor;
		_DXG_COLOR m_ArrowColor;
		_DXG_COLOR m_DropBgColor;
		_DXG_COLOR m_ItemHoverBg;
		_DXG_COLOR m_ItemSelBg;

		std::function<void(int)> m_OnChange;

	public:
		C_DXG_COMBOBOX()
			: m_nSel(-1)
			, m_bOpen(false)
			, m_bJustOpened(false)
			, m_hFont(INVALID_FONT)
			, m_fItemH(24.0f)
			, m_fCellPad(6.0f)
			, m_fScrollY(0.0f)
			, m_BgColor(0xFFFFFFFFu)
			, m_TextColor(0xFF222838u)
			, m_BorderColor(0xFF8896A8u)
			, m_BorderOpenColor(0xFF236EE0u)
			, m_ArrowColor(0xFF5A6678u)
			, m_DropBgColor(0xFFFFFFFFu)
			, m_ItemHoverBg(0xFFEAF1FBu)
			, m_ItemSelBg(0xFFD6E4FFu)
		{
		}

		void SetFont(FontHandle _h)        { m_hFont = _h; }
		void SetItemHeight(float _h)       { m_fItemH = _h; }
		void SetOnChange(std::function<void(int)> _fn) { m_OnChange = std::move(_fn); }

		void AddItem(const std::wstring& _s) { m_vItems.push_back(_s); if (m_nSel < 0) { m_nSel = 0; } }
		void ClearItems() { m_vItems.clear(); m_nSel = -1; m_bOpen = false; m_fScrollY = 0.0f; }

		int  GetSelected() const { return m_nSel; }
		void SetSelected(int _i) { if (_i >= 0 && _i < static_cast<int>(m_vItems.size())) { m_nSel = _i; } }
		const std::wstring& GetSelectedText() const
		{
			static const std::wstring kEmpty;
			return (m_nSel >= 0 && m_nSel < static_cast<int>(m_vItems.size())) ? m_vItems[m_nSel] : kEmpty;
		}
		size_t ItemCount() const { return m_vItems.size(); }
		bool   IsOpen() const { return m_bOpen; }
		// 드롭다운 강제 닫기 — 호스트가 창 비활성(포그라운드 상실) 시 호출(매니저 CloseAllPopups).
		void   Close() { m_bOpen = false; m_bJustOpened = false; m_fScrollY = 0.0f; }
		// 열린 동안 모달 — 매니저가 pass1 입력을 캡처해 아래 위젯으로의 클릭 누수 차단.
		// (드롭다운 항목 클릭이 겹친 하단 버튼으로 관통하던 버그 차단.)
		bool   IsModalActive() const override { return m_bOpen; }

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_COMBOBOX; }
		const char*       GetTypeName() const override { return "combobox"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
		void RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin) override;

	private:
		void drawTextClip_(IDrawContext& _ctx, const std::wstring& _s,
			float _x, float _w, float _top, float _h, _DXG_COLOR _color);
	};

} // namespace dxgui
