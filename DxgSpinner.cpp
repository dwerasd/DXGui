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

		const float fTx0 = abs_.x + 6.0f;	// 텍스트 좌측 기준 X

		// 캐럿 인덱스 → 픽셀 X(텍스트 좌측 기준). 부분문자열 측정.
		auto caretX_ = [&](size_t _i) -> float
		{
			if (m_hFont == INVALID_FONT || _i == 0 || m_sBuf.empty()) { return 0.0f; }
			if (_i > m_sBuf.size()) { _i = m_sBuf.size(); }
			return _ctx.MeasureText(m_hFont, m_sBuf.substr(0, _i).c_str(), m_fFontScale).w;
		};
		// 로컬 X → 가장 가까운 문자 경계 인덱스.
		auto hitIdx_ = [&](float _localX) -> size_t
		{
			size_t uBest_ = 0;
			float  fBestD_ = (_localX < 0.0f) ? -_localX : _localX;
			for (size_t i_ = 1; i_ <= m_sBuf.size(); ++i_)
			{
				const float fW_ = caretX_(i_);
				const float fD_ = (_localX < fW_) ? (fW_ - _localX) : (_localX - fW_);
				if (fD_ <= fBestD_) { fBestD_ = fD_; uBest_ = i_; }
			}
			return uBest_;
		};

		// ── 입력(버튼 / 텍스트 포커스+캐럿 / 더블클릭 전체선택 / 드래그선택 / 외부 클릭 커밋) ──
		const bool bHovUp = _ctx.IsMouseHovered(upR_);
		const bool bHovDn = _ctx.IsMouseHovered(dnR_);
		if (m_bEnabled && _ctx.IsMouseHovered(textR_) && _ctx.IsMouseDoubleClicked(DXG_MOUSE_LEFT))
		{
			if (!m_bFocused && m_pData != nullptr)
			{
				wchar_t b[32]{}; ::swprintf_s(b, L"%d", *m_pData);
				m_sBuf = b; m_bFocused = true; m_nBlink = 0;
			}
			m_uSelAnchor = 0; m_uCaret = m_sBuf.size();	// 전체선택
			m_bDragSel = false;
		}
		else if (m_bEnabled && _ctx.IsMouseClicked(DXG_MOUSE_LEFT))
		{
			if (bHovUp)      { this->step_(+1); }
			else if (bHovDn) { this->step_(-1); }
			else if (_ctx.IsMouseHovered(textR_))
			{
				if (!m_bFocused && m_pData != nullptr)
				{
					wchar_t b[32]{}; ::swprintf_s(b, L"%d", *m_pData);
					m_sBuf = b; m_bFocused = true; m_nBlink = 0;
				}
				const size_t uHit_ = hitIdx_(_ctx.GetMousePos().x - fTx0);
				m_uCaret = uHit_; m_uSelAnchor = uHit_;	// 캐럿만(선택 해제)
				m_bDragSel = true;
			}
			else if (m_bFocused) { this->commitBuf_(); m_bFocused = false; m_bDragSel = false; }	// 외부 클릭 커밋
		}
		// 드래그 중 — 앵커 고정, 캐럿이 마우스를 따라가며 범위 확장.
		if (m_bDragSel && m_bFocused && _ctx.IsMouseDown(DXG_MOUSE_LEFT))
		{
			m_uCaret = hitIdx_(_ctx.GetMousePos().x - fTx0);
		}
		if (_ctx.IsMouseReleased(DXG_MOUSE_LEFT)) { m_bDragSel = false; }

		// 캐럿/앵커 클램프 후 선택범위 산출.
		if (m_uCaret > m_sBuf.size())     { m_uCaret = m_sBuf.size(); }
		if (m_uSelAnchor > m_sBuf.size()) { m_uSelAnchor = m_sBuf.size(); }
		const size_t uSelL_ = (m_uSelAnchor < m_uCaret) ? m_uSelAnchor : m_uCaret;
		const size_t uSelR_ = (m_uSelAnchor < m_uCaret) ? m_uCaret : m_uSelAnchor;
		const bool   bHasSel_ = (uSelL_ != uSelR_);

		// 표시 문자열(포커스=버퍼 / 아니면 값).
		wchar_t szVal[32]{};
		if (m_pData != nullptr) { ::swprintf_s(szVal, L"%d", *m_pData); }
		const std::wstring sShow = m_bFocused ? m_sBuf : std::wstring(szVal);
		const float fFontH = _ctx.GetFontHeight(m_hFont, m_fFontScale);
		const float fTy = abs_.y + (abs_.h - fFontH) * 0.5f;
		if (m_hFont != INVALID_FONT && !sShow.empty())
		{
			_ctx.PushClipRect(textR_);
			if (m_bFocused && bHasSel_)	// 선택 하이라이트(텍스트 뒤)
			{
				const float fXL_ = caretX_(uSelL_);
				const float fXR_ = caretX_(uSelR_);
				_ctx.FillRect(_DXG_RECT(fTx0 + fXL_, fTy, fXR_ - fXL_, fFontH), _DXG_COLOR(0xFFB5D2FFu));
			}
			_ctx.DrawText(m_hFont, _DXG_POINT(fTx0, fTy), sShow.c_str(), m_TextColor, m_fFontScale);
			_ctx.PopClipRect();
		}
		// 캐럿(선택범위 없을 때만).
		if (m_bFocused)
		{
			m_nBlink = (m_nBlink + 1) % 60;
			if (m_nBlink < 30 && !bHasSel_)
			{
				_ctx.FillRect(_DXG_RECT(fTx0 + caretX_(m_uCaret), fTy, 2.0f, fFontH), m_BorderFocusColor);
			}
		}

		// ── 키 입력(포커스 시) ──
		if (m_bFocused && m_bEnabled)
		{
			if (_ctx.IsKeyPressed(DXG_VK_BACK))
			{
				if (bHasSel_) { m_sBuf.erase(uSelL_, uSelR_ - uSelL_); m_uCaret = uSelL_; m_uSelAnchor = uSelL_; }
				else if (m_uCaret > 0) { m_sBuf.erase(m_uCaret - 1, 1); --m_uCaret; m_uSelAnchor = m_uCaret; }
			}
			if (_ctx.IsKeyPressed(DXG_VK_DELETE))
			{
				if (bHasSel_) { m_sBuf.erase(uSelL_, uSelR_ - uSelL_); m_uCaret = uSelL_; m_uSelAnchor = uSelL_; }
				else if (m_uCaret < m_sBuf.size()) { m_sBuf.erase(m_uCaret, 1); }
			}
			const wchar_t* pTxt = _ctx.PollTextInput();
			if (pTxt != nullptr)
			{
				if (bHasSel_) { m_sBuf.erase(uSelL_, uSelR_ - uSelL_); m_uCaret = uSelL_; m_uSelAnchor = uSelL_; }
				const std::wstring sN(pTxt);
				for (const wchar_t c : sN)
				{
					const bool bDigit_ = (c >= L'0' && c <= L'9');
					const bool bSign_  = (c == L'-' && m_uCaret == 0 && m_sBuf.find(L'-') == std::wstring::npos);
					if (bDigit_ || bSign_) { m_sBuf.insert(m_uCaret, 1, c); ++m_uCaret; }
				}
				m_uSelAnchor = m_uCaret;
			}
			if (_ctx.IsKeyPressed(DXG_VK_LEFT)  && m_uCaret > 0)            { --m_uCaret; m_uSelAnchor = m_uCaret; }
			if (_ctx.IsKeyPressed(DXG_VK_RIGHT) && m_uCaret < m_sBuf.size()) { ++m_uCaret; m_uSelAnchor = m_uCaret; }
			if (_ctx.IsKeyPressed(DXG_VK_HOME)) { m_uCaret = 0; m_uSelAnchor = 0; }
			if (_ctx.IsKeyPressed(DXG_VK_END))  { m_uCaret = m_sBuf.size(); m_uSelAnchor = m_uCaret; }
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
