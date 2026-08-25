// DxgAutoComplete.cpp
#include "DxgAutoComplete.h"
#include "DxgKeys.h"


namespace dxgui
{

	C_DXG_AUTOCOMPLETE::C_DXG_AUTOCOMPLETE()
		: m_nSel(-1)
		, m_bOpen(false)
		, m_bJustOpened(false)
		, m_bHoverDropLast(false)
		, m_bScrollToSel(false)
		, m_bItemsWDirty(false)
		, m_nMaxVisible(8)
		, m_fItemH(24.0f)
		, m_fCellPad(6.0f)
		, m_fScrollY(0.0f)
		, m_fItemsMaxW(0.0f)
		, m_DropBgColor(Theme().card)
		, m_DropBorderColor(Theme().accent)
		, m_ItemTextColor(Theme().textMain)
		, m_ItemHoverBg(0xFFEAF1FBu)
		, m_ItemSelBg(0xFFD6E4FFu)
		, m_ScrollColor(0xFF5A6678u)
	{
		// 베이스의 Enter 커밋 콜백을 가로챈다 - 드롭다운 선택이 있으면 그것을 확정하고,
		// 없을 때만 사용자 콜백으로 넘긴다. this 는 위젯 수명 내 고정(복사·이동 금지 베이스).
		C_DXG_EDITBOX::SetOnEnter([this]() { this->onBaseEnter_(); });
	}


	void C_DXG_AUTOCOMPLETE::close_()
	{
		m_bOpen = false;
		m_bJustOpened = false;
		m_bHoverDropLast = false;
		m_bScrollToSel = false;
		m_nSel = -1;
		m_fScrollY = 0.0f;
		m_vItems.clear();
		m_fItemsMaxW = 0.0f;
		m_bItemsWDirty = false;
	}


	// 질의가 바뀐 프레임에만 호출 - 후보를 새로 받아 열림/닫힘을 갱신.
	void C_DXG_AUTOCOMPLETE::refresh_(const std::wstring& _sQuery)
	{
		m_sLastQuery = _sQuery;
		m_vItems.clear();
		m_nSel = -1;
		m_fScrollY = 0.0f;
		m_bScrollToSel = false;
		if (_sQuery.empty() || !m_Provider)
		{
			m_bOpen = false;
			m_bJustOpened = false;
			return;
		}
		m_Provider(_sQuery, &m_vItems);
		if (m_vItems.empty())
		{
			m_bOpen = false;
			m_bJustOpened = false;
			return;
		}
		m_bOpen = true;
		m_bJustOpened = true;
		m_bItemsWDirty = true;	// 실제 측정은 컨텍스트가 있는 오버레이에서 1회
	}


	void C_DXG_AUTOCOMPLETE::confirm_(int _nIdx)
	{
		if (_nIdx < 0 || _nIdx >= static_cast<int>(m_vItems.size())) { return; }
		const _DXG_AC_ITEM item_ = m_vItems[_nIdx];	// close_ 가 목록을 비우므로 값 복사
		this->SetBufferAndCommit_(item_.sKey);
		m_sLastQuery = item_.sKey;					// 확정값으로 재질의하지 않도록 동기
		this->close_();
		if (m_OnSelect) { m_OnSelect(item_); }
	}


	// 베이스가 Enter 를 소화한 직후(커밋 + 포커스 해제 완료) 호출된다.
	void C_DXG_AUTOCOMPLETE::onBaseEnter_()
	{
		if (m_bOpen && m_nSel >= 0 && m_nSel < static_cast<int>(m_vItems.size()))
		{
			this->confirm_(m_nSel);
			return;
		}
		this->close_();
		if (m_OnEnterUser) { m_OnEnterUser(); }
	}


	void C_DXG_AUTOCOMPLETE::drawTextClip_(IDrawContext& _ctx, const std::wstring& _s,
		float _x, float _w, float _top, float _h, _DXG_COLOR _color)
	{
		const FontHandle hFont_ = this->Font_();
		if (_s.empty() || hFont_ == INVALID_FONT || _w <= 0.0f) { return; }
		_ctx.PushClipRect(_DXG_RECT(_x, _top, _w, _h));
		const _DXG_SIZE sz_ = _ctx.MeasureText(hFont_, _s.c_str(), m_fFontScale);
		const float fTy_ = _top + (_h - sz_.h) * 0.5f;
		_ctx.DrawText(hFont_, _DXG_POINT(_x + m_fCellPad, fTy_), _s.c_str(), _color, m_fFontScale);
		_ctx.PopClipRect();
	}


	void C_DXG_AUTOCOMPLETE::Render(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		// 연 프레임 표식은 여기서 소비한다 - 오버레이(같은 프레임 pass2)에서 지우면 연 프레임
		// 안에서 즉시 소진돼 가드가 성립하지 않는다. refresh_ 는 이 아래 Render 말미에서 세운다.
		m_bJustOpened = false;

		// 위/아래/Esc 는 베이스보다 먼저 처리한다 - 베이스 Render 가 같은 프레임에 Enter 를
		// 소화하므로(커밋 후 콜백) 선택 인덱스가 그 전에 확정돼 있어야 한다.
		if (m_bVisible && m_bEnabled && m_bOpen && this->Focused_())
		{
			const int nCnt_ = static_cast<int>(m_vItems.size());
			if (nCnt_ > 0)
			{
				if (_ctx.IsKeyPressed(DXG_VK_DOWN))
				{
					m_nSel = (m_nSel + 1 < nCnt_) ? (m_nSel + 1) : (nCnt_ - 1);
					m_bScrollToSel = true;
				}
				if (_ctx.IsKeyPressed(DXG_VK_UP))
				{
					m_nSel = (m_nSel > 0) ? (m_nSel - 1) : 0;
					m_bScrollToSel = true;
				}
			}
			if (_ctx.IsKeyPressed(DXG_VK_ESCAPE)) { this->close_(); }
		}

		C_DXG_EDITBOX::Render(_ctx, _origin);

		if (!m_bVisible) { return; }

		// 질의 = 편집 버퍼에 IME 조합중 문자열을 캐럿 위치로 끼워 넣은 것.
		// 조합중 문자열은 호스트가 포커스 시에만 공급하므로 비포커스 프레임은 자동으로 비어 있다.
		if (m_bEnabled && this->Focused_())
		{
			std::wstring sQuery_ = this->Buffer_();
			const wchar_t* pComp_ = _ctx.PollComposition();
			if (pComp_ != nullptr && pComp_[0] != L'\0')
			{
				size_t uPos_ = this->Caret_();
				if (uPos_ > sQuery_.size()) { uPos_ = sQuery_.size(); }
				sQuery_.insert(uPos_, pComp_);
			}
			if (sQuery_ != m_sLastQuery) { this->refresh_(sQuery_); }
		}
		else if (m_bOpen)
		{
			this->close_();		// 포커스 상실 = 닫기
		}
	}


	void C_DXG_AUTOCOMPLETE::RenderOverlay(IDrawContext& _ctx, _DXG_POINT _origin)
	{
		// 그리지 않는 프레임은 모달도 아니다(호버 표식이 낡은 채 남으면 pass1 이 계속 캡처된다).
		if (!m_bVisible || !m_bOpen || m_vItems.empty()) { m_bHoverDropLast = false; return; }

		const _DXG_RECT abs_ = this->AbsRect(_origin);
		const int   n_      = static_cast<int>(m_vItems.size());
		const float fFullH_ = m_fItemH * static_cast<float>(n_);

		// 펼침 방향/높이 - 캔버스(창) 안에 들어가도록 클램프. 아래 공간이 너무 작으면 위로 펼침.
		// g_fOverlayCanvasH<=0(미주입) 이면 기존 동작(전체 높이 아래로).
		const float fCanvasH_ = g_fOverlayCanvasH;
		const float fMargin_  = 2.0f;
		float fRoomBelow_ = (fCanvasH_ > 0.0f) ? (fCanvasH_ - (abs_.y + abs_.h) - fMargin_) : fFullH_;
		float fRoomAbove_ = (fCanvasH_ > 0.0f) ? (abs_.y - fMargin_) : fFullH_;
		if (fRoomBelow_ < 0.0f) { fRoomBelow_ = 0.0f; }
		if (fRoomAbove_ < 0.0f) { fRoomAbove_ = 0.0f; }
		const float fMinH_  = m_fItemH * 4.0f;	// 아래가 이보다 좁고 위가 더 넓으면 위로 전환
		const bool  bUp_    = (fCanvasH_ > 0.0f) && (fRoomBelow_ < fFullH_) && (fRoomBelow_ < fMinH_) && (fRoomAbove_ > fRoomBelow_);
		const float fAvail_ = bUp_ ? fRoomAbove_ : fRoomBelow_;
		int nVis_ = (fAvail_ >= fFullH_) ? n_ : static_cast<int>(fAvail_ / m_fItemH);
		if (nVis_ < 1) { nVis_ = 1; }
		if (nVis_ > n_) { nVis_ = n_; }
		if (nVis_ > m_nMaxVisible) { nVis_ = m_nMaxVisible; }
		const float fDropH_ = m_fItemH * static_cast<float>(nVis_);
		const float fDropY_ = bUp_ ? (abs_.y - fDropH_) : (abs_.y + abs_.h);

		// 스크롤 필요 여부를 폭보다 먼저 정한다 - 스크롤바 폭이 드롭다운 폭에 포함된다.
		const float fMaxScroll_ = fFullH_ - fDropH_;
		const bool  bScroll_    = fMaxScroll_ > 0.5f;
		const float fBarW_      = bScroll_ ? 6.0f : 0.0f;	// 스크롤바 폭(있을 때만)

		// 최장 항목 폭 - 후보가 바뀐 프레임에만 1회 측정한다(폰트 미등록이면 다음 프레임으로 미룬다).
		if (m_bItemsWDirty && this->Font_() != INVALID_FONT)
		{
			m_fItemsMaxW = 0.0f;
			for (const _DXG_AC_ITEM& item_ : m_vItems)
			{
				if (item_.sText.empty()) { continue; }
				const float fW_ = _ctx.MeasureText(this->Font_(), item_.sText.c_str(), m_fFontScale).w;
				if (fW_ > m_fItemsMaxW) { m_fItemsMaxW = fW_; }
			}
			m_bItemsWDirty = false;
		}

		// 폭 = max(에디트 폭, 최장 항목 + 좌우 여백 + 스크롤바). 캔버스 폭(주입 시)으로 클램프하고
		// 오른쪽으로 넘치면 왼쪽으로 민다. g_fOverlayCanvasW<=0(미주입) 이면 가로 클램프 없음.
		float fDropW_ = m_fItemsMaxW + m_fCellPad * 2.0f + fBarW_;
		if (fDropW_ < abs_.w) { fDropW_ = abs_.w; }
		float fDropX_ = abs_.x;
		const float fCanvasW_ = g_fOverlayCanvasW;
		if (fCanvasW_ > 0.0f)
		{
			if (fDropW_ > fCanvasW_) { fDropW_ = fCanvasW_; }
			if (abs_.x + fDropW_ > fCanvasW_)
			{
				fDropX_ = fCanvasW_ - fDropW_;
				if (fDropX_ < 0.0f) { fDropX_ = 0.0f; }
			}
		}
		const _DXG_RECT dd_(fDropX_, fDropY_, fDropW_, fDropH_);
		const bool bHoverDrop_ = _ctx.IsMouseHovered(dd_);	// 모달 범위 판정 + 밖-클릭 판정 공용

		// 스크롤(휠 + 키 선택 추종). 노치당 3행.
		if (bScroll_)
		{
			const float fWheel_ = _ctx.GetWheelDelta();
			if (fWheel_ != 0.0f) { m_fScrollY -= fWheel_ * m_fItemH * 3.0f; }
		}
		if (m_bScrollToSel && m_nSel >= 0)
		{
			const float fSelTop_ = m_fItemH * static_cast<float>(m_nSel);
			if (fSelTop_ < m_fScrollY) { m_fScrollY = fSelTop_; }
			else if (fSelTop_ + m_fItemH > m_fScrollY + fDropH_) { m_fScrollY = fSelTop_ + m_fItemH - fDropH_; }
			m_bScrollToSel = false;
		}
		if (m_fScrollY < 0.0f) { m_fScrollY = 0.0f; }
		if (m_fScrollY > fMaxScroll_) { m_fScrollY = (fMaxScroll_ > 0.0f) ? fMaxScroll_ : 0.0f; }

		_ctx.FillRect(dd_, m_DropBgColor);
		_ctx.DrawRectOutline(dd_, m_DropBorderColor, 1.0f);

		const float fItemW_ = dd_.w - fBarW_;

		int nClick_ = -1;
		_ctx.PushClipRect(dd_);	// 항목을 드롭다운 박스로 클립(스크롤 넘침/캔버스 밖 미표시)
		for (int i = 0; i < n_; ++i)
		{
			const float fItemY_ = dd_.y + static_cast<float>(i) * m_fItemH - m_fScrollY;
			if (fItemY_ + m_fItemH <= dd_.y || fItemY_ >= dd_.y + dd_.h) { continue; }	// 박스 밖 = skip
			const _DXG_RECT itemR_(dd_.x, fItemY_, fItemW_, m_fItemH);
			const bool bHov_ = _ctx.IsMouseHovered(itemR_);
			if (i == m_nSel) { _ctx.FillRect(itemR_, m_ItemSelBg); }
			else if (bHov_)  { _ctx.FillRect(itemR_, m_ItemHoverBg); }
			this->drawTextClip_(_ctx, m_vItems[i].sText, itemR_.x, itemR_.w, itemR_.y, itemR_.h, m_ItemTextColor);

			if (m_bEnabled && bHov_ && _ctx.IsMouseReleased(DXG_MOUSE_LEFT))	// UP 구동(아래 위젯 클릭관통 차단)
			{
				nClick_ = i;
				break;	// 확정은 클립 해제 후 - confirm_ 이 목록을 비운다
			}
		}
		_ctx.PopClipRect();

		// 스크롤바(트랙+thumb) - 우측 가장자리. 휠 구동(드래그 미지원).
		if (bScroll_ && nClick_ < 0)
		{
			const float fTrackX_ = dd_.x + dd_.w - fBarW_;
			_ctx.FillRect(_DXG_RECT(fTrackX_, dd_.y, fBarW_, dd_.h), m_ItemHoverBg);
			const float fThumbH_ = (dd_.h > 0.0f) ? (dd_.h * (dd_.h / fFullH_)) : 0.0f;
			const float fThumbY_ = dd_.y + (m_fScrollY / fMaxScroll_) * (dd_.h - fThumbH_);
			_ctx.FillRect(_DXG_RECT(fTrackX_ + 1.0f, fThumbY_, fBarW_ - 2.0f, fThumbH_), m_ScrollColor);
		}

		if (nClick_ >= 0)
		{
			this->confirm_(nClick_);
			return;
		}

		// 드롭다운 밖 클릭 -> 닫기. 연 프레임은 무시한다(즉시 닫힘 방지) - 표식 소비는 다음 프레임 Render.
		if (!m_bJustOpened && m_bEnabled && _ctx.IsMouseReleased(DXG_MOUSE_LEFT) && !bHoverDrop_)	// UP 구동
		{
			this->close_();
			// 에디트 본문 밖까지 나간 클릭이면 커밋 + 포커스 해제까지 여기서 대행한다
			// (드롭다운 위 프레임의 모달 캡처로 pass1 밖-클릭 해제 경로가 막힐 수 있다).
			if (!_ctx.IsMouseHovered(abs_)) { this->SetFocused(false); }
			return;	// close_ 가 표식을 내렸다 - 아래 기록으로 되살리지 않는다
		}

		m_bHoverDropLast = bHoverDrop_;	// 다음 프레임 모달 판정용(닫히면 close_ 가 내린다)
	}

} // namespace dxgui
