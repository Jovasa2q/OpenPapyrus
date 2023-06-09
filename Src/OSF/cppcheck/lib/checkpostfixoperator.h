/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#ifndef checkpostfixoperatorH
#define checkpostfixoperatorH

class ErrorLogger;
class Settings;
class Token;

/// @addtogroup Checks
/// @{

/**
 * @brief Using postfix operators ++ or -- rather than postfix operator.
 */

class CPPCHECKLIB CheckPostfixOperator : public Check {
public:
	/** This constructor is used when registering the CheckPostfixOperator */
	CheckPostfixOperator() : Check(myName()) 
	{
	}
	/** This constructor is used when running checks. */
	CheckPostfixOperator(const Tokenizer * tokenizer, const Settings * settings, ErrorLogger * errorLogger) : Check(myName(), tokenizer, settings, errorLogger) 
	{
	}
	void runChecks(const Tokenizer * tokenizer, const Settings * settings, ErrorLogger * errorLogger) override 
	{
		if(tokenizer->isC())
			return;
		CheckPostfixOperator checkPostfixOperator(tokenizer, settings, errorLogger);
		checkPostfixOperator.postfixOperator();
	}
	/** Check postfix operators */
	void postfixOperator();
private:
	/** Report Error */
	void postfixOperatorError(const Token * tok);
	void getErrorMessages(ErrorLogger * errorLogger, const Settings * settings) const override 
	{
		CheckPostfixOperator c(nullptr, settings, errorLogger);
		c.postfixOperatorError(nullptr);
	}
	static std::string myName() { return "Using postfix operators"; }
	std::string classInfo() const override { return "Warn if using postfix operators ++ or -- rather than prefix operator\n"; }
};

/// @}
#endif // checkpostfixoperatorH
