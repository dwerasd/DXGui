// DxgComboBox.cpp
#include "DxgComboBox.h"


namespace dxgui
{

	void C_DXG_COMBOBOX::drawTextClip_(IDrawContext& _ctx, const std::wstring& _s,
		float _x, float _w, float _top, float _h, _DXG_COLOR _color)
	{
		if (_s.empty() || m_hFont == INVALID_FONT || _w <= 0.0f) { return; }
		_ctx.PushClipRect(_DXG_RECT(_x, _top, _w, _h));
		const _DXG_SIZE sz_ = _ctx.MeasureText(m_hFont, _s.c_str(), m_fFontScale);
		const float fTy = _top + (_h - sz_.h) * 0.5f;
		_ctx.DrawText(m_hFont, _DXG_POINT(_x + m_fCellPad, fTy), _s.c_str(), _color, m_fFontScale);
		_ctx.PopClipRect();
	}

	void C_DXG_COMBOBOX::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const bool bHov = _ctx.IsMouseHovered(abs_);

		// 박스 배경 + 테두리(열림 시 강조).
		_ctx.FillRect(abs_, m_BgColor);
		_ctx.DrawRectOutline(abs_, m_bOpen ? m_BorderOpenColor : m_BorderColor, m_bOpen ? 2.0f : 1.0f);

		// 선택 텍스트(화살표 영역 제외 폭).
		const float fArrowZone = 18.0f;
		this->drawTextClip_(_ctx, this->GetSelectedText(), abs_.x, abs_.w - fArrowZone, abs_.y, abs_.h, m_TextColor);

		// 화살표(아래 삼각형).
		const float cx = abs_.x + abs_.w - 10.0f;
		const float cy = abs_.y + abs_.h * 0.5f;
		const _DXG_POINT tri[3] = {
			_DXG_POINT(cx - 4.0f, cy - 2.5f),
			_DXG_POINT(cx + 4.0f, cy - 2.5f),
			_DXG_POINT(cx,        cy + 3.0f),
		};
		_ctx.FillTriangle(tri, m_ArrowColor);

		// 열기만 처리(헤더 클릭). 닫기/항목선택은 오버레이 패스에서 처리.
		// 열린 동안은 모달(IsModalActive=true) → pass1 캡처로 이 블록도 차단되므로
		// 닫기 로직을 여기 두면 동작하지 않는다. 닫기는 RenderOverlay 로 이동.
		if (m_bEnabled && !m_bOpen && bHov && _ctx.IsMouseClicked(DXG_MOUSE_LEFT))
		{
			m_bOpen = true;
			m_bJustOpened = true;	// 연 클릭이 오버레이의 외부클릭 닫기를 같은 프레임에 트리거하지 않도록
		}
	}

	void C_DXG_COMBOBOX::RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible || !m_bOpen || m_vItems.empty()) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const _DXG_RECT dd_  = this->dropRect_(abs_);

		_ctx.FillRect(dd_, m_DropBgColor);
		_ctx.DrawRectOutline(dd_, m_BorderOpenColor, 1.0f);

		const int n = static_cast<int>(m_vItems.size());
		for (int i = 0; i < n; ++i)
		{
			const _DXG_RECT itemR_(dd_.x, dd_.y + static_cast<float>(i) * m_fItemH, dd_.w, m_fItemH);
			const bool bHov = _ctx.IsMouseHovered(itemR_);
			if (i == m_nSel)      { _ctx.FillRect(itemR_, m_ItemSelBg); }
			else if (bHov)        { _ctx.FillRect(itemR_, m_ItemHoverBg); }
			this->drawTextClip_(_ctx, m_vItems[i], itemR_.x, itemR_.w, itemR_.y, itemR_.h, m_TextColor);

			if (m_bEnabled && bHov && _ctx.IsMouseClicked(DXG_MOUSE_LEFT))
			{
				if (m_nSel != i) { m_nSel = i; if (m_OnChange) { m_OnChange(i); } }
				m_bOpen = false;
			}
		}

		// 드롭다운 밖(헤더 포함) 클릭 → 닫기. 오버레이 패스(capture=false)에서만 동작.
		// 연 프레임은 헤더 클릭이 곧 외부클릭이므로 1프레임 무시(즉시 닫힘 방지).
		if (m_bJustOpened)
		{
			m_bJustOpened = false;
		}
		else if (m_bEnabled && _ctx.IsMouseClicked(DXG_MOUSE_LEFT) && !_ctx.IsMouseHovered(dd_))
		{
			m_bOpen = false;
		}
	}

} // namespace dxgui
