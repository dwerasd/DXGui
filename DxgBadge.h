// DxgBadge.h: 배지(pill) 위젯 + 배지 색 어휘.
// 문법 = [본색 15% 틴트 배경 pill] + [본색 텍스트]. 상태/등락을 텍스트색만으로 쓰지 않고
// 감싸서 시선을 잡는다. 그리드의 배지 셀도 동일 어휘(E_DXG_BADGE_KIND)를 공유한다.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"
#include "DxgTheme.h"

#include <string>


namespace dxgui
{

	enum E_DXG_BADGE_KIND : uint8_t
	{
		DXG_BADGE_SUCCESS = 0,
		DXG_BADGE_INFO    = 1,
		DXG_BADGE_WARNING = 2,
		DXG_BADGE_DANGER  = 3,
		DXG_BADGE_NEUTRAL = 4,
		DXG_BADGE_UP      = 5,	// 상승(적)
		DXG_BADGE_DOWN    = 6,	// 하락(청)
	};


	// 배지 본색 - 테마 시맨틱 색 매핑. 배경은 ThemeTint(본색).
	inline _DXG_COLOR BadgeColor(E_DXG_BADGE_KIND _k)
	{
		switch (_k)
		{
		case DXG_BADGE_SUCCESS: return Theme().success;
		case DXG_BADGE_INFO:    return Theme().accent;
		case DXG_BADGE_WARNING: return Theme().warning;
		case DXG_BADGE_DANGER:  return Theme().danger;
		case DXG_BADGE_UP:      return Theme().up;
		case DXG_BADGE_DOWN:    return Theme().down;
		default:                return Theme().neutral;
		}
	}


	class C_DXG_BADGE : public C_DXG_WIDGET
	{
	private:
		E_DXG_BADGE_KIND m_Kind;
		FontHandle       m_hFont;

	public:
		C_DXG_BADGE()
			: m_Kind(DXG_BADGE_NEUTRAL)
			, m_hFont(INVALID_FONT)
		{
		}

		void SetText(const std::wstring& _s) { m_sName = _s; }
		void SetKind(E_DXG_BADGE_KIND _k)    { m_Kind = _k; }
		void SetFont(FontHandle _h)          { m_hFont = _h; }
		E_DXG_BADGE_KIND Kind() const        { return m_Kind; }

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_BADGE; }
		const char*       GetTypeName() const override { return "badge"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
	};

} // namespace dxgui
