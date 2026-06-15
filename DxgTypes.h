// DxgTypes.h: DXGui 위젯 시스템 공용 타입 (point/size/rect/color).
// namespace dxgui — 렌더러무관 Unicode 위젯 라이브러리.
// 외부 의존성 0 (cstdint 만). IDrawContext / C_DXG_WIDGET 등이 사용.
// 좌표=float(서브픽셀 보존, D2D/차트 네이티브), 색=u32 ARGB(0xAARRGGBB).
#pragma once

#include <cstdint>


namespace dxgui
{

	struct _DXG_POINT
	{
		float x;
		float y;

		_DXG_POINT() : x(0.0f), y(0.0f) {}
		_DXG_POINT(float _x, float _y) : x(_x), y(_y) {}
	};


	struct _DXG_SIZE
	{
		float w;
		float h;

		_DXG_SIZE() : w(0.0f), h(0.0f) {}
		_DXG_SIZE(float _w, float _h) : w(_w), h(_h) {}
	};


	struct _DXG_RECT
	{
		float x;
		float y;
		float w;
		float h;

		_DXG_RECT() : x(0.0f), y(0.0f), w(0.0f), h(0.0f) {}
		_DXG_RECT(float _x, float _y, float _w, float _h)
			: x(_x), y(_y), w(_w), h(_h)
		{
		}

		bool Contains(float _px, float _py) const
		{
			return _px >= x && _px < (x + w)
			    && _py >= y && _py < (y + h);
		}

		_DXG_POINT GetCenter() const { return _DXG_POINT(x + w * 0.5f, y + h * 0.5f); }
		_DXG_POINT GetTopLeft() const { return _DXG_POINT(x, y); }
		_DXG_SIZE  GetSize() const { return _DXG_SIZE(w, h); }
	};


	// ARGB 32비트 — 상위 바이트 alpha. 렌더러 backend 의 색과 동일 비트레이아웃.
	struct _DXG_COLOR
	{
		uint32_t argb;

		_DXG_COLOR() : argb(0xFFFFFFFFu) {}
		explicit _DXG_COLOR(uint32_t _argb) : argb(_argb) {}

		static _DXG_COLOR Rgba(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255)
		{
			return _DXG_COLOR(
				  (static_cast<uint32_t>(_a) << 24)
				| (static_cast<uint32_t>(_r) << 16)
				| (static_cast<uint32_t>(_g) << 8)
				|  static_cast<uint32_t>(_b));
		}

		uint8_t A() const { return static_cast<uint8_t>((argb >> 24) & 0xFFu); }
		uint8_t R() const { return static_cast<uint8_t>((argb >> 16) & 0xFFu); }
		uint8_t G() const { return static_cast<uint8_t>((argb >>  8) & 0xFFu); }
		uint8_t B() const { return static_cast<uint8_t>( argb        & 0xFFu); }
	};

} // namespace dxgui
