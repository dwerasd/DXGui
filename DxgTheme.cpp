// DxgTheme.cpp: 전역 테마 인스턴스 정의.
#include "DxgTheme.h"


namespace dxgui
{

	_DXG_THEME& Theme()
	{
		static _DXG_THEME s_Theme;
		return s_Theme;
	}

} // namespace dxgui
