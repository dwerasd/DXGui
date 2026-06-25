// DxgColorField.h: 색 입력 위젯(스와치 + 팝업[프리셋 팔레트 + hex 직접입력/붙여넣기]).
// 닫힘=현재 색 스와치. 클릭=팝업 토글. 팝업은 오버레이 패스(최상위)로 렌더(콤보와 동일 모달 규약).
// 커스텀 색은 hex(RRGGBB/AARRGGBB) 타이핑 또는 Ctrl+V 붙여넣기(colorcop/트레이딩뷰 값 복붙). 스포이드 없음.
// 재사용 대상: 거래마커 색, 향후 지표 선 색 등 색 입력이 필요한 모든 곳.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"

#include <string>
#include <functional>


namespace dxgui
{

	class C_DXG_COLOR_FIELD : public C_DXG_WIDGET
	{
	private:
		unsigned int m_Color;		// ARGB(0xAARRGGBB)
		bool         m_bOpen;
		bool         m_bJustOpened;	// 연 프레임 외부클릭 닫기 1프레임 무시
		FontHandle   m_hFont;
		std::wstring m_sHex;		// hex 편집 버퍼("RRGGBB"/"AARRGGBB", '#' 미포함, 대문자)
		std::wstring m_sR, m_sG, m_sB;	// R/G/B 10진 편집 버퍼(0~255)
		int          m_nFocus;		// 입력 포커스 필드: 0=hex, 1=R, 2=G, 3=B

		_DXG_COLOR   m_BorderColor;
		_DXG_COLOR   m_BorderOpenColor;
		_DXG_COLOR   m_PopupBgColor;
		_DXG_COLOR   m_TextColor;

		std::function<void(unsigned int)> m_OnChange;

	public:
		C_DXG_COLOR_FIELD()
			: m_Color(0xFFFFFFFFu)
			, m_bOpen(false)
			, m_bJustOpened(false)
			, m_hFont(INVALID_FONT)
			, m_nFocus(0)
			, m_BorderColor(0xFF8896A8u)
			, m_BorderOpenColor(0xFF236EE0u)
			, m_PopupBgColor(0xFFFFFFFFu)
			, m_TextColor(0xFF222838u)
		{
		}

		void SetFont(FontHandle _h)        { m_hFont = _h; }
		void SetColor(unsigned int _argb)  { m_Color = _argb; syncBuffersFromColor_(); }
		unsigned int GetColor() const      { return m_Color; }
		void SetOnChange(std::function<void(unsigned int)> _fn) { m_OnChange = std::move(_fn); }

		bool IsOpen() const { return m_bOpen; }
		void Close()        { m_bOpen = false; m_bJustOpened = false; }
		bool IsModalActive() const override { return m_bOpen; }		// 열린 동안 모달(아래 위젯 클릭 차단)

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_COLORFIELD; }
		const char*       GetTypeName() const override { return "colorfield"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
		void RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin) override;

	private:
		static bool IsHexCh_(wchar_t _c)
		{
			return (_c >= L'0' && _c <= L'9') || (_c >= L'A' && _c <= L'F') || (_c >= L'a' && _c <= L'f');
		}
		static std::wstring HexFromColor_(unsigned int _argb);	// 알파 FF → "RRGGBB", 그 외 → "AARRGGBB"
		void applyHex_();			// m_sHex 가 6/8 자리면 m_Color 갱신 + OnChange(라이브 프리뷰)
		void applyRgb_();			// R/G/B 버퍼(0~255) → m_Color 갱신 + OnChange(라이브)
		void pickColor_(unsigned int _argb);	// 팔레트 선택 → 색/버퍼 갱신 + OnChange + 닫기
		void syncBuffersFromColor_();			// m_Color → hex/R/G/B 버퍼 전부 동기
		void syncOthersFromColor_();			// m_Color → 포커스 아닌 필드 버퍼만 동기(편집 중 보존)
	};

} // namespace dxgui
