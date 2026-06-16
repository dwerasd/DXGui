// DxgLabel.h: 텍스트 라벨 위젯.
// 색상 + 정렬(L/C/R, T/M/B) + 폰트 스케일. 클릭/포커스 없음.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"


namespace dxgui
{

	class C_DXG_LABEL : public C_DXG_WIDGET
	{
	private:
		_DXG_COLOR        m_Color;
		E_DXG_TEXT_ALIGN  m_Align;
		E_DXG_VTEXT_ALIGN m_VAlign;
		FontHandle        m_hFont;	// 호스트가 등록한 폰트 핸들

	public:
		C_DXG_LABEL()
			: m_Color(0xFF222838u)
			, m_Align(DXG_TEXT_ALIGN_LEFT)
			, m_VAlign(DXG_VALIGN_TOP)
			, m_hFont(INVALID_FONT)
		{
		}

		// 속성 setter
		void SetColor(_DXG_COLOR _c)         { m_Color = _c; }
		void SetAlign(E_DXG_TEXT_ALIGN _a)   { m_Align = _a; }
		void SetVAlign(E_DXG_VTEXT_ALIGN _a) { m_VAlign = _a; }
		void SetFont(FontHandle _h)          { m_hFont = _h; }

		_DXG_COLOR        GetColor()  const { return m_Color; }
		E_DXG_TEXT_ALIGN  GetAlign()  const { return m_Align; }
		E_DXG_VTEXT_ALIGN GetVAlign() const { return m_VAlign; }
		FontHandle        GetFont()   const { return m_hFont; }

		// 타입
		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_LABEL; }
		const char*       GetTypeName() const override { return "label"; }

		// 렌더 — m_sName(wstring) 을 정렬 후 DrawText. '\n' 포함 시 멀티라인.
		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;

	private:
		void renderMultiline_(IDrawContext& _ctx, _DXG_POINT _origin);	// '\n' 분할 렌더
	};

} // namespace dxgui
