// DxgWidget.h: GUI 위젯 추상 베이스(leaf). 컨테이너는 C_DXG_PANEL.
// 위치/크기/키/표시명/폰트스케일/visible/enable 공통 + Render(IDrawContext&,_origin) virtual.
// 콘크리트: C_DXG_LABEL / C_DXG_BUTTON / C_DXG_EDITBOX / C_DXG_CHECKBOX / C_DXG_PANEL.
// 표시명(m_sName)은 wstring(UTF-16) — 설계 §7 wstring 단일.
#pragma once

#include "DxgTypes.h"

#include <string>


namespace dxgui
{

	class IDrawContext;


	enum E_DXG_WIDGET_TYPE : uint8_t
	{
		DXG_WIDGET_UNKNOWN  = 0,
		DXG_WIDGET_LABEL    = 1,
		DXG_WIDGET_BUTTON   = 2,
		DXG_WIDGET_EDITBOX  = 3,
		DXG_WIDGET_CHECKBOX = 4,
		DXG_WIDGET_PANEL    = 5,
	};


	// 위젯 추상 베이스. 좌표는 부모(패널) 원점 기준 px.
	// 부모가 매 프레임 Render(ctx, _origin) 호출 — 위젯은 m_Rect 더해 절대 좌표 산출.
	class C_DXG_WIDGET
	{
	protected:
		std::string  m_sKey;        // 영속 키(ASCII) — 외부 영속 시 식별자
		std::wstring m_sName;       // 표시명(라벨/버튼 텍스트). UTF-16.
		_DXG_RECT    m_Rect;        // 위치/크기(부모 원점 기준 px)
		float        m_fFontScale;  // 1.0 = 기본
		bool         m_bVisible;
		bool         m_bEnabled;

	public:
		C_DXG_WIDGET()
			: m_fFontScale(1.0f)
			, m_bVisible(true)
			, m_bEnabled(true)
		{
		}
		virtual ~C_DXG_WIDGET() = default;

		C_DXG_WIDGET(const C_DXG_WIDGET&) = delete;
		C_DXG_WIDGET& operator=(const C_DXG_WIDGET&) = delete;

		// ── 공통 속성 ──
		const std::string& GetKey()  const { return m_sKey; }
		void               SetKey(const std::string& _s) { m_sKey = _s; }

		const std::wstring& GetName() const { return m_sName; }
		void                SetName(const std::wstring& _s) { m_sName = _s; }

		const _DXG_RECT& GetRect() const { return m_Rect; }
		void             SetRect(const _DXG_RECT& _r) { m_Rect = _r; }
		void             SetPos(float _x, float _y) { m_Rect.x = _x; m_Rect.y = _y; }
		void             SetSize(float _w, float _h) { m_Rect.w = _w; m_Rect.h = _h; }

		float GetFontScale() const { return m_fFontScale; }
		void  SetFontScale(float _s) { m_fFontScale = _s; }

		bool IsVisible() const { return m_bVisible; }
		void SetVisible(bool _b) { m_bVisible = _b; }

		bool IsEnabled() const { return m_bEnabled; }
		void SetEnabled(bool _b) { m_bEnabled = _b; }

		// ── 타입 식별(직렬화/팩토리 용) ──
		virtual E_DXG_WIDGET_TYPE GetType() const = 0;
		virtual const char* GetTypeName() const = 0;	// "label" / "button" / ...

		// ── 렌더 / 입력 ──
		// 부모가 _origin(절대 screen pos) 전달 — 위젯은 m_Rect 더해 절대 좌표.
		virtual void Render(IDrawContext& _ctx, _DXG_POINT _origin) = 0;

		// 현재 프레임의 절대 사각형. 자식/hit-test 에서 사용.
		_DXG_RECT AbsRect(_DXG_POINT _origin) const
		{
			return _DXG_RECT(_origin.x + m_Rect.x, _origin.y + m_Rect.y,
			                 m_Rect.w, m_Rect.h);
		}
	};

} // namespace dxgui
