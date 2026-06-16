// DxgSpinner.cpp
#include "DxgSpinner.h"

#include <cstdio>


namespace dxgui
{

	void C_DXG_SPINNER::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		if (!m_bVisible) { return; }

		const _DXG_RECT abs_ = AbsRect(_origin);
		const float fBtnW = 18.0f;
		const float fHalf = abs_.h * 0.5f;
		const _DXG_RECT upR_(abs_.x + abs_.w - fBtnW, abs_.y, fBtnW, fHalf);
		const _DXG_RECT dnR_(abs_.x + abs_.w - fBtnW, abs_.y + fHalf, fBtnW, abs_.h - fHalf);

		// 배경 + 테두리.
		_ctx.FillRect(abs_, m_BgColor);
		_ctx.DrawRectOutline(abs_, m_BorderColor, 1.0f);

		// 값 텍스트(좌, 버튼 영역 제외).
		if (m_hFont != INVALID_FONT && m_pData != nullptr)
		{
			wchar_t szv[32]{};
			::swprintf_s(szv, L"%d", *m_pData);
			const _DXG_RECT clip_(abs_.x, abs_.y, abs_.w - fBtnW, abs_.h);
			_ctx.PushClipRect(clip_);
			const _DXG_SIZE sz = _ctx.MeasureText(m_hFont, szv, m_fFontScale);
			const float fTy = abs_.y + (abs_.h - sz.h) * 0.5f;
			_ctx.DrawText(m_hFont, _DXG_POINT(abs_.x + 6.0f, fTy), szv, m_TextColor, m_fFontScale);
			_ctx.PopClipRect();
		}

		// 버튼 배경(hover) + 분리선.
		const bool bHovUp = _ctx.IsMouseHovered(upR_);
		const bool bHovDn = _ctx.IsMouseHovered(dnR_);
		_ctx.FillRect(upR_, bHovUp ? m_BtnHover : m_BtnBg);
		_ctx.FillRect(dnR_, bHovDn ? m_BtnHover : m_BtnBg);
		_ctx.DrawLine(_DXG_POINT(upR_.x, abs_.y), _DXG_POINT(upR_.x, abs_.y + abs_.h), m_BorderColor, 1.0f);
		_ctx.DrawLine(_DXG_POINT(upR_.x, abs_.y + fHalf), _DXG_POINT(abs_.x + abs_.w, abs_.y + fHalf), m_BorderColor, 1.0f);

		// 화살표 ▲ / ▼.
		const float fUx = upR_.x + fBtnW * 0.5f;
		const float fUy = upR_.y + upR_.h * 0.5f;
		const _DXG_POINT triUp[3] = { { fUx - 4.0f, fUy + 2.0f }, { fUx + 4.0f, fUy + 2.0f }, { fUx, fUy - 3.0f } };
		_ctx.FillTriangle(triUp, m_ArrowColor);
		const float fDx = dnR_.x + fBtnW * 0.5f;
		const float fDy = dnR_.y + dnR_.h * 0.5f;
		const _DXG_POINT triDn[3] = { { fDx - 4.0f, fDy - 2.0f }, { fDx + 4.0f, fDy - 2.0f }, { fDx, fDy + 3.0f } };
		_ctx.FillTriangle(triDn, m_ArrowColor);

		// 클릭 → 증감.
		if (m_bEnabled && _ctx.IsMouseClicked(DXG_MOUSE_LEFT))
		{
			if (bHovUp)      { this->step_(+1); }
			else if (bHovDn) { this->step_(-1); }
		}
	}

} // namespace dxgui
