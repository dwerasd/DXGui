// DxgEditbox.h: 입력 박스 위젯.
// 타입: INT32 / INT64 / FLOAT / TEXT / YMD. 외부 변수 포인터에 직접 바인딩.
// 포커스 = 클릭 시 획득. 키 입력은 IDrawContext::PollTextInput / IsKeyPressed.
// IME(한글) 한정 지원 — 호스트가 WM_CHAR/IME COMPOSITION 결과를 PollTextInput 으로 공급.
// 편집 버퍼/TEXT 바인딩은 wstring(UTF-16) — 설계 §7 wstring 단일.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"
#include "DxgTheme.h"

#include <string>
#include <functional>


namespace dxgui
{

	enum E_DXG_EDIT_TYPE : uint8_t
	{
		DXG_EDIT_INT32 = 0,
		DXG_EDIT_INT64 = 1,
		DXG_EDIT_FLOAT = 2,
		DXG_EDIT_TEXT  = 3,
		DXG_EDIT_YMD   = 4,   // int32 YYYYMMDD 바인딩 — 표시는 YYYY-MM-DD 자동 마스킹
	};


	class C_DXG_EDITBOX : public C_DXG_WIDGET
	{
	private:
		E_DXG_EDIT_TYPE m_DataType;
		void*           m_pData;          // 바인딩 변수 포인터 (타입 일치 책임=호출자)
		size_t          m_uTextMax;       // TEXT 타입의 max length (wchar, NUL 제외)
		FontHandle      m_hFont;
		_DXG_COLOR      m_TextColor;
		_DXG_COLOR      m_BgColor;
		_DXG_COLOR      m_BorderColor;
		_DXG_COLOR      m_BorderFocusColor;

		// 편집 상태
		bool         m_bFocused;
		std::wstring m_sBuffer;           // 편집 중 표시 버퍼 (TEXT) 또는 숫자 입력 버퍼
		size_t       m_uCaret;            // 캐럿 위치 (m_sBuffer 인덱스, 선택 끝점)
		size_t       m_uSelAnchor;        // 선택 앵커. ==m_uCaret 이면 선택 없음(캐럿만).
		bool         m_bDragSel;          // 좌버튼 드래그로 범위선택 중
		int          m_nBlinkCnt;         // caret blink 카운터 (frame 단위, 60 = ~1초 주기)
		float        m_fScrollX;          // 가로 스크롤 오프셋(px) — 캐럿 가시화(ES_AUTOHSCROLL 등가)
		E_DXG_TEXT_ALIGN m_eTextAlign;    // 텍스트 가로정렬(기본 좌). 텍스트가 박스보다 짧을 때만 적용.
		std::function<void()> m_OnEnter;  // 엔터 커밋 콜백(없으면 무시)

	public:
		C_DXG_EDITBOX()
			: m_DataType(DXG_EDIT_INT32)
			, m_pData(nullptr)
			, m_uTextMax(255)
			, m_hFont(INVALID_FONT)
			, m_TextColor(Theme().textMain)
			, m_BgColor(Theme().card)
			, m_BorderColor(0xFF8896A8u)
			, m_BorderFocusColor(Theme().accent)
			, m_bFocused(false)
			, m_uCaret(0)
			, m_uSelAnchor(0)
			, m_bDragSel(false)
			, m_nBlinkCnt(0)
			, m_fScrollX(0.0f)
			, m_eTextAlign(DXG_TEXT_ALIGN_LEFT)
		{
		}

		// 바인딩 — 데이터 변수 포인터를 등록. 타입과 일치해야 함.
		void BindInt32(int32_t* _p)        { m_DataType = DXG_EDIT_INT32; m_pData = _p; }
		void BindInt64(int64_t* _p)        { m_DataType = DXG_EDIT_INT64; m_pData = _p; }
		void BindFloat(float* _p)          { m_DataType = DXG_EDIT_FLOAT; m_pData = _p; }
		void BindText(std::wstring* _p, size_t _uMax = 255)
		{ m_DataType = DXG_EDIT_TEXT; m_pData = _p; m_uTextMax = _uMax; }
		// YYYY-MM-DD 자동 마스킹. 내부는 int32 YYYYMMDD (예: 20200102).
		void BindYmd(int32_t* _p)          { m_DataType = DXG_EDIT_YMD; m_pData = _p; }

		void SetFont(FontHandle _h)        { m_hFont = _h; }
		void SetTextColor(_DXG_COLOR _c)   { m_TextColor = _c; }
		void SetBgColor(_DXG_COLOR _c)     { m_BgColor = _c; }
		void SetBorderColor(_DXG_COLOR _c, _DXG_COLOR _focus)
		{ m_BorderColor = _c; m_BorderFocusColor = _focus; }
		// 엔터 커밋 시 콜백(StringToData_ 이후 호출) — 종목적용/조회 등 "확정" 동작 라우팅.
		void SetOnEnter(std::function<void()> _fn) { m_OnEnter = std::move(_fn); }

		E_DXG_EDIT_TYPE GetDataType() const { return m_DataType; }
		bool   IsSelected() const           { return m_uSelAnchor != m_uCaret; }
		size_t GetBufferSize() const        { return m_sBuffer.size(); }

		// 텍스트 가로정렬 — 베이스 virtual 연결. 설정창/영속이 일반적으로 질의(0좌/1중/2우).
		bool SupportsTextAlign() const override { return true; }
		int  GetTextAlign() const override      { return static_cast<int>(m_eTextAlign); }
		void SetTextAlign(int _a) override      { m_eTextAlign = static_cast<E_DXG_TEXT_ALIGN>((_a < 0 || _a > 2) ? 0 : _a); }

		// 타입
		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_EDITBOX; }
		const char*       GetTypeName() const override { return "editbox"; }

		// 포커스 순회(Tab) — 편집 위젯. SetFocused 는 진입=버퍼적재+전체선택 / 이탈=커밋.
		bool AcceptsFocus() const override { return true; }
		bool IsFocused() const override    { return m_bFocused; }
		void SetFocused(bool _b) override;

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;

	private:
		// 바인딩 변수 → 표시 문자열 (편집 시작/포커스 종료 시 동기).
		std::wstring DataToString_() const;
		// 표시 문자열 → 바인딩 변수 (편집 종료 / 엔터).
		void StringToData_();
		// YMD 버퍼 정규화 — 숫자만 추출 후 "YYYY-MM-DD" 형식 재구성.
		void NormalizeYmdBuffer_();
	};

} // namespace dxgui
