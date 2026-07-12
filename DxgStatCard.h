// DxgStatCard.h: KPI 스탯 카드(독립 위젯 - 타이틀행이 없어 카드 파생 불필요).
// 구성 = [카드 표면] + (옵션)좌측 원형 아이콘 배지 + 캡션(11) + 큰 값(22 bold) + 델타 배지(▲▼)
//        + 우측 하단 스파크라인(최근 N개, min/max 정규화 폴리라인).
// 값 포맷팅(통화/자릿수)은 호출부 책임 - 위젯은 문자열을 그대로 그린다.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"
#include "DxgTheme.h"

#include <string>


namespace dxgui
{

	class C_DXG_STAT_CARD : public C_DXG_WIDGET
	{
	public:
		static constexpr int kMaxSeries = 64;	// 스파크라인 최대 표본(프레임당 재계산 무방한 규모)

	private:
		std::wstring m_sCaption;
		std::wstring m_sValue;
		float        m_fDeltaPct;		// 0 = 델타 미표시
		bool         m_bHasDelta;
		float        m_Series[kMaxSeries];
		int          m_nSeries;

		FontHandle   m_hCaption;
		FontHandle   m_hValue;
		FontHandle   m_hBody;

		wchar_t      m_cIcon;			// 0 = 아이콘 배지 없음
		_DXG_COLOR   m_IconColor;
		FontHandle   m_hIconFont;
		uint32_t     m_uIconFontPx;

	public:
		C_DXG_STAT_CARD()
			: m_fDeltaPct(0.0f)
			, m_bHasDelta(false)
			, m_nSeries(0)
			, m_hCaption(INVALID_FONT)
			, m_hValue(INVALID_FONT)
			, m_hBody(INVALID_FONT)
			, m_cIcon(0)
			, m_IconColor(Theme().accent)
			, m_hIconFont(INVALID_FONT)
			, m_uIconFontPx(0u)
		{
			for (int i = 0; i < kMaxSeries; ++i) { m_Series[i] = 0.0f; }
		}

		void SetCaption(const std::wstring& _s) { m_sCaption = _s; }
		void SetValue(const std::wstring& _s)   { m_sValue = _s; }
		void SetDelta(float _fPct)              { m_fDeltaPct = _fPct; m_bHasDelta = true; }
		void ClearDelta()                       { m_bHasDelta = false; }
		void SetSeries(const float* _p, int _n);	// 내부 복사(최대 kMaxSeries, 뒤에서 자름)
		void ClearSeries()                      { m_nSeries = 0; }
		void SetFonts(FontHandle _hCaption, FontHandle _hValue, FontHandle _hBody)
		{
			m_hCaption = _hCaption;
			m_hValue = _hValue;
			m_hBody = _hBody;
		}
		// 좌측 원형 틴트 배지 + MDL2 글리프(옵션).
		void SetIconBadge(wchar_t _cGlyph, _DXG_COLOR _color)
		{
			m_cIcon = _cGlyph;
			m_IconColor = _color;
		}

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_STATCARD; }
		const char*       GetTypeName() const override { return "statcard"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
	};

} // namespace dxgui
