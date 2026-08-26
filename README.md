# DXGui

`dxgui::IDrawContext` 추상화에 의존하는 렌더러 독립 GUI 위젯 라이브러리다.

## Public API 범위

- `dxgui::IDrawContext` — 렌더링·폰트·입력·클립보드 추상 인터페이스
- `dxgui::C_DXG_WIDGET`, `dxgui::C_DXG_MANAGER` — 위젯 기반 클래스와 루트 관리자
- 라벨, 버튼, 체크박스, 라디오, 편집 상자, 자동완성 입력, 콤보박스, 컬러 필드, 패널, 카드, 스탯 카드, 배지, 리스트 그리드, 내비 목록, 메뉴, 탭, 스크롤바, 스피너, 스플리터, 테마 등 `Dxg*.h` 위젯

## 렌더링 백엔드

DXGui 프로젝트 자체는 그래픽스 API 구현을 포함하지 않는다. Direct2D 구체 백엔드는 형제 [D2DWrapp](../D2DWrapp)의 `d2d::C_DRAW_CONTEXT_D2D`에 있다.

## 라이선스
이 프로젝트는 MIT License에 따라 배포됩니다.

---

*이 프로젝트는 Claude Opus 4.8을 활용한 바이브 코딩으로 작성되었습니다.*
