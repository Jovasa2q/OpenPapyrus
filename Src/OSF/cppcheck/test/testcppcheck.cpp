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
#include "color.h"
#include "cppcheck.h"

class TestCppcheck : public TestFixture {
public:
	TestCppcheck() : TestFixture("TestCppcheck") {
	}

private:

	class ErrorLogger2 : public ErrorLogger {
public:
		std::list<std::string> id;

		void reportOut(const std::string & /*outmsg*/, Color = Color::Reset) override {
		}

		void reportErr(const ErrorMessage &msg) override {
			id.push_back(msg.id);
		}
	};

	void run() override {
		TEST_CASE(instancesSorted);
		TEST_CASE(classInfoFormat);
		TEST_CASE(getErrorMessages);
	}

	void instancesSorted() const {
		for(std::list<Check *>::const_iterator i = Check::instances().begin(); i != Check::instances().end(); ++i) {
			std::list<Check *>::const_iterator j = i;
			++j;
			if(j != Check::instances().end()) {
				ASSERT_EQUALS(true, (*i)->name() < (*j)->name());
			}
		}
	}

	void classInfoFormat() const {
		for(std::list<Check *>::const_iterator i = Check::instances().begin(); i != Check::instances().end(); ++i) {
			const std::string info = (*i)->classInfo();
			if(!info.empty()) {
				ASSERT('\n' != info[0]); // No \n in the beginning
				ASSERT('\n' == info.back()); // \n at end
				if(info.size() > 1)
					ASSERT('\n' != info[info.length()-2]); // Only one \n at end
			}
		}
	}

	void getErrorMessages() const {
		ErrorLogger2 errorLogger;
		CppCheck cppCheck(errorLogger, true, nullptr);
		cppCheck.getErrorMessages();
		ASSERT(!errorLogger.id.empty());

		// Check if there are duplicate error ids in errorLogger.id
		std::string duplicate;
		for(std::list<std::string>::iterator it = errorLogger.id.begin();
		    it != errorLogger.id.end();
		    ++it) {
			if(std::find(errorLogger.id.begin(), it, *it) != it) {
				duplicate = "Duplicate ID: " + *it;
				break;
			}
		}
		ASSERT_EQUALS("", duplicate);

		// Check for error ids from this class.
		bool foundPurgedConfiguration = false;
		bool foundTooManyConfigs = false;
		for(const std::string & it : errorLogger.id) {
			if(it == "purgedConfiguration")
				foundPurgedConfiguration = true;
			else if(it == "toomanyconfigs")
				foundTooManyConfigs = true;
		}
		ASSERT(foundPurgedConfiguration);
		ASSERT(foundTooManyConfigs);
	}
};

REGISTER_TEST(TestCppcheck)
