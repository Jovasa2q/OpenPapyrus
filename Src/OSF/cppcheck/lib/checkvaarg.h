/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#ifndef checkvaargtH
#define checkvaargtH

#include "check.h"
//#include "cppcheck-config.h"

class ErrorLogger;
class Settings;
class Token;
class Tokenizer;

/// @addtogroup Checks
/// @{

/**
 * @brief Checking for misusage of variable argument lists
 */

class CPPCHECKLIB CheckVaarg : public Check {
public:
	CheckVaarg() : Check(myName()) {
	}

	CheckVaarg(const Tokenizer * tokenizer, const Settings * settings, ErrorLogger * errorLogger)
		: Check(myName(), tokenizer, settings, errorLogger) {
	}

	void runChecks(const Tokenizer * tokenizer, const Settings * settings, ErrorLogger * errorLogger) override {
		CheckVaarg check(tokenizer, settings, errorLogger);
		check.va_start_argument();
		check.va_list_usage();
	}

	void va_start_argument();
	void va_list_usage();

private:
	void wrongParameterTo_va_start_error(const Token * tok, const std::string& paramIsName, const std::string& paramShouldName);
	void referenceAs_va_start_error(const Token * tok, const std::string& paramName);
	void va_end_missingError(const Token * tok, const std::string& varname);
	void va_list_usedBeforeStartedError(const Token * tok, const std::string& varname);
	void va_start_subsequentCallsError(const Token * tok, const std::string& varname);

	void getErrorMessages(ErrorLogger * errorLogger, const Settings * settings) const override {
		CheckVaarg c(nullptr, settings, errorLogger);
		c.wrongParameterTo_va_start_error(nullptr, "arg1", "arg2");
		c.referenceAs_va_start_error(nullptr, "arg1");
		c.va_end_missingError(nullptr, "vl");
		c.va_list_usedBeforeStartedError(nullptr, "vl");
		c.va_start_subsequentCallsError(nullptr, "vl");
	}

	static std::string myName() {
		return "Vaarg";
	}

	std::string classInfo() const override {
		return "Check for misusage of variable argument lists:\n"
		       "- Wrong parameter passed to va_start()\n"
		       "- Reference passed to va_start()\n"
		       "- Missing va_end()\n"
		       "- Using va_list before it is opened\n"
		       "- Subsequent calls to va_start/va_copy()\n";
	}
};

/// @}

//---------------------------------------------------------------------------
#endif // checkvaargtH
