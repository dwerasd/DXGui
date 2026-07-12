// DxgListGrid.h: 표 위젯(고정 헤더 + 세로 스크롤 + 셀 단위 색 + 행 선택).
// 투자자정보/호가/체결/잔고 등 HTS 그리드 렌더용. 데이터는 외부 주입(셀 텍스트/색).
// 가로 스크롤은 미지원(P1) - 컬럼 폭 합이 표 폭을 넘으면 우측 클립.
#pragma once

#include "DxgWidget.h"
#include "DxgDrawContext.h"
#include "DxgTheme.h"
#include "DxgBadge.h"		// E_DXG_BADGE_KIND / BadgeColor (배지 셀)
#include "DxgScrollbar.h"

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
		std::function<void(float, float)> m_OnRightClick;	// 본문 우클릭(x,y 화면좌표) - 컨텍스트 메뉴 호출처

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

	public:
		C_DXG_LISTGRID();

		void SetFont(FontHandle _h)        { m_hFont = _h; }
		void SetRowHeight(float _h)        { m_fRowH = _h; }
		void SetHeaderHeight(float _h)     { m_fHeaderH = _h; }
		void SetRowAlt(bool _b)            { m_bRowAlt = _b; }
		void SetGridLines(bool _b)         { m_bGridLines = _b; }
		void SetHeaderInteract(bool _b)    { m_bHeaderInteract = _b; }	// 정렬/리사이즈 on/off
		void SetStretchLastColumn(bool _b) { m_bStretchLast = _b; }		// 마지막 컬럼 남은폭 채움(클리핑/수동조절 방지)
		void SetHeaderCenter(bool _b)      { m_bHeaderCenter = _b; }		// 헤더 제목 가운데정렬
		void SetOnSelect(std::function<void(int)> _fn) { m_OnSelect = std::move(_fn); }
		// 행 더블클릭 콜백 - 행 인덱스 전달. 단일클릭(선택)과 별개로 발화(HTS 식).
		void SetOnDblClick(std::function<void(int)> _fn) { m_OnDblClick = std::move(_fn); }
		// 본문 우클릭 콜백 - (x,y) 화면좌표 전달. 컨텍스트 메뉴는 호출처가 소유.
		void SetOnRightClick(std::function<void(float, float)> _fn) { m_OnRightClick = std::move(_fn); }
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
		void ClearRows() { m_vRows.clear(); m_nSelRow = -1; }
		int  AddRow()
		{
			m_vRows.emplace_back(m_vCols.size());
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
		void   SetSelectedRow(int _r) { m_nSelRow = _r; }

		// 컬럼 너비 편집 API(설정창) - m_vCols 위임. 폭 단위 px, 최소 m_fMinColW 클램프.
		int          ColumnCount() const override { return static_cast<int>(m_vCols.size()); }
		float        ColumnWidth(int _i) const override
		{ return (_i >= 0 && _i < static_cast<int>(m_vCols.size())) ? m_vCols[_i].fWidth : 0.0f; }
		void         SetColumnWidth(int _i, float _w) override
		{ if (_i >= 0 && _i < static_cast<int>(m_vCols.size())) { m_vCols[_i].fWidth = (_w < m_fMinColW) ? m_fMinColW : _w; } }
		std::wstring ColumnName(int _i) const override
		{ return (_i >= 0 && _i < static_cast<int>(m_vCols.size())) ? m_vCols[_i].sTitle : std::wstring(); }

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
	};

} // namespace dxgui
