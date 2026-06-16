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
		const bool bHover_ = _ctx.IsMouseHovered(abs_);

		// 포커스 처리.
		if (_ctx.IsMouseClicked(DXG_MOUSE_LEFT))
		{
			if (bHover_ && m_bEnabled)
			{
				if (!m_bFocused)
				{
					m_bFocused = true;
					m_sBuffer  = DataToString_();
					m_uCaret   = m_sBuffer.size();   // caret = 끝.
					m_bSelected = false;
					m_nBlinkCnt = 0;
				}
			}
			else if (m_bFocused)
			{
				StringToData_();
				m_bFocused = false;
				m_bSelected = false;
			}
		}

		// 배경 + 테두리. focused 면 BorderFocusColor + 2px.
		_ctx.FillRect(abs_, m_BgColor);
		_ctx.DrawRectOutline(abs_,
			m_bFocused ? m_BorderFocusColor : m_BorderColor,
			m_bFocused ? 2.0f : 1.0f);

		// 표시 문자열 — 포커스 시 편집 버퍼, 아니면 바인딩 변수 값.
		const std::wstring sShow_ = m_bFocused ? m_sBuffer : DataToString_();
		const float fFontH_ = _ctx.GetFontHeight(m_hFont, m_fFontScale);
		const float fTextY_ = abs_.y + (abs_.h - fFontH_) * 0.5f;
		float fTextW_ = 0.0f;
		if (!sShow_.empty() && m_hFont != INVALID_FONT)
		{
			const _DXG_SIZE szText_ = _ctx.MeasureText(m_hFont, sShow_.c_str(), m_fFontScale);
			fTextW_ = szText_.w;
			_ctx.DrawText(m_hFont,
				_DXG_POINT(abs_.x + 4.0f, fTextY_),
				sShow_.c_str(), m_TextColor, m_fFontScale);
		}
		// IME 조합중(미확정) 인라인 프리뷰 — 버퍼 뒤에 밑줄로 표기. 확정은 PollTextInput 으로 들어옴.
		float fCompW_ = 0.0f;
		if (m_bFocused && m_hFont != INVALID_FONT)
		{
			const wchar_t* pComp_ = _ctx.PollComposition();
			if (pComp_ != nullptr && pComp_[0] != L'\0')
			{
				const _DXG_SIZE szComp_ = _ctx.MeasureText(m_hFont, pComp_, m_fFontScale);
				fCompW_ = szComp_.w;
				const float fCx_ = abs_.x + 4.0f + fTextW_;
				_ctx.DrawText(m_hFont, _DXG_POINT(fCx_, fTextY_), pComp_, m_BorderFocusColor, m_fFontScale);
				_ctx.DrawLine(_DXG_POINT(fCx_, fTextY_ + fFontH_ - 1.0f),
					_DXG_POINT(fCx_ + fCompW_, fTextY_ + fFontH_ - 1.0f), m_BorderFocusColor, 1.0f);
			}
		}

		// caret blink — focused 시 60 frame 주기 (30 frame 보임, 30 frame 숨김).
		// 두께 2.5px, BorderFocusColor (파랑) 로 명확. caret X = buffer + 조합중 끝.
		if (m_bFocused)
		{
			m_nBlinkCnt = (m_nBlinkCnt + 1) % 60;
			if (m_nBlinkCnt < 30)
			{
				_ctx.FillRect(_DXG_RECT(abs_.x + 4.0f + fTextW_ + fCompW_, fTextY_, 2.5f, fFontH_),
					m_BorderFocusColor);
			}
		}

		// 키 입력 처리(포커스 시).
		if (m_bFocused && m_bEnabled)
		{
			// 백스페이스.
			if (_ctx.IsKeyPressed(DXG_VK_BACK) && !m_sBuffer.empty())
			{
				if (m_DataType == DXG_EDIT_YMD)
				{
					// 숫자 한 자리 후퇴. dash 가 마지막이면 dash + 그 앞 숫자 함께 제거.
					m_sBuffer.pop_back();
					if (!m_sBuffer.empty() && m_sBuffer.back() == L'-')
					{
						m_sBuffer.pop_back();
					}
					NormalizeYmdBuffer_();
				}
				else
				{
					// wchar(UTF-16) 코드유닛 1개 제거. (BMP 문자=1유닛)
					m_sBuffer.pop_back();
					m_uCaret = m_sBuffer.size();
				}
			}
			// 텍스트 입력 큐 흡수 (IME 결과 포함).
			const wchar_t* pTxt_ = _ctx.PollTextInput();
			if (pTxt_ != nullptr)
			{
				const std::wstring sNew_(pTxt_);
				if (m_DataType == DXG_EDIT_TEXT)
				{
					if (m_sBuffer.size() + sNew_.size() <= m_uTextMax)
					{
						m_sBuffer += sNew_;
					}
					m_uCaret = m_sBuffer.size();
				}
				else if (m_DataType == DXG_EDIT_YMD)
				{
					// 숫자만 받음 + dash 는 무시 (정규화가 자동 삽입).
					for (wchar_t c_ : sNew_)
					{
						if (c_ >= L'0' && c_ <= L'9') { m_sBuffer.push_back(c_); }
					}
					NormalizeYmdBuffer_();
				}
				else
				{
					// 숫자 모드 — 숫자/부호/소수점만 허용.
					for (wchar_t c_ : sNew_)
					{
						const bool bDigit_ = (c_ >= L'0' && c_ <= L'9');
						const bool bSign_  = (c_ == L'-' && m_sBuffer.empty());
						const bool bDot_   = (c_ == L'.' && m_DataType == DXG_EDIT_FLOAT
							&& m_sBuffer.find(L'.') == std::wstring::npos);
						if (bDigit_ || bSign_ || bDot_) { m_sBuffer.push_back(c_); }
					}
					m_uCaret = m_sBuffer.size();
				}
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
