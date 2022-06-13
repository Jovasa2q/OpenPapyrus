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
#include "checkboost.h"
#include "errortypes.h"
#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"

class TestBoost : public TestFixture {
public:
	TestBoost() : TestFixture("TestBoost") {
	}

private:
	Settings settings;

	void run() override {
		settings.severity.enable(Severity::style);
		settings.severity.enable(Severity::performance);

		TEST_CASE(BoostForeachContainerModification);
	}

#define check(code) check_(code, __FILE__, __LINE__)
	void check_(const char code[], const char* file, int line) {
		// Clear the error buffer..
		errout.str("");

		// Tokenize..
		Tokenizer tokenizer(&settings, this);
		std::istringstream istr(code);
		ASSERT_LOC(tokenizer.tokenize(istr, "test.cpp"), file, line);

		// Check..
		CheckBoost checkBoost;
		checkBoost.runChecks(&tokenizer, &settings, this);
	}

	void BoostForeachContainerModification() {
		check("void f() {\n"
		    "    vector<int> data;\n"
		    "    BOOST_FOREACH(int i, data) {\n"
		    "        data.push_back(123);\n"
		    "    }\n"
		    "}");
		ASSERT_EQUALS(
			"[test.cpp:4]: (error) BOOST_FOREACH caches the end() iterator. It's undefined behavior if you modify the container inside.\n",
			errout.str());

		check("void f() {\n"
		    "    set<int> data;\n"
		    "    BOOST_FOREACH(int i, data) {\n"
		    "        data.insert(123);\n"
		    "    }\n"
		    "}");
		ASSERT_EQUALS(
			"[test.cpp:4]: (error) BOOST_FOREACH caches the end() iterator. It's undefined behavior if you modify the container inside.\n",
			errout.str());

		check("void f() {\n"
		    "    set<int> data;\n"
		    "    BOOST_FOREACH(const int &i, data) {\n"
		    "        data.erase(123);\n"
		    "    }\n"
		    "}");
		ASSERT_EQUALS(
			"[test.cpp:4]: (error) BOOST_FOREACH caches the end() iterator. It's undefined behavior if you modify the container inside.\n",
			errout.str());

		// Check single line usage
		check("void f() {\n"
		    "    set<int> data;\n"
		    "    BOOST_FOREACH(const int &i, data)\n"
		    "        data.clear();\n"
		    "}");
		ASSERT_EQUALS(
			"[test.cpp:4]: (error) BOOST_FOREACH caches the end() iterator. It's undefined behavior if you modify the container inside.\n",
			errout.str());

		// Container returned as result of a function -> Be quiet
		check("void f() {\n"
		    "    BOOST_FOREACH(const int &i, get_data())\n"
		    "        data.insert(i);\n"
		    "}");
		ASSERT_EQUALS("", errout.str());

		// Break after modification (#4788)
		check("void f() {\n"
		    "    vector<int> data;\n"
		    "    BOOST_FOREACH(int i, data) {\n"
		    "        data.push_back(123);\n"
		    "        break;\n"
		    "    }\n"
		    "}");
		ASSERT_EQUALS("", errout.str());
	}
};

REGISTER_TEST(TestBoost)
