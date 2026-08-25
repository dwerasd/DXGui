// DxgComboBox.cpp
#include "DxgComboBox.h"


namespace dxgui
{
	// 호스트가 매 프레임 설정하는 캔버스(클라이언트, DIP) 크기 정의(선언=DxgDrawContext.h extern).
	float g_fOverlayCanvasH = 0.0f;
	float g_fOverlayCanvasW = 0.0f;

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
		// ★UP(Released) 구동 — 버튼과 동일. DOWN 구동이면 콤보가 DOWN 에 닫혀 모달이 풀린 뒤
		//   이어진 UP 에서 드롭다운 아래 버튼이 발화(클릭관통). UP 까지 모달 유지해 아래 위젯 차단.
		if (m_bEnabled && !m_bOpen && bHov && _ctx.IsMouseReleased(DXG_MOUSE_LEFT))
		{
			m_bOpen = true;
			m_bJustOpened = true;	// 연 릴리즈가 오버레이의 외부클릭 닫기를 같은 프레임에 트리거하지 않도록
			m_fScrollY = m_fItemH * static_cast<float>(m_nSel > 0 ? m_nSel : 0);	// 선택항목이 보이게(오버레이에서 클램프)
		}
	}

	void C_DXG_COMBOBOX::RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible || !m_bOpen || m_vItems.empty()) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const int n = static_cast<int>(m_vItems.size());
		const float fFullH = m_fItemH * static_cast<float>(n);

		// 펼침 방향/높이 — 캔버스(창) 안에 들어가도록 클램프. 아래 공간이 너무 작으면 위로 펼침.
		// g_fOverlayCanvasH<=0(미주입) 이면 기존 동작(전체 높이 아래로).
		const float fCanvasH = g_fOverlayCanvasH;
		const float fMargin  = 2.0f;
		float fRoomBelow = (fCanvasH > 0.0f) ? (fCanvasH - (abs_.y + abs_.h) - fMargin) : fFullH;
		float fRoomAbove = (fCanvasH > 0.0f) ? (abs_.y - fMargin) : fFullH;
		if (fRoomBelow < 0.0f) { fRoomBelow = 0.0f; }
		if (fRoomAbove < 0.0f) { fRoomAbove = 0.0f; }
		const float fMinH = m_fItemH * 4.0f;	// 아래가 이보다 좁고 위가 더 넓으면 위로 전환
		bool  bUp   = (fCanvasH > 0.0f) && (fRoomBelow < fFullH) && (fRoomBelow < fMinH) && (fRoomAbove > fRoomBelow);
		float fAvail = bUp ? fRoomAbove : fRoomBelow;
		int   nVis  = (fAvail >= fFullH) ? n : static_cast<int>(fAvail / m_fItemH);
		if (nVis < 1) { nVis = 1; }
		if (nVis > n) { nVis = n; }
		const float fDropH = m_fItemH * static_cast<float>(nVis);
		const float fDropY = bUp ? (abs_.y - fDropH) : (abs_.y + abs_.h);
		const _DXG_RECT dd_(abs_.x, fDropY, abs_.w, fDropH);

		// 스크롤 가능 여부 + 휠 처리(호스트가 모달 중 휠을 컨텍스트로 넘김). 노치당 3행.
		const float fMaxScroll = fFullH - fDropH;
		const bool  bScroll    = fMaxScroll > 0.5f;
		if (bScroll)
		{
			const float fWheel = _ctx.GetWheelDelta();
			if (fWheel != 0.0f) { m_fScrollY -= fWheel * m_fItemH * 3.0f; }
		}
		if (m_fScrollY < 0.0f) { m_fScrollY = 0.0f; }
		if (m_fScrollY > fMaxScroll) { m_fScrollY = (fMaxScroll > 0.0f) ? fMaxScroll : 0.0f; }

		_ctx.FillRect(dd_, m_DropBgColor);
		_ctx.DrawRectOutline(dd_, m_BorderOpenColor, 1.0f);

		const float fBarW  = bScroll ? 6.0f : 0.0f;	// 스크롤바 폭(있을 때만)
		const float fItemW = dd_.w - fBarW;

		_ctx.PushClipRect(dd_);	// 항목을 드롭다운 박스로 클립(스크롤 넘침/캔버스 밖 미표시)
		for (int i = 0; i < n; ++i)
		{
			const float fItemY = dd_.y + static_cast<float>(i) * m_fItemH - m_fScrollY;
			if (fItemY + m_fItemH <= dd_.y || fItemY >= dd_.y + dd_.h) { continue; }	// 박스 밖 = skip
			const _DXG_RECT itemR_(dd_.x, fItemY, fItemW, m_fItemH);
			const bool bHov = _ctx.IsMouseHovered(itemR_);
			if (i == m_nSel)      { _ctx.FillRect(itemR_, m_ItemSelBg); }
			else if (bHov)        { _ctx.FillRect(itemR_, m_ItemHoverBg); }
			this->drawTextClip_(_ctx, m_vItems[i], itemR_.x, itemR_.w, itemR_.y, itemR_.h, m_TextColor);

			if (m_bEnabled && bHov && _ctx.IsMouseReleased(DXG_MOUSE_LEFT))	// UP 구동(아래 버튼 클릭관통 차단)
			{
				if (m_nSel != i) { m_nSel = i; if (m_OnChange) { m_OnChange(i); } }
				m_bOpen = false;
			}
		}
		_ctx.PopClipRect();

		// 스크롤바(트랙+thumb) — 우측 가장자리. 휠 구동(드래그 미지원).
		if (bScroll)
		{
			const float fTrackX = dd_.x + dd_.w - fBarW;
			_ctx.FillRect(_DXG_RECT(fTrackX, dd_.y, fBarW, dd_.h), m_ItemHoverBg);
			const float fThumbH = (dd_.h > 0.0f) ? (dd_.h * (dd_.h / fFullH)) : 0.0f;
			const float fThumbY = dd_.y + (m_fScrollY / fMaxScroll) * (dd_.h - fThumbH);
			_ctx.FillRect(_DXG_RECT(fTrackX + 1.0f, fThumbY, fBarW - 2.0f, fThumbH), m_ArrowColor);
		}

		// 드롭다운 밖(헤더 포함) 클릭 → 닫기. 오버레이 패스(capture=false)에서만 동작.
		// 연 프레임은 헤더 클릭이 곧 외부클릭이므로 1프레임 무시(즉시 닫힘 방지).
		if (m_bJustOpened)
		{
			m_bJustOpened = false;
		}
		else if (m_bEnabled && _ctx.IsMouseReleased(DXG_MOUSE_LEFT) && !_ctx.IsMouseHovered(dd_))	// UP 구동
		{
			m_bOpen = false;
		}
	}

} // namespace dxgui
