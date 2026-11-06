// DxgListGrid.h: 표 위젯(고정 헤더 + 세로 스크롤 + 셀 단위 색 + 행 선택).
// 투자자정보/호가/체결/잔고 등 HTS 그리드 렌더용. 데이터는 외부 주입(셀 텍스트/색).
// 가로 스크롤은 미지원(P1) - 컬럼 폭 합이 표 폭을 넘으면 우측 클립.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"
#include "DxgTheme.h"
#include "DxgBadge.h"		// E_DXG_BADGE_KIND / BadgeColor (배지 셀)
#include "DxgScrollbar.h"

#include <algorithm>
#include <cstdint>
#include <vector>
#include <string>
#include <functional>


namespace dxgui
{

	// 컬럼 정의 - 제목/폭(px)/가로정렬.
	struct _DXG_GRID_COL
	{
		std::wstring     sTitle;
		float            fWidth;
		E_DXG_TEXT_ALIGN align;

		_DXG_GRID_COL() : fWidth(80.0f), align(DXG_TEXT_ALIGN_LEFT) {}
		_DXG_GRID_COL(std::wstring _t, float _w, E_DXG_TEXT_ALIGN _a)
			: sTitle(std::move(_t)), fWidth(_w), align(_a) {}
	};

	// 셀 - 텍스트 + 글자색 + 배경색(alpha 0 = 행 기본 배경 사용, HTS +/- 색은 여기 지정).
	// nBadge: 255 = 일반 텍스트 셀(기본, 기존 경로 불변), 그 외 = E_DXG_BADGE_KIND -> pill 렌더.
	struct _DXG_GRID_CELL
	{
		std::wstring sText;
		_DXG_COLOR   textColor;
		_DXG_COLOR   bgColor;
		uint8_t      nBadge;

		_DXG_GRID_CELL() : textColor(Theme().textMain), bgColor(0u), nBadge(255u) {}
	};

	// 방향키 연속 이동(OS 키 반복)의 정착 판정 - 시각(ms)을 호출자가 넣는 순수 상태.
	// 이동마다 보류로 기록하고, 방향키를 놓았거나 마지막 이동 뒤 _nSettleMs 동안 이동이 없으면 한 번만 발화 시점이 된다.
	struct _DXG_KEY_SETTLE
	{
		bool     bPending{ false };
		uint64_t nLastMoveMs{ 0 };

		void OnMove(uint64_t _nNowMs) { bPending = true; nLastMoveMs = _nNowMs; }
		// true = 지금 보류분을 발화(보류를 비운다). _bArrowHeld = 위/아래 키가 아직 눌려 있는가.
		bool Poll(uint64_t _nNowMs, uint32_t _nSettleMs, bool _bArrowHeld)
		{
			if (!bPending) { return false; }
			if (_bArrowHeld && (_nNowMs - nLastMoveMs) < _nSettleMs) { return false; }
			bPending = false;
			return true;
		}
		void Cancel() { bPending = false; }
	};

	// SetKeySettleMs 에 넘기는 권장 정착 대기 - 키 반복 간격(약 30ms)보다 충분히 길고 한 번 누름의 체감 지연으로는 짧은 값.
	inline constexpr uint32_t DXG_KEY_SETTLE_MS = 150;


	class C_DXG_LISTGRID : public C_DXG_WIDGET
	{
	private:
		std::vector<_DXG_GRID_COL>                m_vCols;
		std::vector<std::vector<_DXG_GRID_CELL>>  m_vRows;
		FontHandle m_hFont;
		float m_fRowH;
		float m_fHeaderH;
		int   m_nSelRow;
		bool  m_bRowAlt;		// 교대 행배경
		bool  m_bGridLines;		// 행/열 구분선
		float m_fScrollbarW;
		float m_fCellPad;

		_DXG_COLOR m_HeaderBg, m_HeaderText, m_GridLine, m_RowBg, m_RowAltBg, m_SelBg, m_SelText;

		C_DXG_SCROLLBAR m_ScrollV;	// 임베드 세로 스크롤바(필요 시만 표시)
		C_DXG_SCROLLBAR m_ScrollH;	// 임베드 가로 스크롤바(컬럼폭 합 > 본문폭 일 때)
		std::function<void(int)> m_OnSelect;
		std::function<void(int)> m_OnDblClick;	// 본문 행 더블클릭(행 인덱스) - 차트반영/관심추가 등 호출처 동작
		std::function<void(int)> m_OnClick;		// 본문 행 좌클릭 또는 방향키 이동(행 인덱스) - 이미 선택된 행도 매번 발화(OnSelect 는 선택이 바뀔 때만)
		std::function<void(float, float)> m_OnRightClick;	// 본문 우클릭(x,y 화면좌표) - 컨텍스트 메뉴 호출처
		std::function<void(int)> m_OnSpace;		// 키보드 포커스 중 Space(선택 행 인덱스) - 체크 칸 토글 등 호출처 동작
		bool  m_bKeyFocus{ false };				// 키보드 포커스 - 격자 안 좌클릭으로 얻고 격자 밖 좌클릭/Tab 으로 잃는다. Tab 순회 목록에는 들어가지 않는다.
												// 컨텍스트(호스트 창)당 한 격자만 가진다 - 숨겨졌거나 그려지지 않은 격자는 EndKeyFrame 에서 잃는다
		bool  m_bSpaceHeld{ false };			// 직전 그린 프레임에 Space 가 눌려 있었는가 - 키 반복으로 OnSpace 가 연발되지 않게 첫 누름만 통과
		uint32_t m_nKeySettleMs{ 0 };			// 방향키 연속 이동의 정착 대기(ms). 0 = 이동마다 즉시 발화(기본)
		_DXG_KEY_SETTLE m_Settle;				// 정착 대기 중인 방향키 이동(m_nKeySettleMs > 0 일 때만 쓴다)
		int   m_nPendBase{ -1 };				// 보류 시작 때 호출처가 아는 선택 행 - 정착 뒤 OnSelect 가 바뀌었는지 가르는 기준

		// 헤더 상호작용 - 정렬(클릭) / 컬럼 리사이즈(경계 드래그).
		int   m_nSortCol{ -1 };		// 정렬 컬럼(-1=없음)
		bool  m_bSortAsc{ true };
		int   m_nResizeCol{ -1 };	// 리사이즈 중 컬럼
		bool  m_bResizing{ false };
		float m_fResizeStartMx{ 0.0f };
		float m_fResizeStartW{ 0.0f };
		float m_fMinColW{ 36.0f };
		bool  m_bHeaderInteract{ true };	// 정렬/리사이즈 활성
		bool  m_bStretchLast{ false };		// 마지막 컬럼이 남은 본문폭을 채움(윈도우 ListView 식). 클리핑/수동조절 방지.
		bool  m_bHeaderCenter{ false };		// 헤더 제목 가운데정렬(데이터 정렬과 무관)
		// 다중 선택(기본 끔): 보통 클릭 = 그 행만 표시 + 선택, Ctrl+클릭 = 표시 토글, Shift+클릭 = 기준 행부터 범위 표시.
		// Ctrl/Shift 클릭은 선택(m_nSelRow)과 OnSelect/OnClick 을 건드리지 않는다.
		bool  m_bMulti{ false };
		std::vector<uint8_t> m_vMark;		// 행별 표시(크기 = 행 수)
		int   m_nAnchor{ -1 };				// Shift 범위 기준 행

	public:
		C_DXG_LISTGRID();
		~C_DXG_LISTGRID() override;

		// 호스트 매니저가 프레임 끝에 부른다 - 이 컨텍스트의 키보드 포커스 격자가 이번 프레임에 그려지지 않았으면(숨김 등) 포커스를 푼다.
		static void EndKeyFrame(const IDrawContext& _ctx);

		void SetFont(FontHandle _h)        { m_hFont = _h; }
		void SetRowHeight(float _h)        { m_fRowH = _h; }
		void SetHeaderHeight(float _h)     { m_fHeaderH = _h; }
		void SetRowAlt(bool _b)            { m_bRowAlt = _b; }
		void SetGridLines(bool _b)         { m_bGridLines = _b; }
		void SetHeaderInteract(bool _b)    { m_bHeaderInteract = _b; }	// 정렬/리사이즈 on/off
		void SetStretchLastColumn(bool _b) { m_bStretchLast = _b; }		// 마지막 컬럼 남은폭 채움(클리핑/수동조절 방지)
		void SetHeaderCenter(bool _b)      { m_bHeaderCenter = _b; }		// 헤더 제목 가운데정렬
		void SetMultiSelect(bool _b)       { m_bMulti = _b; }
		bool IsRowMarked(int _r) const
		{ return _r >= 0 && _r < static_cast<int>(m_vMark.size()) && m_vMark[_r] != 0; }
		std::vector<int> MarkedRows() const
		{
			std::vector<int> v;
			for (int r = 0; r < static_cast<int>(m_vMark.size()); ++r) { if (m_vMark[r] != 0) { v.push_back(r); } }
			return v;
		}
		void SetOnSelect(std::function<void(int)> _fn) { m_OnSelect = std::move(_fn); }
		// 행 좌클릭 콜백 - 유효 행 클릭마다 발화(이미 선택된 행 재클릭 포함). 선택 갱신·OnSelect 뒤에 호출되며 OnSelect 를 대체하지 않는다.
		// 포커스 중 위/아래 방향키로 행을 옮길 때도 발화한다(마우스 이벤트 없는 프레임 - 콜백 안에서 마우스 상태를 가정하지 말 것).
		void SetOnClick(std::function<void(int)> _fn) { m_OnClick = std::move(_fn); }
		// 행 더블클릭 콜백 - 행 인덱스 전달. 단일클릭(선택)과 별개로 발화(HTS 식).
		void SetOnDblClick(std::function<void(int)> _fn) { m_OnDblClick = std::move(_fn); }
		// 본문 우클릭 콜백 - (x,y) 화면좌표 전달. 컨텍스트 메뉴는 호출처가 소유.
		void SetOnRightClick(std::function<void(float, float)> _fn) { m_OnRightClick = std::move(_fn); }
		// Space 콜백 - 키보드 포커스 중 선택 행이 있을 때 Space 를 누를 때마다 한 번 발화(행 인덱스 전달, 키 반복은 무시). 미지정이면 Space 는 아무 일도 하지 않는다.
		// 위/아래 방향키는 콜백 없이 선택을 한 칸 옮기며 좌클릭과 같은 OnSelect/OnClick 을 발화한다(더블클릭은 발화하지 않는다).
		// 다중 선택 격자는 표시가 선택 행 하나뿐일 때만 표시가 따라가고, Ctrl/Shift 로 만든 표시 집합은 지우지 않고 선택 행(테두리로 표시)만 옮긴다.
		void SetOnSpace(std::function<void(int)> _fn) { m_OnSpace = std::move(_fn); }
		// 방향키 연속 이동(OS 키 반복)의 정착 대기(ms). 0(기본) = 이동마다 OnSelect/OnClick 즉시. 0 초과 = 선택 행은 바로 옮기되
		// OnSelect/OnClick 은 방향키를 놓았거나 마지막 이동 뒤 이 시간 동안 이동이 없을 때 최종 행으로 한 번만 발화한다(행마다 무거운 일을 하는 호출처용).
		void SetKeySettleMs(uint32_t _nMs) { m_nKeySettleMs = _nMs; if (_nMs == 0) { m_Settle.Cancel(); } }
		bool HasKeyFocus() const { return m_bKeyFocus; }
		void SetColors(_DXG_COLOR _hdrBg, _DXG_COLOR _hdrText, _DXG_COLOR _line,
			_DXG_COLOR _rowBg, _DXG_COLOR _rowAltBg, _DXG_COLOR _selBg, _DXG_COLOR _selText)
		{
			m_HeaderBg = _hdrBg; m_HeaderText = _hdrText; m_GridLine = _line;
			m_RowBg = _rowBg; m_RowAltBg = _rowAltBg; m_SelBg = _selBg; m_SelText = _selText;
		}

		// 컬럼 구성.
		void AddColumn(const std::wstring& _sTitle, float _fWidth, E_DXG_TEXT_ALIGN _a = DXG_TEXT_ALIGN_LEFT)
		{ m_vCols.emplace_back(_sTitle, _fWidth, _a); }
		void ClearColumns() { m_vCols.clear(); }

		// 행 구성 - AddRow 후 SetCell. 셀은 컬럼 수만큼 자동 확보.
		void ClearRows() { m_vRows.clear(); m_nSelRow = -1; m_vMark.clear(); m_nAnchor = -1; m_Settle.Cancel(); }
		int  AddRow()
		{
			m_vRows.emplace_back(m_vCols.size());
			m_vMark.push_back(0);
			return static_cast<int>(m_vRows.size()) - 1;
		}
		void SetCell(int _nRow, int _nCol, const std::wstring& _sText, _DXG_COLOR _textColor)
		{
			if (_nRow < 0 || _nRow >= static_cast<int>(m_vRows.size())) { return; }
			if (_nCol < 0 || _nCol >= static_cast<int>(m_vRows[_nRow].size())) { return; }
			m_vRows[_nRow][_nCol].sText = _sText;
			m_vRows[_nRow][_nCol].textColor = _textColor;
		}
		void SetCellBg(int _nRow, int _nCol, _DXG_COLOR _bg)
		{
			if (_nRow < 0 || _nRow >= static_cast<int>(m_vRows.size())) { return; }
			if (_nCol < 0 || _nCol >= static_cast<int>(m_vRows[_nRow].size())) { return; }
			m_vRows[_nRow][_nCol].bgColor = _bg;
		}
		// 배지 셀 - 셀을 pill(틴트 배경 + 본색 텍스트)로 렌더. 정렬은 컬럼 정렬을 따른다.
		void SetCellBadge(int _nRow, int _nCol, const std::wstring& _sText, E_DXG_BADGE_KIND _kind)
		{
			if (_nRow < 0 || _nRow >= static_cast<int>(m_vRows.size())) { return; }
			if (_nCol < 0 || _nCol >= static_cast<int>(m_vRows[_nRow].size())) { return; }
			m_vRows[_nRow][_nCol].sText = _sText;
			m_vRows[_nRow][_nCol].nBadge = static_cast<uint8_t>(_kind);
		}

		size_t RowCount() const { return m_vRows.size(); }
		size_t ColCount() const { return m_vCols.size(); }
		int    SelectedRow() const { return m_nSelRow; }
		void   SetSelectedRow(int _r)
		{
			m_nSelRow = _r;
			m_Settle.Cancel();	// 호출처가 직접 정한 선택 - 보류 중이던 방향키 이동 알림은 버린다
			if (m_bMulti)	// 다중 선택이면 표시도 그 행 하나로
			{
				std::fill(m_vMark.begin(), m_vMark.end(), static_cast<uint8_t>(0));
				if (_r >= 0 && _r < static_cast<int>(m_vMark.size())) { m_vMark[_r] = 1; }
				m_nAnchor = _r;
			}
		}
		// 행을 본문 안으로 즉시 스크롤한다. 행을 막 다시 채운 직후에도 맞도록 현재 행 수로 세로 범위를 먼저 다시 맞춘다
		// (가로 스크롤바가 없는 격자 전제 - SetStretchLastColumn(true)).
		void   ScrollToRow(int _nRow)
		{
			if (_nRow < 0 || _nRow >= static_cast<int>(m_vRows.size()) || m_fRowH <= 0.0f) { return; }
			m_ScrollV.SetMetrics(static_cast<float>(m_vRows.size()) * m_fRowH, m_ScrollV.ViewportH());
			const float fTop_ = static_cast<float>(_nRow) * m_fRowH;
			if (m_ScrollV.ViewportH() <= 0.0f) { m_ScrollV.SetValue(fTop_); return; }	// 첫 렌더 전(보이는 높이 미정) - 행을 맨 위에 둔다
			const float fCur_ = m_ScrollV.GetValue();
			const float fVis_ = m_ScrollV.ViewportH();
			if (fTop_ < fCur_) { m_ScrollV.SetValue(fTop_); }
			else if (fTop_ + m_fRowH > fCur_ + fVis_) { m_ScrollV.SetValue(fTop_ + m_fRowH - fVis_); }
		}

		// 컬럼 너비 편집 API(설정창) - m_vCols 위임. 폭 단위 px, 최소 m_fMinColW 클램프.
		int          ColumnCount() const override { return static_cast<int>(m_vCols.size()); }
		float        ColumnWidth(int _i) const override
		{ return (_i >= 0 && _i < static_cast<int>(m_vCols.size())) ? m_vCols[_i].fWidth : 0.0f; }
		void         SetColumnWidth(int _i, float _w) override
		{ if (_i >= 0 && _i < static_cast<int>(m_vCols.size())) { m_vCols[_i].fWidth = (_w < m_fMinColW) ? m_fMinColW : _w; } }
		std::wstring ColumnName(int _i) const override
		{ return (_i >= 0 && _i < static_cast<int>(m_vCols.size())) ? m_vCols[_i].sTitle : std::wstring(); }
		void         SetColumnTitle(int _i, const std::wstring& _sTitle)	// 머리글만 바꾼다(폭·정렬 유지 - 단위 전환 등)
		{ if (_i >= 0 && _i < static_cast<int>(m_vCols.size())) { m_vCols[_i].sTitle = _sTitle; } }

		E_DXG_WIDGET_TYPE GetType() const override     { return DXG_WIDGET_LISTGRID; }
		const char*       GetTypeName() const override { return "listgrid"; }

		void Render(IDrawContext& _ctx, _DXG_POINT _origin) override;

	private:
		float totalColW_() const
		{
			float w = 0.0f;
			for (const _DXG_GRID_COL& c : m_vCols) { w += c.fWidth; }
			return w;
		}
		// 셀/헤더 텍스트를 영역(_colX..+_colW, _top..+_h)에 정렬 클립 렌더.
		void drawCellText_(IDrawContext& _ctx, const std::wstring& _s, _DXG_COLOR _color,
			float _colX, float _colW, float _top, float _h, E_DXG_TEXT_ALIGN _align);
		float effColW_(int _c, float _fBodyW) const;	// 컬럼 유효폭(stretch 모드면 마지막=남은폭)
		void sortRows_();	// m_nSortCol/m_bSortAsc 기준 행 정렬(숫자/문자 자동)
		void pickRow_(int _r);	// 보통 좌클릭 행 선택(표시 갱신 + OnSelect(선택이 바뀔 때만) + OnClick)
		void keyMove_(int _nTo);	// 위/아래 방향키 행 이동 - 선택 행을 옮기고 pickRow_ 와 같은 콜백을 즉시(m_nKeySettleMs == 0) 또는 정착 뒤(flushKeyPick_) 발화
		void flushKeyPick_();	// 정착한 방향키 이동의 OnSelect(호출처가 아는 선택에서 바뀐 때만) + OnClick
		void setKeyFocus_(IDrawContext& _ctx, bool _bFocus);	// 키보드 포커스 설정/해제 - 얻으면 이 컨텍스트의 소유자가 된다
	};

} // namespace dxgui
