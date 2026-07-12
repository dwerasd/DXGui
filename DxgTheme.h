// DxgTheme.h: 디자인 토큰 단일출처(라이트 테마).
// 위젯 ctor 의 기본색/기본치수가 본 테마를 참조 - 색/간격 변경은 여기 한 곳만 수정하면 전파.
// 호출부가 SetColors/SetBgColor 등으로 명시 지정한 외형은 테마와 무관(오버라이드 우선).
// 폰트 핸들은 IDrawContext(RegisterFont) 소속이라 테마는 px 크기만 보유 - 셸이 기동 시 등록해 배포.
#pragma once

#include "DxgTypes.h"

#include <cstdint>


namespace dxgui
{

	struct _DXG_THEME
	{
		// 표면
		_DXG_COLOR canvas   { 0xFFF5F6F8u };	// 창 배경(회색 캔버스)
		_DXG_COLOR card     { 0xFFFFFFFFu };	// 카드 배경
		_DXG_COLOR border   { 0xFFE4E8EEu };	// 카드/컨트롤 1px 테두리
		_DXG_COLOR divider  { 0xFFEEF1F5u };	// 카드 내부 구분선

		// 텍스트
		_DXG_COLOR textMain { 0xFF222838u };	// 본문
		_DXG_COLOR textSub  { 0xFF8A93A6u };	// 캡션/보조
		_DXG_COLOR textDim  { 0xFFB9C0CCu };	// 비활성

		// 포인트 / 시맨틱
		_DXG_COLOR accent        { 0xFF2563EBu };	// 주 파랑(버튼/선택/링크)
		_DXG_COLOR accentHover   { 0xFF3B82F6u };
		_DXG_COLOR accentPressed { 0xFF1D4ED8u };
		_DXG_COLOR up      { 0xFFC62828u };		// 상승/매수 적(KR 관례)
		_DXG_COLOR down    { 0xFF1565C0u };		// 하락/매도 청
		_DXG_COLOR success { 0xFF16A34Au };		// 성공/실행
		_DXG_COLOR warning { 0xFFF59E0Bu };
		_DXG_COLOR danger  { 0xFFDC2626u };		// 오류/중지
		_DXG_COLOR neutral { 0xFF64748Bu };		// 대기/중립

		// 형태 / 간격 / 타이포(px)
		float radiusCard    { 8.0f };
		float radiusCtl     { 4.0f };
		float pad           { 16.0f };	// 카드 내부 패딩
		float gap           { 12.0f };	// 카드 사이 간격
		uint32_t fontCaption{ 11u };
		uint32_t fontBody   { 13u };
		uint32_t fontTitle  { 15u };	// 섹션 제목(bold 등록)
		uint32_t fontKpi    { 22u };	// KPI 큰 숫자(bold 등록)
	};


	// 전역 단일 인스턴스(정의 DxgTheme.cpp). 셸이 기동 시 값 조정 가능.
	_DXG_THEME& Theme();


	// 배지/선택 pill 틴트 - 본색 RGB 유지 + 알파 15%.
	inline _DXG_COLOR ThemeTint(_DXG_COLOR _c)
	{
		return _DXG_COLOR((_c.argb & 0x00FFFFFFu) | 0x26000000u);
	}

} // namespace dxgui
