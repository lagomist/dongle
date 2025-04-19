#pragma once

#include "stdint.h"
#include "stddef.h"

namespace Wrapper {

namespace AppTimer {

constexpr uint32_t const TICK_PERIOD_MS	= 1;
constexpr int const LOOP_FOREVER 		= -1;

using Action = void(*)(uint32_t);

class Task {
public:

	enum class Cmd : uint8_t {
		PAUSE = 1,
		RESUME,
		KILL,
		PERIOD,
		PARAM,
		COUNT,
		RESTART,
		ACTIVATED,
		REMAINING,
	};

	Task() {}
	Task(Action task, uint32_t period, uint32_t param = 0, int count = LOOP_FOREVER);

	int create(Action task, uint32_t period, uint32_t param = 0, int count = LOOP_FOREVER);
	uint32_t control(Cmd cmd, uint32_t param = 0);

private:
	void * _handle = nullptr;
};


void init();
void sliceProcess();

}

}
