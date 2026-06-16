// DxgEditbox.cpp
#include "DxgEditbox.h"
#include "DxgKeys.h"

#include <cstdio>
#include <cwchar>
#include <cstdlib>


namespace dxgui
{

	std::wstring C_DXG_EDITBOX::DataToString_() const
	{
		if (m_pData == nullptr) { return std::wstring(); }
		wchar_t buf_[64];
		switch (m_DataType)
		{
		case DXG_EDIT_INT32:
			std::swprintf(buf_, 64, L"%d",
				*static_cast<int32_t*>(m_pData));
			return std::wstring(buf_);
		case DXG_EDIT_INT64:
			std::swprintf(buf_, 64, L"%lld",
				static_cast<long long>(*static_cast<int64_t*>(m_pData)));
			return std::wstring(buf_);
		case DXG_EDIT_FLOAT:
			std::swprintf(buf_, 64, L"%g",
				static_cast<double>(*static_cast<float*>(m_pData)));
			return std::wstring(buf_);
		case DXG_EDIT_TEXT:
			return *static_cast<std::wstring*>(m_pData);
		case DXG_EDIT_YMD:
		{
			const int32_t v_ = *static_cast<int32_t*>(m_pData);
			if (v_ <= 0) { return std::wstring(); }
			const int y_ = v_ / 10000;
			const int m_ = (v_ / 100) % 100;
			const int d_ = v_ % 100;
			std::swprintf(buf_, 64, L"%04d-%02d-%02d", y_, m_, d_);
			return std::wstring(buf_);
		}
		}
		return std::wstring();
	}


	void C_DXG_EDITBOX::StringToData_()
	{
		if (m_pData == nullptr) { return; }
		switch (m_DataType)
		{
		case DXG_EDIT_INT32:
			*static_cast<int32_t*>(m_pData) =
				static_cast<int32_t>(std::wcstol(m_sBuffer.c_str(), nullptr, 10));
			break;
		case DXG_EDIT_INT64:
			*static_cast<int64_t*>(m_pData) =
				static_cast<int64_t>(std::wcstoll(m_sBuffer.c_str(), nullptr, 10));
			break;
		case DXG_EDIT_FLOAT:
			*static_cast<float*>(m_pData) =
				static_cast<float>(std::wcstod(m_sBuffer.c_str(), nullptr));
			break;
		case DXG_EDIT_TEXT:
			*static_cast<std::wstring*>(m_pData) = m_sBuffer;
			break;
		case DXG_EDIT_YMD:
		{
			std::wstring digits_;
			for (wchar_t c_ : m_sBuffer)
			{
				if (c_ >= L'0' && c_ <= L'9') { digits_.push_back(c_); }
			}
			if (digits_.size() == 8)
			{
				*static_cast<int32_t*>(m_pData) =
					static_cast<int32_t>(std::wcstol(digits_.c_str(), nullptr, 10));
			}
			// 불완전한 입력은 데이터 변경 안 함 (기존 값 보존).
			break;
		}
		}
	}


	// 숫자만 추출 후 "YYYY-MM-DD" 형식 재구성. 8자리 초과는 잘림.
	void C_DXG_EDITBOX::NormalizeYmdBuffer_()
	{
		std::wstring digits_;
		for (wchar_t c_ : m_sBuffer)
		{
			if (c_ >= L'0' && c_ <= L'9') { digits_.push_back(c_); }
		}
		if (digits_.size() > 8) { digits_.resize(8); }
		std::wstring out_;
		for (size_t i_ = 0; i_ < digits_.size(); ++i_)
		{
			if (i_ == 4 || i_ == 6) { out_.push_back(L'-'); }
			out_.push_back(digits_[i_]);
		}
		m_sBuffer = std::move(out_);
		m_uCaret = m_sBuffer.size();
	}


	void C_DXG_EDITBOX::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const bool  bHover_ = _ctx.IsMouseHovered(abs_);
		const float fTx0_   = abs_.x + 4.0f;	// 텍스트 좌측 기준 X
		// YMD 는 마스킹 필드 — 캐럿/드래그 미세편집 대신 append+정규화(캐럿=끝) 단순 모드.
		const bool  bYmd_   = (m_DataType == DXG_EDIT_YMD);

		// 캐럿 인덱스(문자 경계) → 픽셀 X(텍스트 좌측 기준). 부분문자열 측정.
		auto caretX_ = [&](size_t _i) -> float
		{
			if (m_hFont == INVALID_FONT || _i == 0 || m_sBuffer.empty()) { return 0.0f; }
			if (_i > m_sBuffer.size()) { _i = m_sBuffer.size(); }
			return _ctx.MeasureText(m_hFont, m_sBuffer.substr(0, _i).c_str(), m_fFontScale).w;
		};
		// 로컬 X(픽셀, 텍스트 좌측 기준) → 가장 가까운 문자 경계 인덱스.
		auto hitIdx_ = [&](float _localX) -> size_t
		{
			size_t uBest_ = 0;
			float  fBestD_ = (_localX < 0.0f) ? -_localX : _localX;
			for (size_t i_ = 1; i_ <= m_sBuffer.size(); ++i_)
			{
				const float fW_ = caretX_(i_);
				const float fD_ = (_localX < fW_) ? (fW_ - _localX) : (_localX - fW_);
				if (fD_ <= fBestD_) { fBestD_ = fD_; uBest_ = i_; }
			}
			return uBest_;
		};

		// ── 입력: 더블클릭(전체선택) / 클릭(캐럿) / 드래그(범위) / 외부클릭(커밋) ──
		if (m_bEnabled && bHover_ && _ctx.IsMouseDoubleClicked(DXG_MOUSE_LEFT))
		{
			if (!m_bFocused) { m_bFocused = true; m_sBuffer = DataToString_(); m_nBlinkCnt = 0; }
			m_uSelAnchor = 0; m_uCaret = m_sBuffer.size();	// 전체선택
			m_bDragSel = false;
		}
		else if (_ctx.IsMouseClicked(DXG_MOUSE_LEFT))
		{
			if (bHover_ && m_bEnabled)
			{
				if (!m_bFocused) { m_bFocused = true; m_sBuffer = DataToString_(); m_nBlinkCnt = 0; }
				const size_t uHit_ = bYmd_ ? m_sBuffer.size()
					: hitIdx_(_ctx.GetMousePos().x - fTx0_);
				m_uCaret = uHit_; m_uSelAnchor = uHit_;	// 캐럿만(선택 해제)
				m_bDragSel = !bYmd_;	// YMD 제외 드래그선택 개시
			}
			else if (m_bFocused)	// 박스 밖 클릭 → 커밋 + 포커스 해제
			{
				StringToData_();
				m_bFocused = false;
				m_bDragSel = false;
			}
		}
		// 드래그 중 — 앵커 고정, 캐럿이 마우스를 따라가며 범위 확장.
		if (m_bDragSel && m_bFocused && _ctx.IsMouseDown(DXG_MOUSE_LEFT))
		{
			m_uCaret = hitIdx_(_ctx.GetMousePos().x - fTx0_);
		}
		if (_ctx.IsMouseReleased(DXG_MOUSE_LEFT)) { m_bDragSel = false; }

		// 캐럿/앵커 클램프 후 선택범위 산출.
		if (m_uCaret > m_sBuffer.size())     { m_uCaret = m_sBuffer.size(); }
		if (m_uSelAnchor > m_sBuffer.size()) { m_uSelAnchor = m_sBuffer.size(); }
		const size_t uSelL_ = (m_uSelAnchor < m_uCaret) ? m_uSelAnchor : m_uCaret;
		const size_t uSelR_ = (m_uSelAnchor < m_uCaret) ? m_uCaret : m_uSelAnchor;
		const bool   bHasSel_ = (uSelL_ != uSelR_);

		// ── 렌더: 배경/테두리/[선택]/텍스트/[조합]/캐럿 ──
		_ctx.FillRect(abs_, m_BgColor);
		_ctx.DrawRectOutline(abs_,
			m_bFocused ? m_BorderFocusColor : m_BorderColor,
			m_bFocused ? 2.0f : 1.0f);

		const std::wstring sShow_ = m_bFocused ? m_sBuffer : DataToString_();
		const float fFontH_ = _ctx.GetFontHeight(m_hFont, m_fFontScale);
		const float fTextY_ = abs_.y + (abs_.h - fFontH_) * 0.5f;

		_ctx.PushClipRect(abs_);

		// 선택 하이라이트(텍스트 뒤).
		if (m_bFocused && bHasSel_ && m_hFont != INVALID_FONT)
		{
			const float fXL_ = caretX_(uSelL_);
			const float fXR_ = caretX_(uSelR_);
			_ctx.FillRect(_DXG_RECT(fTx0_ + fXL_, fTextY_, fXR_ - fXL_, fFontH_),
				_DXG_COLOR(0xFFB5D2FFu));
		}
		// 텍스트.
		if (!sShow_.empty() && m_hFont != INVALID_FONT)
		{
			_ctx.DrawText(m_hFont, _DXG_POINT(fTx0_, fTextY_), sShow_.c_str(), m_TextColor, m_fFontScale);
		}
		// IME 조합중(미확정) — 캐럿 위치에 밑줄로 표기. 확정은 PollTextInput 으로 들어옴.
		float fCompW_ = 0.0f;
		if (m_bFocused && m_hFont != INVALID_FONT)
		{
			const wchar_t* pComp_ = _ctx.PollComposition();
			if (pComp_ != nullptr && pComp_[0] != L'\0')
			{
				fCompW_ = _ctx.MeasureText(m_hFont, pComp_, m_fFontScale).w;
				const float fCx_ = fTx0_ + caretX_(m_uCaret);
				_ctx.DrawText(m_hFont, _DXG_POINT(fCx_, fTextY_), pComp_, m_BorderFocusColor, m_fFontScale);
				_ctx.DrawLine(_DXG_POINT(fCx_, fTextY_ + fFontH_ - 1.0f),
					_DXG_POINT(fCx_ + fCompW_, fTextY_ + fFontH_ - 1.0f), m_BorderFocusColor, 1.0f);
			}
		}
		// 캐럿 blink — 선택범위가 없을 때만(선택 중엔 하이라이트로 대체). 60 frame 주기.
		if (m_bFocused)
		{
			m_nBlinkCnt = (m_nBlinkCnt + 1) % 60;
			if (m_nBlinkCnt < 30 && !bHasSel_)
			{
				const float fCaretX_ = fTx0_ + caretX_(m_uCaret) + fCompW_;
				_ctx.FillRect(_DXG_RECT(fCaretX_, fTextY_, 2.0f, fFontH_), m_BorderFocusColor);
			}
		}

		_ctx.PopClipRect();

		// ── 키 입력 처리(포커스 시) ──
		if (m_bFocused && m_bEnabled)
		{
			// 백스페이스.
			if (_ctx.IsKeyPressed(DXG_VK_BACK))
			{
				if (bHasSel_)	// 선택범위 → 통째 삭제
				{
					m_sBuffer.erase(uSelL_, uSelR_ - uSelL_);
					m_uCaret = uSelL_; m_uSelAnchor = uSelL_;
				}
				else if (bYmd_ && !m_sBuffer.empty())
				{
					// 숫자 한 자리 후퇴. dash 가 마지막이면 dash + 그 앞 숫자 함께 제거.
					m_sBuffer.pop_back();
					if (!m_sBuffer.empty() && m_sBuffer.back() == L'-') { m_sBuffer.pop_back(); }
					NormalizeYmdBuffer_();
					m_uCaret = m_sBuffer.size(); m_uSelAnchor = m_uCaret;
				}
				else if (m_uCaret > 0)
				{
					m_sBuffer.erase(m_uCaret - 1, 1);	// 캐럿 앞 1 코드유닛 제거
					--m_uCaret; m_uSelAnchor = m_uCaret;
				}
			}
			// Delete — 캐럿 뒤 1자(또는 선택범위). YMD 제외.
			if (_ctx.IsKeyPressed(DXG_VK_DELETE) && !bYmd_)
			{
				if (bHasSel_)
				{
					m_sBuffer.erase(uSelL_, uSelR_ - uSelL_);
					m_uCaret = uSelL_; m_uSelAnchor = uSelL_;
				}
				else if (m_uCaret < m_sBuffer.size())
				{
					m_sBuffer.erase(m_uCaret, 1);
				}
			}
			// 텍스트 입력 큐 흡수(IME 결과 포함) — 선택범위가 있으면 교체 후 캐럿 삽입.
			const wchar_t* pTxt_ = _ctx.PollTextInput();
			if (pTxt_ != nullptr)
			{
				if (bHasSel_)
				{
					m_sBuffer.erase(uSelL_, uSelR_ - uSelL_);
					m_uCaret = uSelL_; m_uSelAnchor = uSelL_;
				}
				const std::wstring sNew_(pTxt_);
				if (bYmd_)
				{
					// 숫자만 받음 + dash 무시(정규화가 자동 삽입). 캐럿=끝.
					for (wchar_t c_ : sNew_)
					{
						if (c_ >= L'0' && c_ <= L'9') { m_sBuffer.push_back(c_); }
					}
					NormalizeYmdBuffer_();
					m_uCaret = m_sBuffer.size(); m_uSelAnchor = m_uCaret;
				}
				else
				{
					for (wchar_t c_ : sNew_)
					{
						bool bOk_;
						if (m_DataType == DXG_EDIT_TEXT)
						{
							bOk_ = (m_sBuffer.size() < m_uTextMax);
						}
						else
						{
							// 숫자 모드 — 숫자/부호/소수점만. 부호는 맨앞, 소수점은 FLOAT+미존재.
							const bool bDigit_ = (c_ >= L'0' && c_ <= L'9');
							const bool bSign_  = (c_ == L'-' && m_uCaret == 0
								&& m_sBuffer.find(L'-') == std::wstring::npos);
							const bool bDot_   = (c_ == L'.' && m_DataType == DXG_EDIT_FLOAT
								&& m_sBuffer.find(L'.') == std::wstring::npos);
							bOk_ = bDigit_ || bSign_ || bDot_;
						}
						if (bOk_)
						{
							m_sBuffer.insert(m_uCaret, 1, c_);
							++m_uCaret;
						}
					}
					m_uSelAnchor = m_uCaret;
				}
			}
			// 좌우 화살표 / Home / End — 캐럿 이동(선택 해제). YMD 제외.
			if (!bYmd_)
			{
				if (_ctx.IsKeyPressed(DXG_VK_LEFT)  && m_uCaret > 0)               { --m_uCaret; m_uSelAnchor = m_uCaret; }
				if (_ctx.IsKeyPressed(DXG_VK_RIGHT) && m_uCaret < m_sBuffer.size()) { ++m_uCaret; m_uSelAnchor = m_uCaret; }
				if (_ctx.IsKeyPressed(DXG_VK_HOME))  { m_uCaret = 0; m_uSelAnchor = 0; }
				if (_ctx.IsKeyPressed(DXG_VK_END))   { m_uCaret = m_sBuffer.size(); m_uSelAnchor = m_uCaret; }
			}
			// 엔터/Tab → 커밋 + 포커스 해제.
			if (_ctx.IsKeyPressed(DXG_VK_RETURN) || _ctx.IsKeyPressed(DXG_VK_TAB))
			{
				StringToData_();
				m_bFocused = false;
			}
		}
	}

} // namespace dxgui
