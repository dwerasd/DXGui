// DxgListGrid.cpp
#include "DxgListGrid.h"

#include <cmath>


namespace dxgui
{

	C_DXG_LISTGRID::C_DXG_LISTGRID()
		: m_hFont(INVALID_FONT)
		, m_fRowH(22.0f)
		, m_fHeaderH(26.0f)
		, m_nSelRow(-1)
		, m_bRowAlt(true)
		, m_bGridLines(true)
		, m_fScrollbarW(14.0f)
		, m_fCellPad(6.0f)
		, m_HeaderBg(0xFFEDF1F6u)
		, m_HeaderText(0xFF333A44u)
		, m_GridLine(0xFFE2E6ECu)
		, m_RowBg(0xFFFFFFFFu)
		, m_RowAltBg(0xFFF7F9FCu)
		, m_SelBg(0xFFD6E4FFu)
		, m_SelText(0xFF1A2638u)
	{
	}

	void C_DXG_LISTGRID::drawCellText_(IDrawContext& _ctx, const std::wstring& _s, _DXG_COLOR _color,
		float _colX, float _colW, float _top, float _h, E_DXG_TEXT_ALIGN _align)
	{
		if (_s.empty() || m_hFont == INVALID_FONT || _colW <= 0.0f) { return; }
		_ctx.PushClipRect(_DXG_RECT(_colX, _top, _colW, _h));
		const _DXG_SIZE sz_ = _ctx.MeasureText(m_hFont, _s.c_str(), m_fFontScale);
		float fTx = _colX + m_fCellPad;
		if (_align == DXG_TEXT_ALIGN_CENTER)     { fTx = _colX + (_colW - sz_.w) * 0.5f; }
		else if (_align == DXG_TEXT_ALIGN_RIGHT) { fTx = _colX + _colW - sz_.w - m_fCellPad; }
		const float fTy = _top + (_h - sz_.h) * 0.5f;
		_ctx.DrawText(m_hFont, _DXG_POINT(fTx, fTy), _s.c_str(), _color, m_fFontScale);
		_ctx.PopClipRect();
	}

	void C_DXG_LISTGRID::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		if (abs_.w <= 0.0f || abs_.h <= 0.0f) { return; }

		const int   nRows    = static_cast<int>(m_vRows.size());
		const int   nCols    = static_cast<int>(m_vCols.size());
		const float fHeaderH = m_fHeaderH;
		const float fBodyTop = abs_.y + fHeaderH;
		const float fBodyH   = abs_.h - fHeaderH;
		const float fContentH = static_cast<float>(nRows) * m_fRowH;

		// 스크롤바 필요 판정 + 본문 폭 산출.
		m_ScrollV.SetMetrics(fContentH, (fBodyH > 0.0f) ? fBodyH : 0.0f);
		const bool  bScroll = m_ScrollV.Needed() && fBodyH > 0.0f;
		const float fSbW    = bScroll ? m_fScrollbarW : 0.0f;
		const float fBodyW  = abs_.w - fSbW;
		const _DXG_RECT bodyAbs_(abs_.x, fBodyTop, fBodyW, (fBodyH > 0.0f) ? fBodyH : 0.0f);

		// 휠(본문 위) → 스크롤.
		const float fWheel = _ctx.GetWheelDelta();
		if (bScroll && fWheel != 0.0f && _ctx.IsMouseHovered(bodyAbs_))
		{
			m_ScrollV.ScrollBy(-fWheel * m_fRowH * 3.0f);
		}

		// 스크롤바(드래그 처리) — 우측 스트립. 값 확정 후 본문 레이아웃.
		if (bScroll)
		{
			m_ScrollV.SetRect(_DXG_RECT(m_Rect.x + fBodyW, m_Rect.y + fHeaderH, fSbW, fBodyH));
			m_ScrollV.Render(_ctx, _origin);
		}
		const float fScrollY = m_ScrollV.GetValue();

		// 행 클릭 선택(본문 영역, 스크롤바 제외).
		if (m_bEnabled && _ctx.IsMouseClicked(DXG_MOUSE_LEFT) && _ctx.IsMouseHovered(bodyAbs_))
		{
			const float fMy = _ctx.GetMousePos().y;
			const int r = (m_fRowH > 0.0f) ? static_cast<int>(::floorf((fMy - fBodyTop + fScrollY) / m_fRowH)) : -1;
			if (r >= 0 && r < nRows && m_nSelRow != r)
			{
				m_nSelRow = r;
				if (m_OnSelect) { m_OnSelect(r); }
			}
		}

		// ── 본문(행) — 클립 후 가시 행만 ──
		if (fBodyH > 0.0f)
		{
			_ctx.PushClipRect(bodyAbs_);
			int firstR = (m_fRowH > 0.0f) ? static_cast<int>(::floorf(fScrollY / m_fRowH)) : 0;
			int lastR  = (m_fRowH > 0.0f) ? static_cast<int>(::ceilf((fScrollY + fBodyH) / m_fRowH)) : 0;
			if (firstR < 0) { firstR = 0; }
			for (int r = firstR; r < nRows && r <= lastR; ++r)
			{
				const float fRowY = fBodyTop + static_cast<float>(r) * m_fRowH - fScrollY;
				// 행 기본 배경(선택 > 교대 > 기본).
				_DXG_COLOR base = m_RowBg;
				if (r == m_nSelRow)               { base = m_SelBg; }
				else if (m_bRowAlt && (r & 1))    { base = m_RowAltBg; }
				_ctx.FillRect(_DXG_RECT(abs_.x, fRowY, fBodyW, m_fRowH), base);

				const std::vector<_DXG_GRID_CELL>& row = m_vRows[r];
				float fCx = abs_.x;
				for (int c = 0; c < nCols; ++c)
				{
					const float fCw = m_vCols[c].fWidth;
					if (fCx >= abs_.x + fBodyW) { break; }	// 우측 클립
					const _DXG_GRID_CELL* pCell = (c < static_cast<int>(row.size())) ? &row[c] : nullptr;
					if (pCell != nullptr && pCell->bgColor.A() > 0 && r != m_nSelRow)
					{
						_ctx.FillRect(_DXG_RECT(fCx, fRowY, fCw, m_fRowH), pCell->bgColor);
					}
					if (pCell != nullptr)
					{
						const _DXG_COLOR tcol = (r == m_nSelRow) ? m_SelText : pCell->textColor;
						this->drawCellText_(_ctx, pCell->sText, tcol, fCx, fCw, fRowY, m_fRowH, m_vCols[c].align);
					}
					fCx += fCw;
				}
				if (m_bGridLines)
				{
					_ctx.DrawLine(_DXG_POINT(abs_.x, fRowY + m_fRowH),
						_DXG_POINT(abs_.x + fBodyW, fRowY + m_fRowH), m_GridLine, 1.0f);
				}
			}
			_ctx.PopClipRect();
		}

		// ── 헤더(고정) ──
		_ctx.FillRect(_DXG_RECT(abs_.x, abs_.y, abs_.w, fHeaderH), m_HeaderBg);
		{
			float fHx = abs_.x;
			for (int c = 0; c < nCols; ++c)
			{
				const float fCw = m_vCols[c].fWidth;
				if (fHx >= abs_.x + abs_.w) { break; }
				if (m_bGridLines && c > 0)
				{
					_ctx.DrawLine(_DXG_POINT(fHx, abs_.y), _DXG_POINT(fHx, abs_.y + fHeaderH), m_GridLine, 1.0f);
				}
				this->drawCellText_(_ctx, m_vCols[c].sTitle, m_HeaderText, fHx, fCw, abs_.y, fHeaderH, m_vCols[c].align);
				fHx += fCw;
			}
		}
		// 헤더 하단선 + 외곽 테두리.
		_ctx.DrawLine(_DXG_POINT(abs_.x, fBodyTop), _DXG_POINT(abs_.x + abs_.w, fBodyTop), m_GridLine, 1.0f);
		_ctx.DrawRectOutline(abs_, m_GridLine, 1.0f);
	}

} // namespace dxgui
