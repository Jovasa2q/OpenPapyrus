/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include <cppcheck-test-internal.h>
#pragma hdrstop
#include "timer.h"

class TestTimer : public TestFixture {
public:
	TestTimer() : TestFixture("TestTimer") {
	}
private:
	void run() override { TEST_CASE(result); }
	void result() const 
	{
		TimerResultsData t1;
		t1.mClocks = ~(std::clock_t)0;
		ASSERT(t1.seconds() > 100.0);

		t1.mClocks = CLOCKS_PER_SEC * 5 / 2;
		ASSERT(std::fabs(t1.seconds()-2.5) < 0.01);
	}
};

REGISTER_TEST(TestTimer)
