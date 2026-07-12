// DxgRadio.h: 라디오 버튼 위젯. int* 그룹값에 바인딩 — 같은 포인터를 공유하는 라디오끼리 배타 선택.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"
#include "DxgTheme.h"

#include <functional>


namespace dxgui
{

	class C_DXG_RADIO : public C_DXG_WIDGET
	{
	private:
		int*       m_pData;		// 그룹 공유 값(이 값 == m_nValue 면 선택)
		int        m_nValue;	// 이 라디오의 값
		FontHandle m_hFont;
		float      m_fSize;		// 원 지름(px)
		_DXG_COLOR m_TextColor, m_RingColor, m_DotColor;
		std::function<void(int)> m_OnChange;	// 선택 변경 시 새 값으로 호출
		bool       m_bPressedInside{ false };

	public:
		C_DXG_RADIO()
			: m_pData(nullptr), m_nValue(0)
			, m_hFont(INVALID_FONT)
			, m_fSize(16.0f)
			, m_TextColor(Theme().textMain)
			, m_RingColor(0xFF8896A8u)
			, m_DotColor(Theme().accent)
		{
		}

		void Bind(int* _pData, int _nValue) { m_pData = _pData; m_nValue = _nValue; }
		void SetFont(FontHandle _h)         { m_hFont = _h; }
		void SetSize(float _f)              { m_fSize = _f; }
		void SetTextColor(_DXG_COLOR _c)    { m_TextColor = _c; }
		void SetOnChange(std::function<void(int)> _fn) { m_OnChange = std::move(_fn); }

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_RADIO; }
		const char*       GetTypeName() const override { return "radio"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
	};

} // namespace dxgui
