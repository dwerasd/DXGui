// DxgLabel.cpp
#include "DxgLabel.h"


namespace dxgui
{

	void C_DXG_LABEL::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible || m_sName.empty() || m_hFont == INVALID_FONT)
		{
			return;
		}

		// 멀티라인('\n' 포함) — 라인별 가로정렬 + 블록 단위 세로정렬. 라인 간격=폰트 줄높이.
		if (m_sName.find(L'\n') != std::wstring::npos)
		{
			this->renderMultiline_(_ctx, _origin);
			return;
		}

		// ── 단일행(기존 동작 보존) ──
		const _DXG_SIZE sz_ = _ctx.MeasureText(m_hFont, m_sName.c_str(), m_fFontScale);

		float fX_ = _origin.x + m_Rect.x;
		float fY_ = _origin.y + m_Rect.y;
		if (m_Rect.w > 0.0f)
		{
			if (m_Align == DXG_TEXT_ALIGN_CENTER)
			{
				fX_ += (m_Rect.w - sz_.w) * 0.5f;
			}
			else if (m_Align == DXG_TEXT_ALIGN_RIGHT)
			{
				fX_ += (m_Rect.w - sz_.w);
			}
		}
		if (m_Rect.h > 0.0f)
		{
			if (m_VAlign == DXG_VALIGN_CENTER)
			{
				fY_ += (m_Rect.h - sz_.h) * 0.5f;
			}
			else if (m_VAlign == DXG_VALIGN_BOTTOM)
			{
				fY_ += (m_Rect.h - sz_.h);
			}
		}
		_ctx.DrawText(m_hFont, _DXG_POINT(fX_, fY_),
			m_sName.c_str(), m_Color, m_fFontScale);
	}

	// 멀티라인 렌더 — '\n' 분할. 줄높이=GetFontHeight, 블록높이=줄높이*줄수.
	// 세로정렬은 블록 기준, 가로정렬은 라인별. 빈 줄도 한 줄 높이 차지.
	void C_DXG_LABEL::renderMultiline_(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		const float fLineH = _ctx.GetFontHeight(m_hFont, m_fFontScale);

		size_t nLines = 1;
		for (const wchar_t c : m_sName) { if (c == L'\n') { ++nLines; } }
		const float fBlockH = fLineH * static_cast<float>(nLines);

		float fYBlock = _origin.y + m_Rect.y;
		if (m_Rect.h > 0.0f)
		{
			if (m_VAlign == DXG_VALIGN_CENTER)      { fYBlock += (m_Rect.h - fBlockH) * 0.5f; }
			else if (m_VAlign == DXG_VALIGN_BOTTOM) { fYBlock += (m_Rect.h - fBlockH); }
		}

		size_t nStart = 0;
		int    nIdx = 0;
		while (true)
		{
			const size_t nNl = m_sName.find(L'\n', nStart);
			const std::wstring sLine = m_sName.substr(nStart,
				(nNl == std::wstring::npos) ? std::wstring::npos : (nNl - nStart));

			float fX = _origin.x + m_Rect.x;
			if (!sLine.empty() && m_Rect.w > 0.0f
				&& (m_Align == DXG_TEXT_ALIGN_CENTER || m_Align == DXG_TEXT_ALIGN_RIGHT))
			{
				const _DXG_SIZE sz = _ctx.MeasureText(m_hFont, sLine.c_str(), m_fFontScale);
				if (m_Align == DXG_TEXT_ALIGN_CENTER) { fX += (m_Rect.w - sz.w) * 0.5f; }
				else                                  { fX += (m_Rect.w - sz.w); }
			}
			const float fY = fYBlock + static_cast<float>(nIdx) * fLineH;
			if (!sLine.empty())
			{
				_ctx.DrawText(m_hFont, _DXG_POINT(fX, fY), sLine.c_str(), m_Color, m_fFontScale);
			}

			if (nNl == std::wstring::npos) { break; }
			nStart = nNl + 1;
			++nIdx;
		}
	}

} // namespace dxgui
