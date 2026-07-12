// DxgStatCard.cpp
#include "DxgStatCard.h"
#include "DxgIcons.h"

#include <cstdio>


namespace dxgui
{

	void C_DXG_STAT_CARD::SetSeries(const float* _p, int _n)
	{
		if (_p == nullptr || _n <= 0) { m_nSeries = 0; return; }
		const int nTake_ = (_n > kMaxSeries) ? kMaxSeries : _n;
		const float* pSrc_ = _p + (_n - nTake_);	// 최근 구간(뒤에서 kMaxSeries)
		for (int i = 0; i < nTake_; ++i) { m_Series[i] = pSrc_[i]; }
		m_nSeries = nTake_;
	}


	void C_DXG_STAT_CARD::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		_ctx.FillRoundRect(abs_, Theme().radiusCard, Theme().card);
		_ctx.DrawRoundRectOutline(abs_, Theme().radiusCard, Theme().border, 1.0f);

		const float fPad_ = 12.0f;
		float fTextX_ = abs_.x + fPad_;

		// 좌측 원형 틴트 배지 + 글리프.
		if (m_cIcon != 0)
		{
			const float fRad_ = 16.0f;
			const _DXG_POINT c_(abs_.x + fPad_ + fRad_, abs_.y + abs_.h * 0.5f);
			_ctx.FillCircle(c_, fRad_, ThemeTint(m_IconColor));

			const uint32_t uPx_ = 16u;
			if (m_hIconFont == INVALID_FONT || m_uIconFontPx != uPx_)
			{
				m_hIconFont = _ctx.RegisterFont(icons::Face, uPx_, false);
				m_uIconFontPx = uPx_;
			}
			if (m_hIconFont != INVALID_FONT)
			{
				const wchar_t szIcon_[2] = { m_cIcon, L'\0' };
				const _DXG_SIZE szi_ = _ctx.MeasureText(m_hIconFont, szIcon_, 1.0f);
				_ctx.DrawText(m_hIconFont,
					_DXG_POINT(c_.x - szi_.w * 0.5f, c_.y - szi_.h * 0.5f),
					szIcon_, m_IconColor, 1.0f);
			}
			fTextX_ = abs_.x + fPad_ + fRad_ * 2.0f + 10.0f;
		}

		// 캡션 / 값.
		float fY_ = abs_.y + fPad_;
		if (!m_sCaption.empty() && m_hCaption != INVALID_FONT)
		{
			_ctx.DrawText(m_hCaption, _DXG_POINT(fTextX_, fY_),
				m_sCaption.c_str(), Theme().textSub, 1.0f);
			fY_ += _ctx.GetFontHeight(m_hCaption, 1.0f) + 4.0f;
		}
		if (!m_sValue.empty() && m_hValue != INVALID_FONT)
		{
			_ctx.DrawText(m_hValue, _DXG_POINT(fTextX_, fY_),
				m_sValue.c_str(), Theme().textMain, 1.0f);
		}

		// 델타 - 삼각 화살표 + 부호색 텍스트(pill 틴트 배경).
		const _DXG_COLOR colDelta_ = (m_fDeltaPct >= 0.0f) ? Theme().up : Theme().down;
		if (m_bHasDelta && m_hBody != INVALID_FONT)
		{
			wchar_t szBuf_[32]{};
			::swprintf_s(szBuf_, L"%+.2f%%", static_cast<double>(m_fDeltaPct));
			const _DXG_SIZE sz_ = _ctx.MeasureText(m_hBody, szBuf_, 1.0f);

			const float fBadgeH_ = sz_.h + 4.0f;
			const float fArrowW_ = 8.0f;
			const float fBadgeW_ = sz_.w + fArrowW_ + 14.0f;
			const _DXG_RECT rcB_(abs_.x + abs_.w - fPad_ - fBadgeW_, abs_.y + fPad_, fBadgeW_, fBadgeH_);
			_ctx.FillRoundRect(rcB_, fBadgeH_ * 0.5f, ThemeTint(colDelta_));

			const float fAx_ = rcB_.x + 6.0f;
			const float fAy_ = rcB_.y + fBadgeH_ * 0.5f;
			_DXG_POINT tri_[3];
			if (m_fDeltaPct >= 0.0f)
			{
				tri_[0] = _DXG_POINT(fAx_ + fArrowW_ * 0.5f, fAy_ - 4.0f);
				tri_[1] = _DXG_POINT(fAx_ + fArrowW_,        fAy_ + 4.0f);
				tri_[2] = _DXG_POINT(fAx_,                   fAy_ + 4.0f);
			}
			else
			{
				tri_[0] = _DXG_POINT(fAx_ + fArrowW_ * 0.5f, fAy_ + 4.0f);
				tri_[1] = _DXG_POINT(fAx_,                   fAy_ - 4.0f);
				tri_[2] = _DXG_POINT(fAx_ + fArrowW_,        fAy_ - 4.0f);
			}
			_ctx.FillTriangle(tri_, colDelta_);
			_ctx.DrawText(m_hBody, _DXG_POINT(fAx_ + fArrowW_ + 4.0f, rcB_.y + 2.0f),
				szBuf_, colDelta_, 1.0f);
		}

		// 스파크라인 - 우측 하단, min/max 정규화 폴리라인(델타 부호색).
		if (m_nSeries >= 2)
		{
			float fLo_ = m_Series[0];
			float fHi_ = m_Series[0];
			for (int i = 1; i < m_nSeries; ++i)
			{
				if (m_Series[i] < fLo_) { fLo_ = m_Series[i]; }
				if (m_Series[i] > fHi_) { fHi_ = m_Series[i]; }
			}
			const float fRange_ = (fHi_ - fLo_ > 1e-9f) ? (fHi_ - fLo_) : 1.0f;

			const float fW_ = 84.0f;
			const float fH_ = 26.0f;
			const float fX0_ = abs_.x + abs_.w - fPad_ - fW_;
			const float fY0_ = abs_.y + abs_.h - fPad_ - fH_;
			if (fX0_ > fTextX_)		// 값 텍스트와 겹치면 생략
			{
				const float fStep_ = fW_ / static_cast<float>(m_nSeries - 1);
				float fPx_ = fX0_;
				float fPy_ = fY0_ + fH_ - (m_Series[0] - fLo_) / fRange_ * fH_;
				for (int i = 1; i < m_nSeries; ++i)
				{
					const float fNx_ = fX0_ + fStep_ * static_cast<float>(i);
					const float fNy_ = fY0_ + fH_ - (m_Series[i] - fLo_) / fRange_ * fH_;
					_ctx.DrawLine(_DXG_POINT(fPx_, fPy_), _DXG_POINT(fNx_, fNy_), colDelta_, 1.5f);
					fPx_ = fNx_;
					fPy_ = fNy_;
				}
			}
		}
	}

} // namespace dxgui
