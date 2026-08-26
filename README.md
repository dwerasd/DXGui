# DXGui

`dxgui::IDrawContext` 추상화에 의존하는 렌더러 독립 즉시모드형 GUI 위젯 라이브러리다.
위젯은 `IDrawContext`가 제공하는 텍스트/도형 그리기와 마우스·키보드·클립보드·IME
조회만 사용하며, D2D/DX11/DX12 등 실제 그래픽 API를 직접 참조하지 않는다.

## 구성

- `dxgui::IDrawContext` (`DxgDrawContext.h`) — 프레임 라이프사이클(BeginFrame/
  EndFrame), 폰트 등록·측정, 텍스트·사각형·라운드사각형·선·원 등 도형 그리기,
  차트용 quad/triangle/circle 프리미티브, 마우스·키보드 폴링, 클립보드,
  IME 조합 문자열, 클립 영역 push/pop, 입력 캡처(모달)를 정의하는 순수 가상
  인터페이스
- `dxgui::C_DXG_WIDGET` (`DxgWidget.h`) — 위젯 공통 베이스(leaf). 위치/크기/
  영속 키(ASCII)/표시명(UTF-16 wstring)/폰트 스케일/visible/enable을 갖고
  `Render(IDrawContext&, _origin)`을 가상함수로 노출
- `dxgui::C_DXG_MANAGER` (`DxgManager.h`) — 최상위 위젯 N개를 소유하는 루트
  관리자. 매 프레임 `Render(ctx)`로 2패스(일반 Render → RenderOverlay) 렌더,
  Tab 포커스 순회, 모달(콤보/메뉴/컬러필드 드롭다운) 감지·입력 캡처·강제 닫기,
  디자인모드용 키 위젯 히트테스트를 담당
- 위젯 구현: 라벨(`DxgLabel`), 버튼(`DxgButton`), 체크박스(`DxgCheckbox`),
  라디오(`DxgRadio`), 편집 상자(`DxgEditbox`), 자동완성 입력
  (`DxgAutoComplete`), 콤보박스(`DxgComboBox`), 컬러 필드(`DxgColorField`),
  패널(`DxgPanel`), 카드(`DxgCard`), 스탯 카드(`DxgStatCard`), 배지
  (`DxgBadge`), 리스트 그리드(`DxgListGrid`), 내비 목록(`DxgNavList`), 메뉴
  (`DxgMenu`), 탭(`DxgTabs`), 스크롤바(`DxgScrollbar`), 스피너
  (`DxgSpinner`), 스플리터(`DxgSplitter`), 테마(`DxgTheme`)
- 위젯 타입은 `E_DXG_WIDGET_TYPE`(`DxgWidget.h`)로 식별되며, 좌표계는
  float px(부모 원점 기준), 색은 32비트 ARGB, 문자열은 UTF-16(`wchar_t`)로
  통일되어 있다
- `dxgui::IOrderSink`(`DxgOrderSink.h`) — 주문 라우팅 추상 인터페이스(페이퍼/실주문
  전환용). 현재 이 저장소의 어떤 위젯·소스에서도 include/사용되지 않는
  독립 정의로, 향후 주문 위젯 도입을 위한 선배치 상태다

## 렌더링 백엔드

DXGui 프로젝트 자체는 그래픽스 API 구현을 포함하지 않는다. Direct2D 구체
백엔드는 형제 [D2DWrapp](../D2DWrapp)의 `d2d::C_DRAW_CONTEXT_D2D`에 있다.
DX11 백엔드는 `DxgDrawContext.h` 주석상 "향후(C_DRAW_CONTEXT_DX11, DX11Wrapp)"로
예정되어 있을 뿐 이 저장소에는 존재하지 않는다.

## 스택

- C++20(Debug/Release, ReleaseMD|Win32) / C++Latest(ReleaseMD|x64) — MSVC v145
  툴셋(`DXGui.vcxproj`). ReleaseMD|x64 만 PCH 미사용 + AVX2 활성화 + 버퍼
  보안검사(`/GS`) 비활성으로 별도 튜닝됨
- Win32 / x64 정적 라이브러리(StaticLibrary) 프로젝트(`DXGui.vcxproj`)
- 표준 라이브러리(`<string>`, `<vector>`, `<memory>`)만 사용, Windows/D2D/D3D
  헤더는 이 프로젝트에서 직접 포함하지 않음(`DxgDrawContext.h` 주석)

## 폴더 구성

저장소는 서브디렉터리 없이 위젯별 `Dxg*.h`/`Dxg*.cpp` 쌍과 공용 헤더
(`DxgTypes.h`, `DxgKeys.h`, `DxgIcons.h`, `DxgOrderSink.h`, `framework.h`,
`pch.h`/`pch.cpp`)가 평면 구조로 배치되어 있다. `DXGui.vcxproj`/
`.vcxproj.filters`가 Visual C++ 프로젝트 정의다.

## 빌드·실행

Windows 전용 Visual C++ 정적 라이브러리 프로젝트다. `DXGui.vcxproj`를 빌드하려면
같은 부모 디렉터리에 형제 프로젝트(예: 렌더 백엔드용 D2DWrapp)가 있는 솔루션
구성이 전제되며, 이 저장소 단독으로는 실행 가능한 산출물(exe)이 없다.

## 상태

이 프로젝트는 Claude Opus 4.8을 활용한 바이브 코딩으로 작성되었습니다.

## 라이선스

이 프로젝트는 MIT License에 따라 배포됩니다.
