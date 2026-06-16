// DxgSpinner.h: 숫자 스피너 위젯. int* 바인딩 + 우측 상/하 증감 버튼(step). 표시 전용(타이핑 X).
// 수량/가격 등 미세조정 입력에. 텍스트 직접편집은 C_DXG_EDITBOX 병행 사용.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"

#include <functional>


namespace dxgui
{

	class C_DXG_SPINNER : public C_DXG_WIDGET
	{
	private:
		int*       m_pData;
		int        m_nMin, m_nMax, m_nStep;
		FontHandle m_hFont;
		_DXG_COLOR m_BgColor, m_TextColor, m_BorderColor, m_BorderFocusColor, m_BtnBg, m_BtnHover, m_ArrowColor;
		std::function<void(int)> m_OnChange;

		// 텍스트 편집(타이핑) 상태 — 스핀박스(타이핑 + ▲▼ 둘 다).
		bool         m_bFocused;
		std::wstring m_sBuf;
		size_t       m_uCaret;		// 캐럿 위치(선택 끝점)
		size_t       m_uSelAnchor;	// 선택 앵커(==m_uCaret 이면 선택 없음)
		bool         m_bDragSel;	// 드래그 범위선택 중
		int          m_nBlink;

		void setValue_(int _v)	// 클램프 후 적용 + onChange
		{
			if (m_pData == nullptr) { return; }
			if (_v < m_nMin) { _v = m_nMin; }
			if (_v > m_nMax) { _v = m_nMax; }
			if (_v != *m_pData) { *m_pData = _v; if (m_OnChange) { m_OnChange(_v); } }
		}
		void commitBuf_();	// 버퍼 → 값

		void step_(int _nDir)
		{
			if (m_pData == nullptr) { return; }
			if (m_bFocused) { this->commitBuf_(); }	// 타이핑 중이면 먼저 반영
			long long v = static_cast<long long>(*m_pData) + static_cast<long long>(_nDir) * m_nStep;
			if (v < m_nMin) { v = m_nMin; }
			if (v > m_nMax) { v = m_nMax; }
			this->setValue_(static_cast<int>(v));
		}

	public:
		C_DXG_SPINNER()
			: m_pData(nullptr)
			, m_nMin(0), m_nMax(1000000000), m_nStep(1)
			, m_hFont(INVALID_FONT)
			, m_BgColor(0xFFFFFFFFu)
			, m_TextColor(0xFF222838u)
			, m_BorderColor(0xFF8896A8u)
			, m_BorderFocusColor(0xFF236EE0u)
			, m_BtnBg(0xFFEDF1F6u)
			, m_BtnHover(0xFFD6E4FFu)
			, m_ArrowColor(0xFF5A6678u)
			, m_bFocused(false)
			, m_uCaret(0)
			, m_uSelAnchor(0)
			, m_bDragSel(false)
			, m_nBlink(0)
		{
		}

		void Bind(int* _p)                  { m_pData = _p; }
		void SetRange(int _min, int _max)   { m_nMin = _min; m_nMax = _max; }
		void SetStep(int _s)                { m_nStep = (_s > 0) ? _s : 1; }
		void SetFont(FontHandle _h)         { m_hFont = _h; }
		void SetOnChange(std::function<void(int)> _fn) { m_OnChange = std::move(_fn); }

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_SPINNER; }
		const char*       GetTypeName() const override { return "spinner"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
	};

} // namespace dxgui
