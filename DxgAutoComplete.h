// DxgAutoComplete.h: 자동완성 입력 위젯 - 에디트박스 + 후보 드롭다운.
// 편집·캐럿·IME·클립보드는 C_DXG_EDITBOX 상속분 그대로. 타이핑할 때마다 Provider 로
// 후보를 받아 에디트 아래(공간 부족 시 위)에 목록을 띄우고 위/아래 키·Enter·마우스로 확정한다.
// 드롭다운 기하: 폭 = max(에디트 폭, 최장 항목 텍스트 + 좌우 여백 + 스크롤바) 를 캔버스 폭으로
// 클램프하고 오른쪽으로 넘치면 왼쪽으로 민다 - 에디트가 좁아도(차트 헤더 100px) 항목이 읽힌다.
// 항목 폭 측정은 후보가 바뀐 프레임에만 1회 수행한다(매 프레임 측정 금지).
// 드롭다운 렌더는 오버레이 패스(최상위). 모달은 **마우스가 드롭다운 사각형 위에 있는 프레임**
// 으로 한정한다 - 그 프레임만 매니저가 pass1 입력을 캡처해 드롭다운에 가린 위젯의 클릭 누수를
// 막고, 창의 나머지(조회 버튼·다른 위젯·Tab 순회·에디트 캐럿/드래그)는 목록이 열려 있어도
// 정상 동작한다. 콤보는 클릭으로 열려 창 전체 캡처가 자연스럽지만 자동완성은 타이핑만으로
// 열리므로 같은 대가를 물리면 입력 직후 창 전체가 먹통이 된다. 키보드·IME 는 영향 없음.
// 루트 위젯으로만 등록할 것(모달 판정이 루트만 본다).
#pragma once

#include "DxgEditbox.h"
#include "DxgDrawContext.h"
#include "DxgTheme.h"

#include <vector>
#include <string>
#include <functional>


namespace dxgui
{

	// 후보 1건 - sText = 목록 표시 문자열, sKey = 확정 시 에디트에 들어가는 값.
	struct _DXG_AC_ITEM
	{
		std::wstring sText;
		std::wstring sKey;
	};


	class C_DXG_AUTOCOMPLETE : public C_DXG_EDITBOX
	{
	private:
		std::vector<_DXG_AC_ITEM> m_vItems;
		std::wstring m_sLastQuery;		// 마지막으로 Provider 에 넘긴 질의(버퍼+조합중)
		int   m_nSel;					// 드롭다운 선택 인덱스(-1 = 없음)
		bool  m_bOpen;
		bool  m_bJustOpened;			// 연 프레임 표식 - 오버레이의 외부클릭 닫기를 1프레임 무시
		bool  m_bHoverDropLast;			// 직전 프레임 오버레이에서 마우스가 드롭다운 위였나(모달 범위 판정)
		bool  m_bScrollToSel;			// 위/아래 키 이동 후 선택항목 가시화 요청(높이는 오버레이가 안다)
		bool  m_bItemsWDirty;			// 후보가 바뀌어 최장 항목 폭 재측정이 필요한가(측정은 오버레이가 1회)
		int   m_nMaxVisible;
		float m_fItemH;
		float m_fCellPad;
		float m_fScrollY;
		float m_fItemsMaxW;				// 최장 항목 텍스트 폭(px) - 드롭다운 폭 산출용 캐시

		_DXG_COLOR m_DropBgColor;
		_DXG_COLOR m_DropBorderColor;
		_DXG_COLOR m_ItemTextColor;
		_DXG_COLOR m_ItemHoverBg;
		_DXG_COLOR m_ItemSelBg;
		_DXG_COLOR m_ScrollColor;

		std::function<void(const std::wstring&, std::vector<_DXG_AC_ITEM>*)> m_Provider;
		std::function<void(const _DXG_AC_ITEM&)> m_OnSelect;
		std::function<void()> m_OnEnterUser;	// 베이스 SetOnEnter 를 은닉해 보관하는 원문 콜백

	public:
		C_DXG_AUTOCOMPLETE();

		// 후보 공급 - 동기 호출(UI 스레드). 후보 개수 상한은 공급자가 자른다.
		void SetProvider(std::function<void(const std::wstring& _sQuery, std::vector<_DXG_AC_ITEM>* _pOut)> _fn)
		{ m_Provider = std::move(_fn); }
		// 후보 확정 콜백(버퍼 = sKey 커밋 + 드롭다운 닫힘 이후 호출).
		void SetOnSelect(std::function<void(const _DXG_AC_ITEM&)> _fn) { m_OnSelect = std::move(_fn); }
		// 베이스의 비가상 SetOnEnter 은닉 - 드롭다운에서 고른 항목이 있으면 그쪽이 우선하고,
		// 없을 때만 여기 등록된 콜백(원문 처리)이 호출된다. 반드시 파생 타입으로 설정할 것.
		void SetOnEnter(std::function<void()> _fn) { m_OnEnterUser = std::move(_fn); }

		void SetMaxVisible(int _n)   { m_nMaxVisible = (_n > 0) ? _n : 1; }
		void SetItemHeight(float _h) { m_fItemH = (_h > 1.0f) ? _h : 1.0f; }

		bool IsOpen() const { return m_bOpen; }
		void Close()        { this->close_(); }

		// 타입은 EDITBOX 유지(직렬화/영속 소비자 호환). 표시명만 구분.
		const char* GetTypeName() const override { return "autocomplete"; }

		// 드롭다운 위에 마우스가 있는 동안만 모달 - 매니저가 그 프레임의 pass1 입력을 캡처해
		// 드롭다운에 가린 위젯으로의 클릭 누수를 막는다. 판정값은 직전 프레임 오버레이가 기록한다.
		bool IsModalActive() const override { return m_bOpen && m_bHoverDropLast; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;
		void RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin) override;

	private:
		void close_();
		void refresh_(const std::wstring& _sQuery);
		void confirm_(int _nIdx);
		void onBaseEnter_();
		void drawTextClip_(IDrawContext& _ctx, const std::wstring& _s,
			float _x, float _w, float _top, float _h, _DXG_COLOR _color);
	};

} // namespace dxgui
