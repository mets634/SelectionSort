#pragma once
#include <time.h>

struct _timer {
	void start() {
		begin = time(0);
	}
	void stop() {
		end = time(0);
	}

	double get_time() {
		return end - begin;
	}
private:
	double begin;
	double end;
};