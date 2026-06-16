// DxgSpinner.cpp
#include "DxgSpinner.h"
#include "DxgKeys.h"

#include <cstdio>
#include <cwchar>
#include <cstdlib>


namespace dxgui
{

	void C_DXG_SPINNER::commitBuf_()
	{
		if (m_pData == nullptr) { return; }
		const long long v = static_cast<long long>(::wcstol(m_sBuf.c_str(), nullptr, 10));
		long long c = v;
		if (c < m_nMin) { c = m_nMin; }
		if (c > m_nMax) { c = m_nMax; }
		this->setValue_(static_cast<int>(c));
	}

	void C_DXG_SPINNER::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const float fBtnW = 18.0f;
		const float fHalf = abs_.h * 0.5f;
		const _DXG_RECT textR_(abs_.x, abs_.y, abs_.w - fBtnW, abs_.h);
		const _DXG_RECT upR_(abs_.x + abs_.w - fBtnW, abs_.y, fBtnW, fHalf);
		const _DXG_RECT dnR_(abs_.x + abs_.w - fBtnW, abs_.y + fHalf, fBtnW, abs_.h - fHalf);

		// 배경 + 테두리(포커스 강조).
		_ctx.FillRect(abs_, m_BgColor);
		_ctx.DrawRectOutline(abs_, m_bFocused ? m_BorderFocusColor : m_BorderColor, m_bFocused ? 2.0f : 1.0f);

		// ── 입력(버튼 / 텍스트 포커스 / 외부 클릭 커밋) ──
		const bool bHovUp = _ctx.IsMouseHovered(upR_);
		const bool bHovDn = _ctx.IsMouseHovered(dnR_);
		if (m_bEnabled && _ctx.IsMouseClicked(DXG_MOUSE_LEFT))
		{
			if (bHovUp)      { this->step_(+1); }
			else if (bHovDn) { this->step_(-1); }
			else if (_ctx.IsMouseHovered(textR_))
			{
				if (!m_bFocused && m_pData != nullptr)
				{
					wchar_t b[32]{}; ::swprintf_s(b, L"%d", *m_pData);
					m_sBuf = b; m_bFocused = true; m_nBlink = 0; m_bSelected = true;	// 클릭=전체선택
				}
				else if (m_bFocused) { m_bSelected = true; }	// 재클릭 → 다시 전체선택
			}
			else if (m_bFocused) { this->commitBuf_(); m_bFocused = false; }	// 외부 클릭 커밋
		}

		// 표시 문자열(포커스=버퍼 / 아니면 값).
		wchar_t szVal[32]{};
		if (m_pData != nullptr) { ::swprintf_s(szVal, L"%d", *m_pData); }
		const std::wstring sShow = m_bFocused ? m_sBuf : std::wstring(szVal);
		const float fFontH = _ctx.GetFontHeight(m_hFont, m_fFontScale);
		const float fTy = abs_.y + (abs_.h - fFontH) * 0.5f;
		float fTextW = 0.0f;
		if (m_hFont != INVALID_FONT && !sShow.empty())
		{
			_ctx.PushClipRect(textR_);
			const _DXG_SIZE sz = _ctx.MeasureText(m_hFont, sShow.c_str(), m_fFontScale);
			fTextW = sz.w;
			if (m_bFocused && m_bSelected)	// 전체선택 하이라이트
			{
				_ctx.FillRect(_DXG_RECT(abs_.x + 6.0f, fTy, fTextW, fFontH), _DXG_COLOR(0xFFB5D2FFu));
			}
			_ctx.DrawText(m_hFont, _DXG_POINT(abs_.x + 6.0f, fTy), sShow.c_str(), m_TextColor, m_fFontScale);
			_ctx.PopClipRect();
		}
		// 캐럿.
		if (m_bFocused)
		{
			m_nBlink = (m_nBlink + 1) % 60;
			if (m_nBlink < 30)
			{
				_ctx.FillRect(_DXG_RECT(abs_.x + 6.0f + fTextW, fTy, 2.0f, fFontH), m_BorderFocusColor);
			}
		}

		// ── 키 입력(포커스 시) ──
		if (m_bFocused && m_bEnabled)
		{
			if (_ctx.IsKeyPressed(DXG_VK_BACK) && !m_sBuf.empty())
			{
				if (m_bSelected) { m_sBuf.clear(); m_bSelected = false; }	// 전체선택 → 통째 삭제
				else { m_sBuf.pop_back(); }
			}
			const wchar_t* pTxt = _ctx.PollTextInput();
			if (pTxt != nullptr)
			{
				if (m_bSelected) { m_sBuf.clear(); m_bSelected = false; }	// 전체선택 → 첫 입력이 교체
				const std::wstring sN(pTxt);
				for (const wchar_t c : sN)
				{
					if ((c >= L'0' && c <= L'9') || (c == L'-' && m_sBuf.empty())) { m_sBuf.push_back(c); }
				}
			}
			if (_ctx.IsKeyPressed(DXG_VK_RETURN) || _ctx.IsKeyPressed(DXG_VK_TAB))
			{
				this->commitBuf_(); m_bFocused = false;
			}
		}

		// ── 버튼(배경/분리선/화살표) ──
		_ctx.FillRect(upR_, bHovUp ? m_BtnHover : m_BtnBg);
		_ctx.FillRect(dnR_, bHovDn ? m_BtnHover : m_BtnBg);
		_ctx.DrawLine(_DXG_POINT(upR_.x, abs_.y), _DXG_POINT(upR_.x, abs_.y + abs_.h), m_BorderColor, 1.0f);
		_ctx.DrawLine(_DXG_POINT(upR_.x, abs_.y + fHalf), _DXG_POINT(abs_.x + abs_.w, abs_.y + fHalf), m_BorderColor, 1.0f);

		const float fUx = upR_.x + fBtnW * 0.5f, fUy = upR_.y + upR_.h * 0.5f;
		const _DXG_POINT triUp[3] = { { fUx - 4.0f, fUy + 2.0f }, { fUx + 4.0f, fUy + 2.0f }, { fUx, fUy - 3.0f } };
		_ctx.FillTriangle(triUp, m_ArrowColor);
		const float fDx = dnR_.x + fBtnW * 0.5f, fDy = dnR_.y + dnR_.h * 0.5f;
		const _DXG_POINT triDn[3] = { { fDx - 4.0f, fDy - 2.0f }, { fDx + 4.0f, fDy - 2.0f }, { fDx, fDy + 3.0f } };
		_ctx.FillTriangle(triDn, m_ArrowColor);
	}

} // namespace dxgui
