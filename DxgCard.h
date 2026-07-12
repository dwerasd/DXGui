// DxgCard.h: 카드 컨테이너(패널 파생).
// 문법 = [흰 배경 + 라운드 + 1px 테두리 + 타이틀행(볼드 제목) + 구분선]. 화면은 회색 캔버스 위
// 카드 타일링으로 구성한다. 자식 배치/클리핑은 C_DXG_PANEL 을 그대로 재사용하고, 표면만 교체
// (패널 배경/테두리는 ctor 에서 비활성 - 카드가 라운드 표면을 직접 그린다).
// 우측 액션 버튼은 전용 슬롯 추상화 없이 자식 위젯을 타이틀행 우측 좌표에 두는 방식.
#pragma once

#include "DxgPanel.h"
#include "DxgTheme.h"

#include <string>


namespace dxgui
{

	class C_DXG_CARD : public C_DXG_PANEL
	{
	private:
		std::wstring m_sTitle;
		FontHandle   m_hTitleFont;
		float        m_fTitleH;
		_DXG_COLOR   m_CardBg;
		_DXG_COLOR   m_CardBorder;
		_DXG_COLOR   m_TitleColor;

	public:
		C_DXG_CARD()
			: m_hTitleFont(INVALID_FONT)
			, m_fTitleH(36.0f)
			, m_CardBg(Theme().card)
			, m_CardBorder(Theme().border)
			, m_TitleColor(Theme().textMain)
		{
			this->SetBgColor(_DXG_COLOR(0u));		// 패널 사각 배경 비활성(카드가 라운드로 그림)
			this->SetBorder(_DXG_COLOR(0u), 0.0f);	// 패널 사각 테두리 비활성
		}

		// 제목 = 빈 문자열이면 타이틀행 없음(순수 카드 표면).
		void SetTitle(const std::wstring& _s, FontHandle _hFont)
		{
			m_sTitle = _s;
			m_hTitleFont = _hFont;
		}
		void SetTitleHeight(float _h)     { m_fTitleH = _h; }
		void SetCardColors(_DXG_COLOR _bg, _DXG_COLOR _border)
		{
			m_CardBg = _bg;
			m_CardBorder = _border;
		}

		// 자식 배치 기준 - 타이틀행 아래 + 패딩. 타이틀 없으면 패딩만.
		float ContentTop() const
		{
			return m_sTitle.empty() ? Theme().pad : (m_fTitleH + Theme().pad * 0.5f);
		}
		float TitleHeight() const { return m_sTitle.empty() ? 0.0f : m_fTitleH; }

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_CARD; }
		const char*       GetTypeName() const override { return "card"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
	};

} // namespace dxgui
