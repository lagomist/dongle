#pragma once

#include "stdint.h"
#include "stddef.h"

namespace Wrapper {

namespace AppTimer {

constexpr uint32_t const TICK_PERIOD_MS	= 1;
constexpr int const LOOP_FOREVER 		= -1;
constexpr int const CALL_IMMEDIATE		= 0;

using Action = void(*)(void *arg);

class Task {
public:
	Task() {}
	Task(Action task, uint32_t period, void *param = nullptr, int count = LOOP_FOREVER);

	int create(Action task, uint32_t period, void *param = nullptr, int count = LOOP_FOREVER);

	void activate();
	void suspend();
	void restart();
	void kill();
	void setPeriod(uint32_t period);
	void setParam(void *param);
	void setCycleCount(uint32_t count);
	bool isActivated();
	uint32_t getRemainingTime();

private:
	void * _handle = nullptr;
};


class Timer {
public:
	Timer() {}
	Timer(Action cb, uint32_t period, void *param = nullptr, int count = 1);

	int create(Action cb, uint32_t period, void *param = nullptr, int count = 1);

	void start();
	void restart();
	void stop();
	void kill();
	void setPeriod(uint32_t period);
	void setCycleCount(uint32_t count);
	bool isRuning();
	uint32_t getRemainingTime();
private:
	void * _handle = nullptr;
	uint32_t _cycle_count;
};

uint32_t getTick();
void sliceProcess();
void init();

}

}
