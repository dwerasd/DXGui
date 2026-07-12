// DxgIcons.h: 아이콘 = 폰트 글리프(Segoe MDL2 Assets, Win10/11 내장).
// 에셋 파이프라인 0 / DPI 무관 벡터 / 색 = 텍스트색. 위젯은 이 문자를 DrawText 로 그린다.
// 코드포인트는 유니코드 사용자영역(PUA) - 소스에는 이스케이프로만 기입(직접 삽입 시 편집기 비가시 + 인코딩 사고).
// 값은 segmdl2.ttf cmap 파싱 + 실렌더로 모양까지 확인(2026-07-12).
#pragma once


namespace dxgui
{
namespace icons
{
	// 재생/제어
	inline constexpr wchar_t Play      = L'\uE768';	// 삼각 재생
	inline constexpr wchar_t Pause     = L'\uE769';	// 일시정지
	inline constexpr wchar_t Stop      = L'\uE71A';	// 사각 정지

	// 명령
	inline constexpr wchar_t Save      = L'\uE74E';	// 디스크
	inline constexpr wchar_t Settings  = L'\uE713';	// 톱니
	inline constexpr wchar_t Refresh   = L'\uE72C';	// 회전 화살표
	inline constexpr wchar_t Add       = L'\uE710';	// +
	inline constexpr wchar_t Search    = L'\uE721';	// 돋보기
	inline constexpr wchar_t Delete_   = L'\uE74D';	// 휴지통
	inline constexpr wchar_t Edit      = L'\uE70F';	// 연필
	inline constexpr wchar_t Download  = L'\uE896';	// 아래 화살표(대응 기능 생기면 사용)
	inline constexpr wchar_t Upload    = L'\uE898';	// 위 화살표(동)

	// 내비게이션(사이드바)
	inline constexpr wchar_t Home      = L'\uE80F';	// 집 - 대시보드
	inline constexpr wchar_t List      = L'\uE8FD';	// 목록 - 종목리스트
	inline constexpr wchar_t Star      = L'\uE734';	// 별 - 관심종목
	inline constexpr wchar_t Chart     = L'\uE9D9';	// 프레임 안 라인차트 - 차트
	inline constexpr wchar_t Calc      = L'\uE8EF';	// 계산기 - 엔진 백테스트
	inline constexpr wchar_t Tag       = L'\uE8EC';	// 태그 라벨 - 태깅

	// 아이콘 폰트 페이스 - 대상 OS(Win10/11) 내장이라 폴백 분기 없음.
	inline constexpr const wchar_t* Face = L"Segoe MDL2 Assets";

} // namespace icons
} // namespace dxgui
