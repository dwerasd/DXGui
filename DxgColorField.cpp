// DxgColorField.cpp
#include "DxgColorField.h"
#include "DxgKeys.h"

#include <cstdio>
#include <cwchar>


namespace dxgui
{
	// 프리셋 팔레트 8열 x 4행(불투명 FF). 회색 램프 + 표준/진한/연한 색조.
	// 거래마커 기본(매수 C62828 / 매도 1565C0) 포함.
	static const unsigned int s_aPalette[32] =
	{
		0xFFFFFFFFu, 0xFFE0E0E0u, 0xFFBDBDBDu, 0xFF9E9E9Eu, 0xFF757575u, 0xFF616161u, 0xFF424242u, 0xFF000000u,
		0xFFEF5350u, 0xFFFF7043u, 0xFFFFCA28u, 0xFF66BB6Au, 0xFF26A69Au, 0xFF42A5F5u, 0xFF7E57C2u, 0xFFEC407Au,
		0xFFC62828u, 0xFFE64A19u, 0xFFF9A825u, 0xFF2E7D32u, 0xFF00695Cu, 0xFF1565C0u, 0xFF4527A0u, 0xFFAD1457u,
		0xFFEF9A9Au, 0xFFFFAB91u, 0xFFFFE082u, 0xFFA5D6A7u, 0xFF80CBC4u, 0xFF90CAF9u, 0xFFB39DDBu, 0xFFF48FB1u,
	};

	static const int   kCols    = 8;
	static const int   kRows    = 4;
	static const float kCell    = 18.0f;
	static const float kGap     = 3.0f;
	static const float kPad     = 8.0f;
	static const float kHexH    = 22.0f;
	static const float kHashW   = 14.0f;	// '#' 표시 폭

	std::wstring C_DXG_COLOR_FIELD::HexFromColor_(unsigned int _argb)
	{
		wchar_t sz[16];
		if (((_argb >> 24) & 0xFFu) == 0xFFu) { ::swprintf_s(sz, L"%06X", _argb & 0x00FFFFFFu); }
		else                                  { ::swprintf_s(sz, L"%08X", _argb); }
		return std::wstring(sz);
	}

	void C_DXG_COLOR_FIELD::applyHex_()
	{
		// 6자리=RGB(기존 알파 보존), 8자리=ARGB. 그 외=미완성(무시).
		if (m_sHex.size() == 6)
		{
			const unsigned int nRgb = static_cast<unsigned int>(::wcstoul(m_sHex.c_str(), nullptr, 16)) & 0x00FFFFFFu;
			m_Color = (m_Color & 0xFF000000u) | nRgb;
			if (m_OnChange) { m_OnChange(m_Color); }
		}
		else if (m_sHex.size() == 8)
		{
			m_Color = static_cast<unsigned int>(::wcstoul(m_sHex.c_str(), nullptr, 16));
			if (m_OnChange) { m_OnChange(m_Color); }
		}
	}

	void C_DXG_COLOR_FIELD::pickColor_(unsigned int _argb)
	{
		m_Color = _argb;
		m_sHex  = HexFromColor_(_argb);
		if (m_OnChange) { m_OnChange(m_Color); }
		m_bOpen = false;
	}

	void C_DXG_COLOR_FIELD::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const bool bHov = _ctx.IsMouseHovered(abs_);

		// 스와치: 체크무늬 생략(마커는 불투명) — 색 채움 + 테두리.
		const _DXG_RECT inR_(abs_.x + 1.0f, abs_.y + 1.0f, abs_.w - 2.0f, abs_.h - 2.0f);
		_ctx.FillRect(inR_, _DXG_COLOR(m_Color));
		_ctx.DrawRectOutline(abs_, (m_bOpen || bHov) ? m_BorderOpenColor : m_BorderColor, (m_bOpen || bHov) ? 2.0f : 1.0f);

		// 열기(헤더 클릭) — UP 구동(콤보와 동일, 클릭관통 차단).
		if (m_bEnabled && !m_bOpen && bHov && _ctx.IsMouseReleased(DXG_MOUSE_LEFT))
		{
			m_bOpen = true;
			m_bJustOpened = true;
			m_sHex = HexFromColor_(m_Color);
		}
	}

	void C_DXG_COLOR_FIELD::RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible || !m_bOpen) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const float fGridW = kCols * kCell + (kCols - 1) * kGap;	// 144 + 21 = 165
		const float fPopW  = fGridW + kPad * 2.0f;				// 181
		const float fGridH = kRows * kCell + (kRows - 1) * kGap;	// 72 + 9 = 81
		const float fPopH  = kPad + fGridH + 10.0f + kHexH + kPad;	// 8+81+10+22+8 = 129
		const _DXG_RECT pop_(abs_.x, abs_.y + abs_.h + 2.0f, fPopW, fPopH);

		_ctx.FillRect(pop_, m_PopupBgColor);
		_ctx.DrawRectOutline(pop_, m_BorderOpenColor, 1.0f);

		// 팔레트 그리드.
		const float gx = pop_.x + kPad;
		const float gy = pop_.y + kPad;
		for (int r = 0; r < kRows; ++r)
		{
			for (int c = 0; c < kCols; ++c)
			{
				const int idx = r * kCols + c;
				const _DXG_RECT cell_(gx + c * (kCell + kGap), gy + r * (kCell + kGap), kCell, kCell);
				_ctx.FillRect(cell_, _DXG_COLOR(s_aPalette[idx]));
				const bool bCellHov = _ctx.IsMouseHovered(cell_);
				const bool bSel = ((s_aPalette[idx] & 0x00FFFFFFu) == (m_Color & 0x00FFFFFFu));
				if (bSel || bCellHov)
				{
					_ctx.DrawRectOutline(cell_, bSel ? m_BorderOpenColor : _DXG_COLOR(0xFF222838u), 2.0f);
				}
				if (m_bEnabled && bCellHov && _ctx.IsMouseReleased(DXG_MOUSE_LEFT))
				{
					this->pickColor_(s_aPalette[idx]);
				}
			}
		}

		// hex 입력줄: '#' + 버퍼 + 캐럿. 항상 활성(팝업 열림=hex 포커스).
		const float hy = gy + fGridH + 10.0f;
		const _DXG_RECT hexBox_(gx + kHashW, hy, fGridW - kHashW, kHexH);
		if (m_hFont != INVALID_FONT)
		{
			_ctx.DrawText(m_hFont, _DXG_POINT(gx, hy + 3.0f), L"#", m_TextColor, m_fFontScale);
		}
		_ctx.FillRect(hexBox_, _DXG_COLOR(0xFFF4F6FAu));
		_ctx.DrawRectOutline(hexBox_, m_BorderOpenColor, 1.0f);
		if (m_hFont != INVALID_FONT)
		{
			_ctx.DrawText(m_hFont, _DXG_POINT(hexBox_.x + 5.0f, hy + 3.0f), m_sHex.c_str(), m_TextColor, m_fFontScale);
			const _DXG_SIZE szT = _ctx.MeasureText(m_hFont, m_sHex.c_str(), m_fFontScale);
			_ctx.FillRect(_DXG_RECT(hexBox_.x + 5.0f + szT.w + 1.0f, hy + 4.0f, 1.5f, kHexH - 8.0f), m_BorderOpenColor);
		}

		// 입력 처리(오버레이 패스 = capture off → 실입력).
		if (m_bEnabled)
		{
			bool bDirty = false;
			// 붙여넣기(Ctrl+V) — colorcop/트레이딩뷰 hex 복붙. '#'/공백 제거, hex 만, 8 캡.
			if (_ctx.IsKeyDown(DXG_VK_CONTROL) && _ctx.IsKeyPressed('V'))
			{
				const wchar_t* pClip = _ctx.GetClipboardText();
				if (pClip != nullptr)
				{
					for (const wchar_t c : std::wstring(pClip))
					{
						if (m_sHex.size() >= 8) { break; }
						if (IsHexCh_(c)) { m_sHex += static_cast<wchar_t>(::towupper(c)); bDirty = true; }
					}
				}
			}
			// 타이핑(IME 결과 포함 텍스트 큐).
			const wchar_t* pTxt = _ctx.PollTextInput();
			if (pTxt != nullptr)
			{
				for (const wchar_t c : std::wstring(pTxt))
				{
					if (m_sHex.size() >= 8) { break; }
					if (IsHexCh_(c)) { m_sHex += static_cast<wchar_t>(::towupper(c)); bDirty = true; }
				}
			}
			// 백스페이스.
			if (_ctx.IsKeyPressed(DXG_VK_BACK) && !m_sHex.empty()) { m_sHex.pop_back(); bDirty = true; }

			if (bDirty) { this->applyHex_(); }	// 6/8 자리면 라이브 반영

			// Enter → 확정 + 닫기.
			if (_ctx.IsKeyPressed(DXG_VK_RETURN)) { this->applyHex_(); m_bOpen = false; }
		}

		// 팝업 밖 클릭 → 닫기(연 프레임 1회 무시).
		if (m_bJustOpened)
		{
			m_bJustOpened = false;
		}
		else if (m_bEnabled && _ctx.IsMouseReleased(DXG_MOUSE_LEFT) && !_ctx.IsMouseHovered(pop_))
		{
			m_bOpen = false;
		}
	}

} // namespace dxgui
