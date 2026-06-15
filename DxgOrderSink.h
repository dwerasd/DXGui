// DxgOrderSink.h: 주문 라우팅 추상(렌더 무관). 주문 위젯은 IOrderSink 만 의존.
// 현재 = 페이퍼(PaperOrderSink, 호스트 구현). 향후 = 실주문(TcpBridgeOrderSink).
#pragma once

#include <string>
#include <cstdint>


namespace dxgui
{

	enum E_DXG_ORDER_SIDE : uint8_t { DXG_ORDER_BUY = 0, DXG_ORDER_SELL = 1 };
	enum E_DXG_ORDER_TYPE : uint8_t { DXG_ORDER_LIMIT = 0, DXG_ORDER_MARKET = 1 };

	// 주문 1건. (향후 확장: tr/accNo/orgOrdNo)
	struct DxgOrder
	{
		std::wstring     sCode;		// 종목코드
		int              nQty;		// 수량
		double           dbPrice;	// 지정가(시장가 시 무시)
		E_DXG_ORDER_SIDE side;
		E_DXG_ORDER_TYPE type;

		DxgOrder()
			: nQty(0), dbPrice(0.0)
			, side(DXG_ORDER_BUY), type(DXG_ORDER_LIMIT)
		{
		}
	};

	// 주문 싱크 — 위젯은 이 인터페이스로만 주문 발행. 배선 1지점 교체로 페이퍼↔실주문 전환.
	class IOrderSink
	{
	public:
		virtual ~IOrderSink() = default;
		virtual bool Submit(const DxgOrder& _order) = 0;	// 접수/체결 성공 시 true
	};

} // namespace dxgui
