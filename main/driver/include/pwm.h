#pragma once

#include <stdint.h>

namespace Wrapper {

class PWM {
public:
	enum Unit : uint8_t {
		UNIT0 = 0,
		UNIT1,
		UNIT2,
	};
	PWM(Unit unit, uint8_t channel, uint8_t pin);

	int start(float duty);
	void update(float duty);

private:
	Unit _unit;
	uint8_t _channel;
};


}
