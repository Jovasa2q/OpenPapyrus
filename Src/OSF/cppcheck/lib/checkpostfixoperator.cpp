/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
//
// You should use ++ and -- as prefix whenever possible as these are more
// efficient than postfix operators
//
#include "cppcheck-internal.h"
#pragma hdrstop

// Register this check class (by creating a static instance of it)
namespace {
CheckPostfixOperator instance;
}

void CheckPostfixOperator::postfixOperator()
{
	if(!mSettings->severity.isEnabled(Severity::performance))
		return;

	const SymbolDatabase * symbolDatabase = mTokenizer->getSymbolDatabase();

	for(const Scope * scope : symbolDatabase->functionScopes) {
		for(const Token* tok = scope->bodyStart->next(); tok != scope->bodyEnd; tok = tok->next()) {
			const Variable * var = tok->variable();
			if(!var || !Token::Match(tok, "%var% ++|--"))
				continue;

			const Token* parent = tok->next()->astParent();
			if(!parent || parent->str() == ";" ||
			    (parent->str() == "," && (!parent->astParent() || parent->astParent()->str() != "("))) {
				if(var->isPointer() || var->isArray())
					continue;

				if(Token::Match(var->nameToken()->previous(),
				    "iterator|const_iterator|reverse_iterator|const_reverse_iterator")) {
					// the variable is an iterator
					postfixOperatorError(tok);
				}
				else if(var->type()) {
					// the variable is an instance of class
					postfixOperatorError(tok);
				}
			}
		}
	}
}

//---------------------------------------------------------------------------

void CheckPostfixOperator::postfixOperatorError(const Token * tok)
{
	reportError(tok, Severity::performance, "postfixOperator",
	    "Prefer prefix ++/-- operators for non-primitive types.\n"
	    "Prefix ++/-- operators should be preferred for non-primitive types. "
	    "Pre-increment/decrement can be more efficient than "
	    "post-increment/decrement. Post-increment/decrement usually "
	    "involves keeping a copy of the previous value around and "
	    "adds a little extra code.", CWE398, Certainty::normal);
}
